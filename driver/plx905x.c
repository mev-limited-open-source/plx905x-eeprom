/*
 * PLX PCI905x serial EEPROM driver.
 * Currently supports PCI9050, PCI9052 and PCI9054 (revision A or later).
 * Untested support for PCI9030, PCI9056, PCI9060, PCI9080 and PCI9656.
 *
 * Written by Ian Abbott @ MEV Ltd. <ian.abbott@mev.co.uk>.
 * Copyright (C) 2002, 2019-2021 MEV Limited.
 *
 *     MEV Limited
 *     Building 67
 *     Europa Business Park
 *     Bird Hall Lane
 *     STOCKPORT
 *     SK3 0XA
 *     UNITED KINGDOM
 *
 *     Tel: +44 (0)161 477 1898
 *     WWW: https://www.mev.co.uk/
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * A copy of the GNU General Public License may be found in the file
 * "COPYING".
 *
 * As the copyright holder, MEV Limited reserves the right to re-use
 * (either directly or by license to third parties) those parts of the
 * program written by MEV Limited or its employees in other programs not
 * covered by the GNU General Public License.
 */

#include "plx905x-eeprom-config.h"

/*
 * Version information from plx905x-eeprom-config.h (set in configure.ac):
 */
#define DRIVER_NAME		PLX905X_EEPROM_DRIVER_NAME
#define DRIVER_VERSION_MAJOR	PLX905X_EEPROM_DRIVER_VERSION_MAJOR
#define DRIVER_VERSION_MINOR	PLX905X_EEPROM_DRIVER_VERSION_MINOR
#define DRIVER_VERSION_MICRO	PLX905X_EEPROM_DRIVER_VERSION_MICRO
#define DRIVER_VERSION_NANO	PLX905X_EEPROM_DRIVER_VERSION_NANO

/*
 * Define prefix for use with pr_...(fmt, ...) macros.
 */
#define pr_fmt(fmt) DRIVER_NAME ": " fmt

#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <linux/stat.h>
#include <asm/io.h>

#define KCOMPAT_DEFINE_CLASS_DEVICE_CREATE	/* For various 2.6.x */

#include "kcompat.h"
#include "kcompat_pci.h"

#ifdef KCOMPAT_HAVE_LINUX_UACCESS_H
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif

/*
 * More driver information:
 */
/* e.g. "v0.10" */
#define DRIVER_VERSION	"v" __MODULE_STRING(DRIVER_VERSION_MAJOR) \
	"." __MODULE_STRING(DRIVER_VERSION_MINOR) \
	"." __MODULE_STRING(DRIVER_VERSION_MICRO) \
	"." __MODULE_STRING(DRIVER_VERSION_NANO)
#define DRIVER_DESC	"PLX PCI905x Serial EEPROM driver"

/*
 * Class name:
 */
#define CLASS_NAME PLX905X_EEPROM_CLASS_NAME

/*
 * Device name prefix:
 */
#define DEVICE_PREFIX PLX905X_EEPROM_DEVICE_PREFIX

/*
 * Redefine pr_debug macro to use "debug" module parameter.
 */
