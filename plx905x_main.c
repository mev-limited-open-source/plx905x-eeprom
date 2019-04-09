/*
 * PLX PCI905x serial EEPROM driver.
 * Currently supports PCI9050, PCI9052 and PCI9054 (revision A or later).
 * Untested support for PCI9030, PCI9056, PCI9060, PCI9080 and PCI9656.
 *
 * Written by Ian Abbott @ MEV Ltd. <ian.abbott@mev.co.uk>.
 * Copyright (C) 2002 MEV Limited.
 *
 *     MEV Ltd.
 *     Suite 8 Baxall Business Centre
 *     Adswood Road Industrial Estate
 *     Stockport
 *     Cheshire
 *     SK3 8LF
 *     UNITED KINGDOM
 *
 *     Tel: +44 (0)161 477 1898
 *     Fax: +44 (0)161 718 3587
 *     WWW: http://www.mev.co.uk/
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

#include <linux/version.h>
#ifndef KERNEL_VERSION
# define KERNEL_VERSION(vers,rel,seq) (((vers)<<16) | ((rel)<<8) | (seq))
#endif

#include <linux/module.h>
#include <linux/kernel.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,1,38)
# include <linux/malloc.h>
#else
# include <linux/slab.h>
#endif

#include <linux/ioport.h>
#include <linux/delay.h>
#include <linux/pci.h>
#include <linux/errno.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/system.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,1,23)
# include <linux/init.h>
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,1,30)
# define __HAVE_SPINLOCK
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,3,0)
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2,2,18)
#   include <linux/spinlock.h>
#  else
#   include <asm/spinlock.h>
#  endif
# else
#  if LINUX_VERSION_CODE >= KERNEL_VERSION(2,3,18)
#   include <linux/spinlock.h>
#  else
#   include <asm/spinlock.h>
#  endif
# endif
#endif

#ifndef set_mb
#define set_mb(var, value) do { var = value; mb(); } while (0)
#endif
#ifndef set_wmb
#define set_wmb(var, value) do { var = value; wmb(); } while (0)
#endif

#ifndef __init
# define __init
#endif
#ifndef __initdata
# define __initdata
#endif
#ifndef __exit
# define __exit
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,1,18)
# define __USE_OLD_SYMTAB__
# define EXPORT_NO_SYMBOLS	register_symtab(NULL)
# define REGISTER_SYMTAB(tab)	register_symtab(tab)
#else
# define REGISTER_SYMTAB(tab)
#endif

#ifndef __MODULE_STRING
# define __MODULE_STRING(s)
#endif
#ifndef MODULE_PARM
# define MODULE_PARM(v,t)
#endif
#ifndef MODULE_PARM_DESC
# define MODULE_PARM_DESC(v,t)
#endif
#ifndef MODULE_AUTHOR
# define MODULE_AUTHOR(n)
#endif
#ifndef MODULE_DESCRIPTION
# define MODULE_DESCRIPTION(d)
#endif
#ifndef MODULE_SUPPORTED_DEVICE
# define MODULE_SUPPORTED_DEVICE(n)
#endif
#ifndef MODULE_LICENSE
# define MODULE_LICENSE(n)
#endif

#ifndef module_init
# define module_init(x)	int init_module(void) {return x(); }
# define module_exit(x)	void cleanup_module(void) { x(); }
#endif

#ifndef SET_MODULE_OWNER
# define SET_MODULE_OWNER(structure)
#endif


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,2,0)
# error "Kernel version is too old for this module"
#endif

#define DRIVER_NAME	"plx905x"
#define DRIVER_DESC	"PLX PCI905x Serial EEPROM Driver"
/* Version 1.02 */
#define DRIVER_VER_MAJOR	1
#define DRIVER_VER_MINOR	2
#define DRIVER_VER_SUF		""
#ifdef PLX905X_DEBUG
#define DRIVER_VER_DEBUG_SUF	"-DEBUG"
#else
#define DRIVER_VER_DEBUG_SUF	""
#endif