#undef pr_debug
#define pr_debug(fmt, args...)						\
	(debug ? printk(KERN_DEBUG pr_fmt(fmt), ##args) : 0)

/*
 * Define pr_debuglvl macro like pr_debug but with a "debug level" parameter.
 */
#define pr_debuglvl(lvl, fmt, args...)					\
	(debug >= (lvl) ? printk(KERN_DEBUG pr_fmt(fmt), ##args) : 0)

/*
 * Redefine csdev_dbg macro (an invention of kcompat.h) to use "debug" module
 * parameter.
 */
#undef csdev_dbg
#define csdev_dbg(cd, fmt, args...)					\
	(debug ? csdev_printk(KERN_DEBUG, cd, fmt, ##args) : 0)

/*
 * Define csdev_dbglvl macro like csdev_dbg but with a "debug level" parameter.
 */
#define csdev_dbglvl(lvl, cd, fmt, args...)				\
	(debug >= (lvl) ? csdev_printk(KERN_DEBUG, cd, fmt, ##args) : 0)

#define PLX_VENDOR_ID		0x10B5
#define PLX9030_DEVICE_ID	0x9030
#define PLX9050_DEVICE_ID	0x9050
#define PLX9054_DEVICE_ID	0x9054
#define PLX9056_DEVICE_ID	0x9056
#define PLX9060_DEVICE_ID	0x9060
#define PLX9060SD_DEVICE_ID	0x906D
#define PLX9060ES_DEVICE_ID	0x906E
#define PLX9080_DEVICE_ID	0x9080
#define PLX9656_DEVICE_ID	0x9656
#define DEFAULT_DEVICE_ID	PLX9050_DEVICE_ID

#define PLX9054_PCIHIDR	0x70
#define PLX9054_PCIHIDR_VALUE	0x905410B5
#define PLX9056_PCIHIDR_VALUE	0x905610B5
#define PLX9060_PCIHIDR_VALUE	0x906010B5
#define PLX9060SD_PCIHIDR_VALUE	0x906D10B5
#define PLX9060ES_PCIHIDR_VALUE	0x906E10B5
#define PLX9080_PCIHIDR_VALUE	0x908010B5
#define PLX9656_PCIHIDR_VALUE	0x965610B5

#define PLX9054_PCIHREV	0x74

#define CS46_EEPROM_SIZE	128
#define CS46_EEPROM_ADDR_LEN	6
#define CS56_EEPROM_SIZE	256
#define CS56_EEPROM_ADDR_LEN	8
#define CS66_EEPROM_SIZE	512
#define CS66_EEPROM_ADDR_LEN	8

#define PLX9050_CNTRL	0x50
#define PLX9054_CNTRL	0x6C
#define EE_SK	0x01000000
#define EE_CS	0x02000000
#define EE_DI	0x04000000	/* from EEPROM's point of view */
#define EE_DO	0x08000000
#define EE_DOE	0x80000000	/* for PCI9056 */

#define PLX9050_EEMASK	(EE_SK | EE_CS | EE_DI | EE_DO)
#define PLX9056_EEMASK	(EE_SK | EE_CS | EE_DI | EE_DO | EE_DOE)

struct plx905x_dev {
	struct pci_dev *pcidev;
	resource_size_t iophys;
	resource_size_t iosize;
	union {
		unsigned long iobase;
		char __iomem *mmbase;
	} u;
#ifdef KCOMPAT_NO_CLASS_DEVICE
	struct device *csdev;
#else
	struct class_device *csdev;
#endif
	unsigned int iospace;
	unsigned int cntrl;
	unsigned int cntrl_eemask;
	size_t eeprom_size;
	unsigned int eeprom_addr_len;
	struct mutex mutex;
	unsigned long status;
};

/*
 * Module information:
 */

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Ian Abbott <ian.abbott@mev.co.uk>");
MODULE_LICENSE("GPL");

/*
 * Module parameters:
 */

/* Major device number; set to 0 to assign dynamically: */
static unsigned int major = PLX905X_MAJOR;
module_param(major, uint, 0444);
MODULE_PARM_DESC(major,
		 "Major device number; set to 0 to assign dynamically;"
		 " (default="__MODULE_STRING(PLX905X_MAJOR)")");

/*
 * Debug level; set non-zero to print debugging information:
 * (0 = no debug, 1 = normal debug, 2 = verbose debug.)
 */
static unsigned int debug = 0;
module_param(debug, uint, 0644);
MODULE_PARM_DESC(debug, "Debug message level (0=none, 1=debug, 2=verbose)");

/*
 * PCI bus number (optional)
 */
static unsigned int bus = 0;
module_param(bus, uint, 0444);
MODULE_PARM_DESC(bus, "PCI bus number (optional)");

/*
 * PCI slot number (optional)
 */
static unsigned int slot = 0;
module_param(slot, uint, 0444);
MODULE_PARM_DESC(bus, "PCI slot number (optional)");

/*
 * PCI vendor ID (optional)
 */
static unsigned int vendor = PCI_ANY_ID;
module_param(vendor, uint, 0444);
MODULE_PARM_DESC(vendor, "PCI Vendor ID (optional)");

static unsigned int device = PCI_ANY_ID;
module_param(device, uint, 0444);
MODULE_PARM_DESC(device, "PCI Device ID (optional)");

static unsigned int subvendor = PCI_ANY_ID;
module_param(subvendor, uint, 0444);
MODULE_PARM_DESC(subvendor, "PCI Subsystem Vendor ID (optional)");

static unsigned int subdevice = PCI_ANY_ID;
module_param(subdevice, uint, 0444);
MODULE_PARM_DESC(subdevice, "PCI Subsystem Device ID (optional)");

static unsigned int instance = 0;
module_param(instance, uint, 0444);
MODULE_PARM_DESC(instance,
		 "Instance of PCI Vendor/Device/Subsystem IDs (default 0)");

static unsigned int eeprom = 0;
module_param(eeprom, uint, 0444);
MODULE_PARM_DESC(eeprom,
		 "EEPROM type 46 (1024-bit), 56 (2048-bit), 66 (4096-bit) "
		 "(default depends on PLX device)");

static unsigned int plx=0;
module_param(plx, uint, 0444);
MODULE_PARM_DESC(plx,
		 "PLX chip type 0x9030, 0x9050, 0x9052 (equivalent to 0x9050), "
		 "0x9054, 0x9056, 0x9060, 0x9080, 0x9656 (default 0x9050)");

/*
 * Sysfs class for 2.6:
 */
static struct class *plx905x_class;

static struct plx905x_dev plx905x_device = {0};

static u32
cntrl_read(struct plx905x_dev *dev)
{
	if (dev->iospace == IORESOURCE_IO) {
		return inl(dev->u.iobase + dev->cntrl);
	} else {
		return readl(dev->u.mmbase + dev->cntrl);
	}
}

static void
cntrl_write(struct plx905x_dev *dev, u32 data)
{
	if (dev->iospace == IORESOURCE_IO) {
		outl(data, dev->u.iobase + dev->cntrl);
	} else {
		writel(data, dev->u.mmbase + dev->cntrl);
	}
}

/* Assert CS and send start bit. */
static void
eeprom_start_cmd(struct plx905x_dev *dev, u32 *cntrl)
{
	u32 cn;

	cn = cntrl_read(dev);
	cn = (cn & ~EE_SK) | ((EE_CS | EE_DI | EE_DOE) & dev->cntrl_eemask);
						/* SK=0, CS=1, DI=1, DOE1=1 */
	cntrl_write(dev, cn);
	udelay(2);
	cn |= EE_SK;				/* SK=1 */
	cntrl_write(dev, cn);
	udelay(2);
	*cntrl = cn;
}

/* Deassert CS.  Assumes *cntrl is valid. */
static void
eeprom_end_cmd(struct plx905x_dev *dev, u32 *cntrl)
{
	u32 cn = *cntrl;

	cn &= ~((EE_CS | EE_SK | EE_DI | EE_DOE) & dev->cntrl_eemask);
						/* CS=0, SK=0, DI=0, DOE=0 */
	cntrl_write(dev, cn);
	udelay(2);
	*cntrl = cn;
}

/* Send a few bits.  Assumes *cntrl is valid. */
static void
eeprom_put_bits(struct plx905x_dev *dev, u32 *cntrl, unsigned int bits,
		unsigned int nbits)
{
	u32 cn = *cntrl;

	while (nbits--) {
		if (bits & (1 << nbits)) {
			cn |= ((EE_DI | EE_DOE) & dev->cntrl_eemask);
							/* DI=1 */
		} else {
			cn &= ~((EE_DI | EE_DOE) & dev->cntrl_eemask);
							/* DI=0 */
		}
		cn &= ~EE_SK;			/* SK=0 */
		cntrl_write(dev, cn);
		udelay(2);
		cn |= EE_SK;			/* SK=1 */
		cntrl_write(dev, cn);
		udelay(2);
	}
	*cntrl = cn;
}

/* Wait for programming cycle to complete. */
static int
eeprom_wait_prog(struct plx905x_dev *dev)
{
	unsigned long old_jiffies;
	unsigned long timeout;
	u32 cn;
	int retval;

	timeout = 1 + (((50 * HZ) + 999) / 1000);	/* ~50ms */
	cn = cntrl_read(dev);
	cn = (cn & ~EE_SK) | ((EE_CS | EE_DI | EE_DOE) & dev->cntrl_eemask);
						/* SK=0, CS=1, DI=1, DOE=1 */
	cntrl_write(dev, cn);
	old_jiffies = jiffies;
	retval = -EIO;
	udelay(2);
	do {
		schedule();
		cn = cntrl_read(dev);
		if ((cn & EE_DO) != 0) {
			/* Cycle complete.  Clear ready status (optional). */
			cn |= EE_SK;		/* SK=1 */
			cntrl_write(dev, cn);
			udelay(2);
			retval = 0;
			break;
		}
	} while (jiffies - old_jiffies < timeout);
	cn &= ~((EE_CS | EE_SK | EE_DI | EE_DOE) & dev->cntrl_eemask);
						/* CS=0, SK=0, DI=0, DOE=0 */
	cntrl_write(dev, cn);
	udelay(2);
	return retval;
}

static int
eeprom_cmd_read_word(struct plx905x_dev *dev, unsigned int offset, u16 *data)
{
	u32 cntrl;
	u16 d;
	int i;
	int retval = 0;

	if (offset >= (dev->eeprom_size >> 1)) {
		return -ENXIO;
	}
	eeprom_start_cmd(dev, &cntrl);
	eeprom_put_bits(dev, &cntrl, 0x2, 2);
	eeprom_put_bits(dev, &cntrl, offset, dev->eeprom_addr_len);
	udelay(1);
	/* Check dummy bit DO==0. */
	cntrl = cntrl_read(dev);
	if ((cntrl & EE_DO) != 0) {
		retval = -EIO;
		goto out;
	}
	cntrl |= ((EE_DI | EE_DOE) & dev->cntrl_eemask);	/* DI=1, DOE=1 */
	/* Read 16 data bits m.s.b. first. */
	d = 0;
	for (i = 0; i < 16; i++) {
		d <<= 1;
		cntrl &= ~EE_SK;		/* SK=0 */
		cntrl_write(dev, cntrl);
		udelay(2);
		cntrl |= EE_SK;			/* SK=1 */
		cntrl_write(dev, cntrl);
		udelay(3);
		cntrl = cntrl_read(dev);
		if ((cntrl & EE_DO) != 0) {
			d |= 1;
		}
	}
	*data = d;
out:
	eeprom_end_cmd(dev, &cntrl);

	return retval;
}

static int
eeprom_cmd_write_word(struct plx905x_dev *dev, unsigned int offset, u16 data)
{
	u32 cntrl;

	if (offset >= (dev->eeprom_size >> 1)) {
		return -ENXIO;
	}
	eeprom_start_cmd(dev, &cntrl);
	eeprom_put_bits(dev, &cntrl, 0x1, 2);
	eeprom_put_bits(dev, &cntrl, offset, dev->eeprom_addr_len);
	eeprom_put_bits(dev, &cntrl, data, 16);
	eeprom_end_cmd(dev, &cntrl);
	return eeprom_wait_prog(dev);
}

static int
eeprom_cmd_write_enable(struct plx905x_dev *dev)
{
	u32 cntrl;

	eeprom_start_cmd(dev, &cntrl);
	eeprom_put_bits(dev, &cntrl, 0x3, 4);
	eeprom_put_bits(dev, &cntrl, 0, dev->eeprom_addr_len - 2);
	eeprom_end_cmd(dev, &cntrl);
	return 0;
}

static int
eeprom_cmd_write_disable(struct plx905x_dev *dev)
{
	u32 cntrl;

	eeprom_start_cmd(dev, &cntrl);
	eeprom_put_bits(dev, &cntrl, 0, dev->eeprom_addr_len + 2);
	eeprom_end_cmd(dev, &cntrl);
	return 0;
}

static void
eeprom_init(struct plx905x_dev *dev)
{
	u32 cn;

	cn = cntrl_read(dev);
	eeprom_end_cmd(dev, &cn);
	cn |= EE_SK;
	cntrl_write(dev, cn);
	udelay(2);
	eeprom_end_cmd(dev, &cn);
}

static int
plx905x_open(struct inode *inode, struct file *filp)
{
	struct plx905x_dev *dev = &plx905x_device;

	filp->private_data = dev;
	mutex_lock(&dev->mutex);
	eeprom_init(dev);
	mutex_unlock(&dev->mutex);
	return 0;
}

static int
plx905x_release(struct inode *inode, struct file *filp)
{
	return 0;
}

static ssize_t
plx905x_read(struct file *filp, char *buf, size_t count, loff_t *f_pos)
{
	struct plx905x_dev *dev = filp->private_data;
	ssize_t retval = 0;
	size_t n = 0;
	unsigned int addr;
	u16 data = 0;

	if (*f_pos >= dev->eeprom_size) {
		return 0;
	}
	if (*f_pos + count > dev->eeprom_size) {
		count = dev->eeprom_size - *f_pos;
	}
	if (mutex_lock_interruptible(&dev->mutex)) {
		return -ERESTARTSYS;
	}
	if (!access_ok(buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	for (addr = *f_pos, n = 0; n < count; addr++, n++) {
		if ((n == 0) || ((addr & 1) == 0)) {
			/* Read 16-bit word from EEPROM. */
			retval = eeprom_cmd_read_word(dev, addr>>1, &data);
			if (retval < 0) {
				break;
			}
		}
		/* Put data to the user in little-endian order. */
		__put_user(((addr&1) ? (data>>8) : data), buf++);
	}

	if (n) {
		retval = n;
		*f_pos += n;
	}

out:
	mutex_unlock(&dev->mutex);
	return retval;
}

static ssize_t
plx905x_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos)
{
	struct plx905x_dev *dev = filp->private_data;
	ssize_t retval = 0;
	int ret;
	size_t n = 0;
	unsigned int addr;
	u16 data = 0;

	if (*f_pos > dev->eeprom_size) {
		return -ENOSPC;
	}
	if (count == 0) {
		return 0;
	}
	if (*f_pos + count > dev->eeprom_size) {
		count = dev->eeprom_size - *f_pos;
		if (count == 0)
			return -ENOSPC;
	}
	if (mutex_lock_interruptible(&dev->mutex)) {
		return -ERESTARTSYS;
	}
	if (!access_ok(buf, count)) {
		retval = -EFAULT;
		goto out;
	}

	retval = eeprom_cmd_write_enable(dev);
	if (retval) {
		goto out;
	}

	for (addr = *f_pos, n = 0; n < count; addr++, n++) {
		u8 byte;

		if (((n == 0) && ((addr&1) != 0))
				|| ((count - n == 1) && ((addr&1) == 0))) {
			/* Modifying half a 16-bit word at a boundary. */
			retval = eeprom_cmd_read_word(dev, addr>>1, &data);
			if (retval) {
				break;
			}
		}
		/* Get data from user in little-endian order. */
		__get_user(byte, buf++);
		if ((addr&1) == 0) {
			data = (data & 0xFF00) | byte;
		} else {
			data = (data & 0x00FF) | (byte << 8);
		}
		if (((addr&1) != 0) || (count - n == 1)) {
			/* Write 16-bit word to EEPROM. */
			retval = eeprom_cmd_write_word(dev, addr>>1, data);
			if (retval) {
				if (((addr&1) != 0) && (n > 0)) {
					n--;
					addr--;
				}
				break;
			}
		}
	}

	ret = eeprom_cmd_write_disable(dev);
	if (!retval) {
		retval = ret;
	}

	if (n) {
		retval = n;
		*f_pos += n;
	}

out:
	mutex_unlock(&dev->mutex);
	return retval;
}

static loff_t
plx905x_llseek(struct file *filp, loff_t off, int whence)
{
	struct plx905x_dev *dev = filp->private_data;
	loff_t pos;

	switch (whence) {
	case 0:	/* SEEK_SET */
		pos = 0;
		break;
	case 1:	/* SEEK_CUR */
		pos = filp->f_pos;
		break;
	case 2:	/* SEEK_END */
		pos = dev->eeprom_size;
		break;
	default:	/* cannot happen */
		return -EINVAL;
	}
	pos += off;
	if ((pos < 0) || (pos > dev->eeprom_size)) {
		return -EINVAL;
	}
	filp->f_pos = pos;
	return pos;
}

static struct file_operations plx905x_fops = {
	.owner = THIS_MODULE,
	.llseek = plx905x_llseek,
	.read = plx905x_read,
	.write = plx905x_write,
	.open = plx905x_open,
	.release = plx905x_release,
};

int __init
plx905x_module_init(void)
{
	struct pci_dev *pcidev = NULL;
	int inst = 0;
	unsigned int dev_vendor = PCI_ANY_ID;
	unsigned int dev_device = PCI_ANY_ID;
	unsigned int dev_subvendor = PCI_ANY_ID;
	unsigned int dev_subdevice = PCI_ANY_ID;
	resource_size_t baraddr, barsize;
	unsigned int barflags;
	unsigned int dev_bus = 0;
	unsigned int dev_slot = 0;
	int rc = 0;
	unsigned model = 0;

	pr_info("%s, %s\n", DRIVER_DESC, DRIVER_VERSION);
	if (!bus && !slot) {
		if (vendor == PCI_ANY_ID && device == PCI_ANY_ID) {
			vendor = PLX_VENDOR_ID;
			switch (plx) {
			case 9030:
			case 0x9030:
				device = PLX9030_DEVICE_ID;
				break;
			case 9050:
			case 0x9050:
			case 9052:
			case 0x9052:
				device = PLX9050_DEVICE_ID;
				break;
			case 9054:
			case 0x9054:
				device = PLX9054_DEVICE_ID;
				break;
			case 9056:
			case 0x9056:
				device = PLX9056_DEVICE_ID;
				break;
			case 9060:
			case 0x9060:
				device = PLX9060_DEVICE_ID;
				break;
			case 9080:
			case 0x9080:
				device = PLX9080_DEVICE_ID;
				break;
			case 9656:
			case 0x9656:
				device = PLX9656_DEVICE_ID;
				break;
			default:
				device = DEFAULT_DEVICE_ID;
				break;
			}
		}
	}

	while ((pcidev = pci_get_subsys(vendor, device, subvendor, subdevice,
					pcidev)) != NULL) {
		if (bus || slot) {
			if (bus != pcidev->bus->number ||
			    slot != PCI_SLOT(pcidev->devfn)) {
				continue;
			}
		}
		if (pcidev->hdr_type != PCI_HEADER_TYPE_NORMAL) {
			continue;
		}
		if (++inst > instance) {
			/* Found required instance. */
			break;
		}
		if (bus || slot) {
			/* Ignore instance if bus or slot specified. */
			break;
		}
	}

	if (!pcidev) {
		pr_err("Could not find PCI device\n");
		rc = -ENODEV;
		goto out_fail_find_device;
	}

	dev_bus = pcidev->bus->number;
	dev_slot = PCI_SLOT(pcidev->devfn);
	dev_vendor = pcidev->vendor;
	dev_device = pcidev->device;
	dev_subvendor = pcidev->subsystem_vendor;
	dev_subdevice = pcidev->subsystem_device;
	pr_info("%02x:%02x %04x:%04x (%04x:%04x) (%d)\n",
		dev_bus, dev_slot, dev_vendor, dev_device,
		dev_subvendor, dev_subdevice, inst - 1);

	rc = pci_enable_device(pcidev);
	if (rc) {
		pr_err("failed to enable PCI device\n");
		goto out_fail_pci_enable_device;
	}
	/* Try to confirm that it really is a supported PLX chip. */
	/* Check for local configuration registers in PCIBAR0 (memory). */
	{
		resource_size_t bar1size;

		barsize = pci_resource_len(pcidev, 0);
		bar1size = pci_resource_len(pcidev, 1);
		if (barsize == 512 &&
		    (pci_resource_flags(pcidev, 0) & IORESOURCE_MEM)) {
			/* Assume it is PLX PCI9056/9656 */
			baraddr = pci_resource_start(pcidev, 0);
			barflags = IORESOURCE_MEM;
		} else if (barsize == 256 &&
		    (pci_resource_flags(pcidev, 0) & IORESOURCE_MEM)) {
			/* Assume it is PLX PCI9054/9080 */
			baraddr = pci_resource_start(pcidev, 0);
			barflags = IORESOURCE_MEM;
		} else if ((barsize == 0 ||
			    (barsize == 128 &&
			     (pci_resource_flags(pcidev, 0) &
			      IORESOURCE_MEM))) &&
			   (bar1size == 0 ||
			    (bar1size == 128 &&
			     (pci_resource_flags(pcidev, 1) &
			      IORESOURCE_IO))) &&
			   (barsize != 0 || bar1size != 0)) {
			/* Assume it is PLX PCI9030/9050/9052 */
			if (barsize != 0) {
				baraddr = pci_resource_start(pcidev, 0);
				barflags = IORESOURCE_MEM;
			} else {
				barsize = bar1size;
				baraddr = pci_resource_start(pcidev, 1);
				barflags = IORESOURCE_IO;
			}
		} else {
			pr_err("not PLX\n");
			rc = -ENODEV;
			goto out_fail_plx_resource_check;
		}
	}

	/* Initialize device. */
	mutex_init(&plx905x_device.mutex);
	plx905x_device.pcidev = pcidev;
	plx905x_device.eeprom_size = CS46_EEPROM_SIZE;
	plx905x_device.eeprom_addr_len = CS46_EEPROM_ADDR_LEN;
	plx905x_device.cntrl = PLX9050_CNTRL;	/* Change later for PCI9054 */
	plx905x_device.cntrl_eemask = PLX9050_EEMASK;
	plx905x_device.iospace = barflags;
	plx905x_device.iophys = baraddr;
	plx905x_device.iosize = barsize;
	if (plx905x_device.iospace == IORESOURCE_IO) {
		/* Get PCI I/O space. */
		if (!request_region(plx905x_device.iophys,
				    plx905x_device.iosize, DRIVER_NAME)) {
			pr_err("I/O port busy\n");
			rc = -EIO;
			goto out_fail_request_region;
		}
		plx905x_device.u.iobase = plx905x_device.iophys;
	} else {
		/* Get PCI memory space. */
		if (!request_mem_region(plx905x_device.iophys,
					plx905x_device.iosize, DRIVER_NAME)) {
			pr_err("I/O port busy\n");
			rc = -EIO;
			goto out_fail_request_region;
		}
		plx905x_device.u.mmbase =
			ioremap(plx905x_device.iophys, plx905x_device.iosize);
		if (plx905x_device.u.mmbase == 0) {
			pr_err("cannot map I/O mem\n");
			rc = -ENOMEM;
			goto out_fail_ioremap;
		}
	}
	/* Set return value for further errors. */
	rc = -ENODEV;
	/* Examine device to determine model. */
	if (plx905x_device.iosize == 128) {
		/* Check for PCI9030/9050/9052 */
		u8 rev;
		u8 pvpdcntl;

		/*
		 * Read PCI9030's VPD control register in PCI header
		 * to distinguish it from PCI9050/5052.
		 */
		pci_read_config_byte(pcidev, 0x4C, &pvpdcntl);
		/* Read PCI revision to distinguish PCI9050/9052 */
		rev = pcidev->revision;
		/*
		 * Check for PCI9030.  PCIBAR0 must be 128 bytes memory
		 * and its PVDCNTL register must be 0x03
		 */
		if (plx905x_device.iospace != IORESOURCE_IO &&
		    pvpdcntl == 0x03) {
			model = 0x9030;
		} else {
			if (rev > 2) {
				pr_err("not PLX PCI9050/9052 (revision is >2)\n");
				goto out_fail_plx_model;
			}
			if (rev < 2) {
				model = 0x9050;
			} else {
				model = 0x9052;
				rev = 1;
			}
		}
		pr_info("PCI%X rev %02X\n", model, rev);
	} else {
		/* Check for PLX PCI9054/9056/9060/9080/9656 */
		u32 hidr;
		u8 hrev;
		int hrev_okay = 0;
		char *suffix = "";

		plx905x_device.cntrl = PLX9054_CNTRL;
		hidr = readl(plx905x_device.u.mmbase + PLX9054_PCIHIDR);
		hrev = readb(plx905x_device.u.mmbase + PLX9054_PCIHREV);
		/* Check for supported type and revision. */
		switch (hidr) {
		case PLX9054_PCIHIDR_VALUE:
			model = 0x9054;
			if (hrev >= 0x0A) {
				hrev_okay = 1;
			}
			break;
		case PLX9056_PCIHIDR_VALUE:
			plx905x_device.cntrl_eemask = PLX9056_EEMASK;
			model = 0x9056;
			hrev_okay = 1;
			break;
		case PLX9060SD_PCIHIDR_VALUE:
			suffix = "SD";
			model = 0x9060;
			break;
		case PLX9060ES_PCIHIDR_VALUE:
			suffix = "ES";
			model = 0x9060;
			break;
		case PLX9060_PCIHIDR_VALUE:
			model = 0x9060;
			break;
		case PLX9080_PCIHIDR_VALUE:
			model = 0x9080;
			hrev_okay = 1;
			break;
		case PLX9656_PCIHIDR_VALUE:
			plx905x_device.cntrl_eemask = PLX9056_EEMASK;
			model = 0x9656;
			if (hrev >= 0xAA) {
				hrev_okay = 1;
			}
			break;
		case 0:
			/* May be a PCI9060 */
			if (plx == 0x9060 || plx == 9060) {
				/* Believe kernel parameter. */
				model = 0x9060;
				hrev_okay = 1;
				break;
			}
			fallthrough;
			/* Else fall through. */
		default:
			pr_err("not PLX\n");
			goto out_fail_plx_model;
		}
		pr_info("PCI%X%s rev %02X\n", (unsigned)(hidr >> 16),
			suffix, (unsigned)hrev);
		if (!hrev_okay) {
			pr_err("bad revision\n");
			goto out_fail_plx_model;
		}
	}
	/* If 'plx' kernel parameter used, check against detected model. */
	if (plx != 0) {
		int model_okay = 0;

		switch (model) {
		case 0x9030:
			if (plx == 0x9030 || plx == 9030) {
				model_okay = 1;
			}
			break;
		case 0x9050:
		case 0x9052:	/* Treat these two as equivalent. */
			if (plx == 0x9050 || plx == 9050 ||
			    plx == 0x9052 || plx == 9052) {
				model_okay = 1;
			}
			break;
		case 0x9054:
			if (plx == 0x9054 || plx == 9054) {
				model_okay = 1;
			}
			break;
		case 0x9056:
			if (plx == 0x9056 || plx == 9056) {
				model_okay = 1;
			}
			break;
		case 0x9080:
			if (plx == 0x9080 || plx == 9080) {
				model_okay = 1;
			}
			break;
		case 0x9656:
			if (plx == 0x9656 || plx == 9656) {
				model_okay = 1;
			}
			break;
		default:
			pr_err("bug %s[%ld]\n", __FILE__, (long)__LINE__);
			goto out_fail_plx_model;
		}
		if (!model_okay) {
			pr_err("not specified PLX\n");
			goto out_fail_plx_model;
		}
	}
	/* Check EEPROM type (if specified). */
	switch (model) {
	case 0x9050:
	case 0x9052:
		switch (eeprom) {
		case 46: /* CS46 */
		case 128:
		case 1024:
		case 0:	/* default to CS46 */
			plx905x_device.eeprom_size = CS46_EEPROM_SIZE;
			plx905x_device.eeprom_addr_len = CS46_EEPROM_ADDR_LEN;
			break;
		default: /* invalid */
			pr_err("invalid EEPROM type for PLX PCI%04X\n", model);
			goto out_fail_eeprom_type;
		}
		break;
	case 0x9030:
	case 0x9054:
	case 0x9056:
	case 0x9656:
		switch (eeprom) {
		case 56: /* CS56 */
		case 256:
		case 2048:
		case 0:	/* default to CS56 */
			plx905x_device.eeprom_size = CS56_EEPROM_SIZE;
			plx905x_device.eeprom_addr_len = CS56_EEPROM_ADDR_LEN;
			break;
		case 66: /* CS66 */
		case 512:
		case 4096:
			plx905x_device.eeprom_size = CS66_EEPROM_SIZE;
			plx905x_device.eeprom_addr_len = CS66_EEPROM_ADDR_LEN;
			break;
		default: /* invalid */
			pr_err("invalid EEPROM type for PLX PCI%04X\n", model);
			goto out_fail_eeprom_type;
		}
		break;
	case 0x9060:
	case 0x9080:
		/* No default EEPROM size for PCI9060/9080 */
		switch (eeprom) {
		case 46: /* CS46 */
		case 128:
		case 1024:
			plx905x_device.eeprom_size = CS46_EEPROM_SIZE;
			plx905x_device.eeprom_addr_len = CS46_EEPROM_ADDR_LEN;
			break;
		case 56: /* CS56 */
		case 256:
		case 2048:
			plx905x_device.eeprom_size = CS56_EEPROM_SIZE;
			plx905x_device.eeprom_addr_len = CS56_EEPROM_ADDR_LEN;
			break;
		default: /* invalid */
			pr_err("must specify valid EEPROM type for PLX PCI%04X\n",
			       model);
			goto out_fail_eeprom_type;
		}
		break;
	default:
		pr_err("bug %s[%ld]\n", __FILE__, (long)__LINE__);
		goto out_fail_eeprom_type;
	}
	/* Try to register character device driver. */
	rc = register_chrdev(major, DRIVER_NAME, &plx905x_fops);
	if (rc < 0) {
		pr_err("cannot get major number\n");
		goto out_fail_register_chrdev;
	}
	if (major == 0) {
		major = rc; 	/* dynamic */
	}
	rc = 0;
	pr_info("major %d\n", major);

	/* Register sysfs class (for 2.6 or later kernel). */
	plx905x_class = class_create(THIS_MODULE, CLASS_NAME);
	if (IS_ERR(plx905x_class)) {
		rc = PTR_ERR(plx905x_class);
		pr_err("failed to register SysFS class\n");
		goto out_fail_class_create;
	}

#ifdef CONFIG_DEVFS_FS
#if defined(KCOMPAT_HAVE_DEVFS_24)
	/* Register single device with DevFS (for 2.4 kernels). */
	if (!devfs_register(NULL, DEVICE_PREFIX, DEVFS_FL_DEFAULT, major, 0,
			    (S_IFCHR | S_IRUSR | S_IWUSR),
			    (struct file_operations *)&plx905x_fops, NULL)) {
		/* Error number is not accurate. */
		rc = -EINVAL;
	}
#elif defined(KCOMPAT_HAVE_DEVFS_26)
	/* Register single device with DevFS (for early 2.6 kernels). */
	rc = devfs_mk_cdev(MKDEV(major, 0), (S_IFCHR | S_IRUSR | S_IWUSR),
			   DEVICE_PREFIX);
#endif
	if (rc) {
		/* Ignore the error. */
		pr_warn("could not register with DevFS\n");
	} else {
		/* It was registered okay. */
		set_bit(PLX905X_STATUS_DEVNAME_REGISTERED,
			&plx905x_device.status);
	}
#endif

	/*
	 * Register single device with SysFS if supported by the kernel.
	 * Even if not supported, it should get faked by our kernel
	 * compatibility stuff.
	 */
#ifdef KCOMPAT_NO_CLASS_DEVICE
	plx905x_device.csdev =
		device_create(plx905x_class,
			      KCOMPAT_PCI_TO_DEVICE_PTR(plx905x_device.pcidev),
			      MKDEV(major, 0), &plx905x_device, DEVICE_PREFIX);
#else
	plx905x_device.csdev =
		class_device_create(plx905x_class, NULL, MKDEV(major, 0),
				    KCOMPAT_PCI_TO_DEVICE_PTR(plx905x_device.pcidev),
				    DEVICE_PREFIX);
#endif
	if (!plx905x_device.csdev) {
		rc = -ENODEV;
	} else if (IS_ERR(plx905x_device.csdev)) {
		rc = PTR_ERR(plx905x_device.csdev);
		plx905x_device.csdev = NULL;
	} else {
		rc = 0;
	}
	if (rc) {
		pr_err("could not register with SysFS\n");
		goto out_fail_class_device_create;
	}

	pr_info("okay\n");

	return 0;

	if (plx905x_device.csdev) {
#ifdef KCOMPAT_NO_CLASS_DEVICE
		device_unregister(plx905x_device.csdev);
#else
		class_device_unregister(plx905x_device.csdev);
#endif
	}
out_fail_class_device_create:

#ifdef CONFIG_DEVFS_FS
	if (test_bit(PLX905X_STATUS_DEVNAME_REGISTERED,
		     &plx905x_device.status)) {
#if defined(KCOMPAT_HAVE_DEVFS_24)
		/* Unregister single device with DevFS (for 2.4 kernels). */
		devfs_unregister(devfs_find_handle(NULL, DEVICE_PREFIX,
				 major, 0, DEVFS_SPECIAL_CHR, 0));
#elif defined(KCOMPAT_HAVE_DEVFS_26)
		/* Unregister single device with DevFS (early 2.6 kernels). */
		devfs_remove(DEVICE_PREFIX);
#endif
	}
#endif

	/* Unregister sysfs class (for 2.6 kernel). */
	class_destroy(plx905x_class);
out_fail_class_create:

	unregister_chrdev(major, DRIVER_NAME);
out_fail_register_chrdev:

out_fail_eeprom_type:
out_fail_plx_model:

	if (plx905x_device.iospace == IORESOURCE_MEM) {
		iounmap((void *)plx905x_device.u.mmbase);
	}
out_fail_ioremap:

	if (plx905x_device.iospace == IORESOURCE_MEM) {
		release_mem_region(plx905x_device.iophys,
				   plx905x_device.iosize);
	} else {
		release_region(plx905x_device.iophys, plx905x_device.iosize);
	}
#ifndef KCOMPAT_PCI_ENABLE_DEVICE_IS_REF_COUNTED
	/* pci_disable_device only called if request regions successful. */
	pci_disable_device(pcidev);
#endif
out_fail_request_region:

out_fail_plx_resource_check:

#ifdef KCOMPAT_PCI_ENABLE_DEVICE_IS_REF_COUNTED
	/* pci_enable/disable_device is reference counted. */
	pci_disable_device(pcidev);
#endif
out_fail_pci_enable_device:

	/* pci_get_subsys() increments a reference.  Decrement it. */
	pci_dev_put(pcidev);
out_fail_find_device:
	return rc;
}

void __exit
plx905x_module_exit(void)
{
	pr_info("exit\n");

	if (plx905x_device.csdev) {
#ifdef KCOMPAT_NO_CLASS_DEVICE
		device_unregister(plx905x_device.csdev);
#else
		class_device_unregister(plx905x_device.csdev);
#endif
	}

#ifdef CONFIG_DEVFS_FS
	if (test_bit(PLX905X_STATUS_DEVNAME_REGISTERED,
		     &plx905x_device.status)) {
#if defined(KCOMPAT_HAVE_DEVFS_24)
		/* Unregister single device with DevFS (for 2.4 kernels). */
		devfs_unregister(devfs_find_handle(NULL, DEVICE_PREFIX,
				 major, 0, DEVFS_SPECIAL_CHR, 0));
#elif defined(KCOMPAT_HAVE_DEVFS_26)
		/* Unregister single device with DevFS (early 2.6 kernels). */
		devfs_remove(DEVICE_PREFIX);
#endif
	}
#endif

	class_destroy(plx905x_class);
	unregister_chrdev(major, DRIVER_NAME);
	if (plx905x_device.iospace == IORESOURCE_IO) {
		release_region(plx905x_device.iophys, plx905x_device.iosize);
	} else {
		iounmap((void *)plx905x_device.u.mmbase);
		release_mem_region(plx905x_device.iophys,
				plx905x_device.iosize);
	}
	pci_disable_device(plx905x_device.pcidev);
	pci_dev_put(plx905x_device.pcidev);
}

module_init(plx905x_module_init);
module_exit(plx905x_module_exit);