#if DRIVER_VER_MINOR < 10
#define DRIVER_VERS	STRINGIFY(DRIVER_VER_MAJOR) \
			".0" STRINGIFY(DRIVER_VER_MINOR) \
			DRIVER_VER_SUF DRIVER_VER_DEBUG_SUF
#else
#define DRIVER_VERS	STRINGIFY(DRIVER_VER_MAJOR) \
			"." STRINGIFY(DRIVER_VER_MINOR) \
			DRIVER_VER_SUF DRIVER_VER_DEBUG_SUF
#endif

#define STRINGIFY(x)	STRINGIFY_(x)
#define STRINGIFY_(x)	#x

#define PLX_VENDOR_ID		0x10B5
#define PLX9030_DEVICE_ID	0x9030
#define PLX9050_DEVICE_ID	0x9050
#define PLX9054_DEVICE_ID	0x9054
#define PLX9056_DEVICE_ID	0x9056
#define PLX9060_DEVICE_ID	0x9060
#define PLX9080_DEVICE_ID	0x9080
#define PLX9656_DEVICE_ID	0x9656
#define DEFAULT_DEVICE_ID	PLX9050_DEVICE_ID

#define PLX9054_PCIHIDR	0x70
#define PLX9054_PCIHIDR_VALUE	0x905410B5
#define PLX9056_PCIHIDR_VALUE	0x905610B5
#define PLX9060_PCIHIDR_VALUE	0x906010B5
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
#define PLX9656_EEMASK	(EE_SK | EE_CS | EE_DI | EE_DO | EE_DOE)

struct plx905x_dev {
	unsigned long iobase;
	unsigned long iophys;
	unsigned long iosize;
	unsigned int iospace;
	unsigned int cntrl;
	unsigned long cntrl_eemask;
	size_t eeprom_size;
	unsigned int eeprom_addr_len;
	struct semaphore sem;
};

int __init plx905x_module_init(void);
void __exit plx905x_module_exit(void);

static int __init get_pci_region(struct pci_dev *dev, int bar,
		u32 *addr, u32 *size, unsigned int *flags);
static struct pci_dev __init *my_pci_find_subsys(
		unsigned int vendor, unsigned int device,
		unsigned int ss_vendor, unsigned int ss_device,
		const struct pci_dev *from);

static loff_t plx905x_llseek(struct file *filp, loff_t off, int whence);
static ssize_t plx905x_read(struct file *filp, char *buf, size_t count,
		loff_t *f_pos);
static ssize_t plx905x_write(struct file *filp, const char *buf, size_t count,
		loff_t *f_pos);
static int plx905x_open(struct inode *inode, struct file *filp);
static int plx905x_release(struct inode *inode, struct file *filp);

static int eeprom_cmd_read_word(struct plx905x_dev *dev, unsigned int offset,
		u16 *data);
static int eeprom_cmd_write_word(struct plx905x_dev *dev, unsigned int offset,
		u16 data);
static int eeprom_cmd_write_enable(struct plx905x_dev *dev);
static int eeprom_cmd_write_disable(struct plx905x_dev *dev);
static void eeprom_init(struct plx905x_dev *dev);
static void eeprom_start_cmd(struct plx905x_dev *dev, u32 *cntrl);
static void eeprom_put_bits(struct plx905x_dev *dev, u32 *cntrl,
		unsigned int bits, unsigned int nbits);
static void eeprom_end_cmd(struct plx905x_dev *dev, u32 *cntrl);
static int eeprom_wait_prog(struct plx905x_dev *dev);

static u32 cntrl_read(struct plx905x_dev *dev);
static void cntrl_write(struct plx905x_dev *dev, u32 data);

static int major = 0;
MODULE_PARM(major, "i");
MODULE_PARM_DESC(major, "Major device number");

static int bus = 0;
MODULE_PARM(bus, "i");
MODULE_PARM_DESC(bus, "PCI bus number (optional)");

static int slot = 0;
MODULE_PARM(slot, "i");
MODULE_PARM_DESC(slot, "PCI slot number (optional)");

static unsigned int vendor = PCI_ANY_ID;
MODULE_PARM(vendor, "i");
MODULE_PARM_DESC(vendor, "PCI Vendor ID (optional)");

static unsigned int device = PCI_ANY_ID;
MODULE_PARM(device, "i");
MODULE_PARM_DESC(device, "PCI Device ID (optional)");

static unsigned int subvendor = PCI_ANY_ID;
MODULE_PARM(subvendor, "i");
MODULE_PARM_DESC(subvendor, "PCI Subsystem Vendor ID (optional)");

static unsigned int subdevice = PCI_ANY_ID;
MODULE_PARM(subdevice, "i");
MODULE_PARM_DESC(subdevice, "PCI Subsystem Device ID (optional)");

static int instance = 0;
MODULE_PARM(instance, "i");
MODULE_PARM_DESC(instance,
		"Instance of PCI Vendor/Device/Subsystem IDs (default 0)");

static int eeprom = 0;
MODULE_PARM(eeprom, "i");
MODULE_PARM_DESC(eeprom,
		"EEPROM type 46 (1024-bit), 56 (2048-bit), 66 (4096-bit) "
		"(default depends on PLX device)");

static int plx=0;
MODULE_PARM(plx, "i");
MODULE_PARM_DESC(plx,
		"PLX chip type 0x9030, 0x9050, 0x9052 (equivalent to 0x9050), "
		"0x9054, 0x9056, 0x9060, 0x9080, 0x9656 (default 0x9050)");

static struct plx905x_dev plx905x_device = {0};
static int plx905x_registered_chrdev = 0;
static struct file_operations plx905x_fops = {
	llseek: plx905x_llseek,
	read: plx905x_read,
	write: plx905x_write,
	open: plx905x_open,
	release: plx905x_release,
};

int __init
plx905x_module_init(void)
{
	struct pci_dev *pcidev = NULL;
	int inst = 0;
	u8 dev_header_type = PCI_HEADER_TYPE_NORMAL;
	unsigned int dev_vendor = PCI_ANY_ID;
	unsigned int dev_device = PCI_ANY_ID;
	unsigned int dev_subvendor = PCI_ANY_ID;
	unsigned int dev_subdevice = PCI_ANY_ID;
	u32 baraddr, barsize;
	unsigned int barflags;
	unsigned int dev_bus = 0;
	unsigned int dev_slot = 0;
	u16 tempu16;
	int retval = 0;

	EXPORT_NO_SYMBOLS;

	printk(KERN_INFO "%s: %s, version %s\n",
			DRIVER_NAME, DRIVER_DESC, DRIVER_VERS);
	SET_MODULE_OWNER(&plx905x_fops);
	vendor = vendor;
	device = device;
	subvendor = subvendor;
	subdevice = subdevice;
	if (bus || slot) {
		dev_bus = bus;
		dev_slot = slot;
		pcidev = pci_find_slot(dev_bus, PCI_DEVFN(dev_slot,0));
		if (pcidev) {
			pci_read_config_word(pcidev, PCI_VENDOR_ID,
					&tempu16);
			dev_vendor = tempu16;
			pci_read_config_word(pcidev, PCI_DEVICE_ID,
					&tempu16);
			dev_device = tempu16;
			pci_read_config_byte(pcidev, PCI_HEADER_TYPE,
					&dev_header_type);
			/* N.B. Multifunction not supported. */
			if (dev_header_type == PCI_HEADER_TYPE_NORMAL) {
				pci_read_config_word(pcidev,
						PCI_SUBSYSTEM_VENDOR_ID,
						&tempu16);
				dev_subvendor = tempu16;
				pci_read_config_word(pcidev, PCI_SUBSYSTEM_ID,
						&tempu16);
				dev_subdevice = tempu16;
			}
			/* Match against PCI IDs if specified. */
			if ((vendor != PCI_ANY_ID)
					|| (device != PCI_ANY_ID)) {
				if ((dev_vendor != vendor)
						|| (dev_device != device)) {
					pcidev = NULL;
				}
			}
			if (dev_header_type != PCI_HEADER_TYPE_NORMAL) {
				pcidev = NULL;
			}
			if ((subvendor != PCI_ANY_ID)
					|| (subdevice != PCI_ANY_ID)) {
				if ((dev_subvendor != subvendor)
						|| (dev_subdevice
							!= subdevice)) {
					pcidev = NULL;
				}
			}
			if (pcidev) {
				++inst;
			}
		}
	} else {
		if ((vendor == PCI_ANY_ID) && (device == PCI_ANY_ID)) {
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
		while ((pcidev = my_pci_find_subsys(vendor, device, subvendor,
						subdevice, pcidev)) != NULL) {
			if (++inst > instance) {
				/*
				 * Found required instance.
				 */
				dev_bus = pcidev->bus->number;
				dev_slot = PCI_SLOT(pcidev->devfn);
				dev_vendor = vendor;
				dev_device = device;
				dev_header_type = PCI_HEADER_TYPE_NORMAL;
				pci_read_config_word(pcidev,
						PCI_SUBSYSTEM_VENDOR_ID,
						&tempu16);
				dev_subvendor = tempu16;
				pci_read_config_word(pcidev, PCI_SUBSYSTEM_ID,
						&tempu16);
				dev_subdevice = tempu16;
				break;
			}
		}
	}

	if (!pcidev) {
		printk(KERN_ERR DRIVER_NAME ": Could not find PCI device\n");
		return -ENODEV;
	}
	pci_enable_device(pcidev);
	printk(KERN_INFO DRIVER_NAME ": %02x:%02x %04x:%04x (%04x:%04x) (%d): ",
			dev_bus, dev_slot, dev_vendor, dev_device,
			dev_subvendor, dev_subdevice, inst - 1);
	/* Try to confirm that it really is a supported PLX chip. */
	/* Check for local configuration registers in PCIBAR0 (memory). */
	{
		int bar0ok = 0, bar1ok = 0;
		int bar0present = 0, bar1present = 0;

		/* Most of these checks are for PLX PCI9050 and PCI9052 */
		get_pci_region(pcidev, PCI_BASE_ADDRESS_1,
				&baraddr, &barsize, &barflags);
		bar1present = (barsize != 0);
		bar1ok = ((barsize == 128) && (barflags == 1));
		get_pci_region(pcidev, PCI_BASE_ADDRESS_0,
				&baraddr, &barsize, &barflags);
		bar0present = (barsize != 0);
		bar0ok = ((barsize == 128) && (barflags == 0));
		if (bar0present && (barsize == 256) && (barflags == 0)) {
			/* Assume it is PLX PCI9054 */
		} else if ((bar0present && !bar0ok)
				|| (bar1present && !bar1ok)
				|| !(bar0present || bar1present)) {
			printk("not PLX\n");
			return -ENODEV;
		} else {
			/* Assume it is PLX PCI9050/9052 */
			unsigned model;
			u8 rev;

			/* Check 'plx' parameter for compatibility. */
			if ((plx != 0) && (plx != 0x9050) && (plx != 9050)
					&& (plx != 0x9052) && (plx != 9052)) {
				printk("not specified PLX model\n");
				return -ENODEV;
			}
			/* Check PCI revision to determine model. */
			pci_read_config_byte(pcidev, PCI_REVISION_ID, &rev);
			if (rev > 2) {
				printk("not PLX PCI9050/9052 (revision is >2)\n");
				return -ENODEV;
			}
			model = (rev < 2) ? 0x9050 : 0x9052;
			printk("PCI%X rev %02X: ", model, rev);

			/* Check 'eeprom' parameter is valid (if specified). */
			if ((eeprom != 0) && (eeprom != 46) && (eeprom != 128)
					&& (eeprom != 1024)) {
				printk("invalid EEPROM type for PLX PCI9050/9052\n");
				return -ENODEV;
			}
		}
		if (!bar0present) {
			get_pci_region(pcidev, PCI_BASE_ADDRESS_1,
					&baraddr, &barsize, &barflags);
		}
	}
	/* Initialize device. */
	sema_init(&plx905x_device.sem, 1);
	plx905x_device.eeprom_size = CS46_EEPROM_SIZE;
	plx905x_device.eeprom_addr_len = CS46_EEPROM_ADDR_LEN;
	plx905x_device.cntrl = PLX9050_CNTRL;	/* Change later for PCI9054 */
	plx905x_device.cntrl_eemask = PLX9050_EEMASK;
	plx905x_device.iospace = (barflags & PCI_BASE_ADDRESS_SPACE);
	plx905x_device.iophys = baraddr;
	plx905x_device.iosize = barsize;
	if (plx905x_device.iospace == PCI_BASE_ADDRESS_SPACE_IO) {
		retval = check_region(plx905x_device.iophys,
				plx905x_device.iosize);
		if (retval) {
			printk("I/O port busy\n");
			return retval;
		}
		request_region(plx905x_device.iophys, plx905x_device.iosize,
				DRIVER_NAME);
		plx905x_device.iobase = plx905x_device.iophys;
	} else {
		retval = check_mem_region(plx905x_device.iophys,
				plx905x_device.iosize);
		if (retval) {
			printk("I/O mem busy\n");
			return retval;
		}
		request_mem_region(plx905x_device.iophys, plx905x_device.iosize,
				DRIVER_NAME);
		plx905x_device.iobase
			= (unsigned long)ioremap_nocache(plx905x_device.iophys,
					plx905x_device.iosize);
		if (plx905x_device.iobase == 0) {
			printk("cannot map I/O mem\n");
			release_mem_region(plx905x_device.iophys,
					plx905x_device.iosize);
			return -ENOMEM;
		}
		/* Check for PLX PCI9030/9054/9056/9060/9080/9656 */
		if (plx905x_device.iosize == 256) {
			unsigned model;
			int model_okay = 0;

			if ((plx == 0x9030) || (plx == 9030)) {
				model = 0x9030;
				model_okay = 1;
				printk("PCI9030");
			} else {
				u32 hidr;
				u8 hrev;
				int hrev_okay = 0;

				plx905x_device.cntrl = PLX9054_CNTRL;
				hidr = readl(plx905x_device.iobase + PLX9054_PCIHIDR);
				hrev = readb(plx905x_device.iobase + PLX9054_PCIHREV);
				/* Check for supported type and revision. */
				/* Check compatibility with 'plx' parameter. */
				if (plx == 0) {
					model_okay = 1;
				}
				switch (hidr) {
				case PLX9054_PCIHIDR_VALUE:
					model = 0x9054;
					if ((plx == 0x9054) || (plx == 9054)) {
						model_okay = 1;
					}
					if (hrev >= 0x0A) {
						hrev_okay = 1;
					}
					break;
				case PLX9056_PCIHIDR_VALUE:
					model = 0x9056;
					if ((plx == 0x9056) || (plx == 9056)) {
						model_okay = 1;
					}
					hrev_okay = 1;
					break;
				case PLX9060_PCIHIDR_VALUE:
					model = 0x9060;
					if ((plx == 0x9060) || (plx == 9060)) {
						model_okay = 1;
					}
					hrev_okay = 1;
					break;
				case PLX9080_PCIHIDR_VALUE:
					model = 0x9080;
					if ((plx == 0x9080) || (plx == 9080)) {
						model_okay = 1;
					}
					hrev_okay = 1;
					break;
				case PLX9656_PCIHIDR_VALUE:
					plx905x_device.cntrl_eemask = PLX9656_EEMASK;
					model = 0x9656;
					if ((plx == 0x9656) || (plx == 9656)) {
						model_okay = 1;
					}
					if (hrev >= 0xAA) {
						hrev_okay = 1;
					}
					break;
				default:
					printk("not PLX\n");
					release_mem_region(plx905x_device.iophys,
							plx905x_device.iosize);
					return -ENODEV;
				}
				if (!model_okay) {
					printk("not specified PLX\n");
					release_mem_region(plx905x_device.iophys,
							plx905x_device.iosize);
					return -ENODEV;
				}
				printk("PCI%X rev %02X: ", (unsigned)(hidr >> 16),
						(unsigned)hrev);
				if (!hrev_okay) {
					printk("bad revision\n");
					release_mem_region(plx905x_device.iophys,
							plx905x_device.iosize);
					return -ENODEV;
				}
			}
			/* Check EEPROM type (if specified). */
			switch (model) {
			case 0x9030:
			case 0x9054:
			case 0x9056:
			case 0x9656:
				switch (eeprom) {
				case 56: /* CS56 */
				case 256:
				case 2048:
				case 0:	/* default to CS56 */
					plx905x_device.eeprom_size
						= CS56_EEPROM_SIZE;
					plx905x_device.eeprom_addr_len
						= CS56_EEPROM_ADDR_LEN;
					break;
				case 66: /* CS66 */
				case 512:
				case 4096:
					plx905x_device.eeprom_size
						= CS66_EEPROM_SIZE;
					plx905x_device.eeprom_addr_len
						= CS66_EEPROM_ADDR_LEN;
					break;
				default: /* invalid */
					printk("invalid EEPROM type for PLX PCI9030/9054/9056/9656\n");
					release_mem_region(
							plx905x_device.iophys,
							plx905x_device.iosize);
					return -ENODEV;
				}
				break;
			case 0x9060:
			case 0x9080:
				/* No default EEPROM size for PCI9060/9080 */
				switch (eeprom) {
				case 46: /* CS46 */
				case 128:
				case 1024:
					plx905x_device.eeprom_size
						= CS46_EEPROM_SIZE;
					plx905x_device.eeprom_addr_len
						= CS46_EEPROM_ADDR_LEN;
					break;
				case 56: /* CS56 */
				case 256:
				case 2048:
					plx905x_device.eeprom_size
						= CS56_EEPROM_SIZE;
					plx905x_device.eeprom_addr_len
						= CS56_EEPROM_ADDR_LEN;
					break;
				default: /* invalid */
					printk("must specify valid EEPROM type for PLX PCI9060/9080\n");
					release_mem_region(
							plx905x_device.iophys,
							plx905x_device.iosize);
					return -ENODEV;
				}
				break;
			default:
				printk("bug %s[%ld]\n",
						__FILE__, (long)__LINE__);
				release_mem_region(plx905x_device.iophys,
						plx905x_device.iosize);
				return -ENODEV;
			}
		}
	}
	/* Try to register character device driver. */
	retval = register_chrdev(major, DRIVER_NAME, &plx905x_fops);
	if (retval < 0) {
		printk("cannot get major number\n");
		plx905x_module_exit();
		return retval;
	}
	plx905x_registered_chrdev = 1;
	if (major == 0) major = retval; 	/* dynamic */
	retval = 0;
	printk("major %d: ", major);

	printk("okay\n");

	return 0;
}

static struct pci_dev * __init
my_pci_find_subsys(unsigned int vendor, unsigned int device,
		unsigned int ss_vendor, unsigned int ss_device,
		const struct pci_dev *from)
{
	struct pci_dev *dev = (struct pci_dev *)from;
	u8 dev_ht;
	u16 tempu16;

	while ((dev = pci_find_device(vendor, device, dev)) != NULL) {
		pci_read_config_byte(dev, PCI_HEADER_TYPE, &dev_ht);
		/* N.B. Multifunction not supported. */
		if (dev_ht != PCI_HEADER_TYPE_NORMAL)
			continue;
		if (ss_vendor != PCI_ANY_ID) {
			pci_read_config_word(dev, PCI_SUBSYSTEM_VENDOR_ID,
					&tempu16);
			if (ss_vendor != tempu16)
				continue;
		}
		if (ss_device != PCI_ANY_ID) {
			pci_read_config_word(dev, PCI_SUBSYSTEM_ID, &tempu16);
			if (ss_device != tempu16)
				continue;
		}
		break;

	}
	return dev;
}

static int __init
get_pci_region(struct pci_dev *dev, int bar,
		u32 *addr, u32 *size, unsigned int *flags)
{
	int ret;
	unsigned long intflags;
	u32 curr, mask;

	*addr = 0;
	*size = 0;
	*flags = 0;
	ret = pci_read_config_dword(dev, bar, &curr);
	if (ret < 0) return ret;
	save_flags(intflags); cli();
	pci_write_config_dword(dev, bar, ~0);
	pci_read_config_dword(dev, bar, &mask);
	ret = pci_write_config_dword(dev, bar, curr);
	restore_flags(intflags);
	if (mask != 0 && mask != 0xFFFFFFFF) {
		if (curr == 0xFFFFFFFF)
			curr = 0;
		if ((curr & PCI_BASE_ADDRESS_SPACE)
				== PCI_BASE_ADDRESS_SPACE_MEMORY) {
			*addr = curr & PCI_BASE_ADDRESS_MEM_MASK;
			*flags = curr & ~PCI_BASE_ADDRESS_MEM_MASK;
			mask &= PCI_BASE_ADDRESS_MEM_MASK;
		} else {
			*addr = curr & PCI_BASE_ADDRESS_IO_MASK;
			*flags = curr & ~PCI_BASE_ADDRESS_IO_MASK;
			mask &= PCI_BASE_ADDRESS_IO_MASK & 0xFFFF;
		}
		*size = mask & ~(mask - 1);
	}
	return ret;
}

void __exit
plx905x_module_exit(void)
{
	printk(KERN_INFO DRIVER_NAME ": exit\n");

	if (plx905x_registered_chrdev) {
		unregister_chrdev(major, DRIVER_NAME);
	}
	if (plx905x_device.iospace == PCI_BASE_ADDRESS_SPACE_IO) {
		release_region(plx905x_device.iophys, plx905x_device.iosize);
	} else {
		if (plx905x_device.iobase)
			iounmap((void *)plx905x_device.iobase);
		release_mem_region(plx905x_device.iophys,
				plx905x_device.iosize);
	}
}

static int
plx905x_open(struct inode *inode, struct file *filp)
{
	struct plx905x_dev *dev = &plx905x_device;

	filp->private_data = dev;
	MOD_INC_USE_COUNT;
	if (down_interruptible(&dev->sem)) {
		MOD_DEC_USE_COUNT;
		return -ERESTARTSYS;
	}
	eeprom_init(dev);
	up(&dev->sem);
	return 0;
}

static int
plx905x_release(struct inode *inode, struct file *filp)
{
	MOD_DEC_USE_COUNT;
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
	if (down_interruptible(&dev->sem)) {
		return -ERESTARTSYS;
	}
	if (!access_ok(VERIFY_WRITE, buf, count)) {
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
	up(&dev->sem);
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
	if (down_interruptible(&dev->sem)) {
		return -ERESTARTSYS;
	}
	if (!access_ok(VERIFY_READ, buf, count)) {
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
	up(&dev->sem);
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

static u32
cntrl_read(struct plx905x_dev *dev)
{
	if (dev->iospace == PCI_BASE_ADDRESS_SPACE_IO) {
		return inl(dev->iobase + dev->cntrl);
	} else {
		return readl(dev->iobase + dev->cntrl);
	}

}

static void
cntrl_write(struct plx905x_dev *dev, u32 data)
{
	if (dev->iospace == PCI_BASE_ADDRESS_SPACE_IO) {
		outl(data, dev->iobase + dev->cntrl);
	} else {
		writel(data, dev->iobase + dev->cntrl);
	}
}

module_init(plx905x_module_init);
module_exit(plx905x_module_exit);

MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_AUTHOR("Ian Abbott <ian.abbott@mev.co.uk>");
MODULE_LICENSE("GPL");
