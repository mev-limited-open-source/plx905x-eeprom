/*
 * MEV common driver compatibility code.
 *
 * Most of this code is taken from or based on source code from various
 * versions of the Linux kernel.
 *
 * Other parts written by MEV Ltd.
 */

/*
 * kcompat_pci.h
 *
 * Kernel compatibility stuff for various PCI card drivers.
 * Also include kcompat.h.
 *
 * Define KCOMPAT_PCI_DEFINE_PCI_POOL macro before including this file
 * to allow this file to define the functions pci_pool_create,
 * pci_pool_destroy, pci_pool_alloc and pci_pool_free with external linkage.
 * This is only done for kernel versions prior to 2.4.4.  The
 * KCOMPAT_PCI_DEFINE_PCI_POOL macro should only be defined in one .c file
 * of the driver.
 */

#ifndef KCOMPAT_PCI_H__INCLUDED
#define KCOMPAT_PCI_H__INCLUDED

#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#include <linux/config.h>
#endif
#include <linux/types.h>
#include <linux/pci.h>
#include <linux/mm.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,53)
#include <linux/dma-mapping.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
#error "*************************************************************"
#error " Sorry, this driver requires kernel version 2.4.0 or higher."
#error "*************************************************************"
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,22)
#define pci_name(pdev) ((pdev)->slot_name)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
/* Use pci_module_init instead of pci_register_driver. */
#ifdef pci_register_driver
#undef pci_register_driver
#endif
#define pci_register_driver	pci_module_init
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
static inline int pci_domain_nr(struct pci_bus *bus) { return 0; }
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,4)

struct pci_pool {	/* the pool */
	struct list_head	page_list;
	spinlock_t		lock;
	size_t			blocks_per_page;
	size_t			size;
	int			flags;
	struct pci_dev		*dev;
	size_t			allocation;
	char			name [32];
	wait_queue_head_t	waitq;
};

struct pci_page {	/* cacheable header for 'allocation' bytes */
	struct list_head	page_list;
	void			*vaddr;
	dma_addr_t		dma;
	unsigned long		bitmap [0];
};

/* These functions won't exist unless KCOMPAT_PCI_DEFINE_PCI_POOL is
 * defined in one of the .c files before including kcompat_pci.h. */
struct pci_pool *pci_pool_create (const char *name, struct pci_dev *dev,
		size_t size, size_t align, size_t allocation, int flags);
void pci_pool_destroy (struct pci_pool *pool);

void *pci_pool_alloc (struct pci_pool *pool, int flags, dma_addr_t *handle);
void pci_pool_free (struct pci_pool *pool, void *vaddr, dma_addr_t addr);

#ifdef KCOMPAT_PCI_DEFINE_PCI_POOL
/* Define pci_pool_create etc. with external linkage. */

#include <linux/config.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/pci.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/bitops.h>

#include <asm/page.h>
#include <asm/dma.h>	/* isa_dma_bridge_buggy */

struct pci_pool *
pci_pool_create (const char *name, struct pci_dev *pdev,
	size_t size, size_t align, size_t allocation, int flags)
{
	struct pci_pool		*retval;

	if (align == 0)
		align = 1;
	if (size == 0)
		return 0;
	else if (size < align)
		size = align;
	else if ((size % align) != 0) {
		size += align + 1;
		size &= ~(align - 1);
	}

	if (allocation == 0) {
		if (PAGE_SIZE < size)
			allocation = size;
		else
			allocation = PAGE_SIZE;
		// FIXME: round up for less fragmentation
	} else if (allocation < size)
		return 0;

	if (!(retval = kmalloc (sizeof *retval, flags)))
		return retval;

#ifdef	CONFIG_PCIPOOL_DEBUG
	flags |= SLAB_POISON;
#endif

	strncpy (retval->name, name, sizeof retval->name);
	retval->name [sizeof retval->name - 1] = 0;

	retval->dev = pdev;
	INIT_LIST_HEAD (&retval->page_list);
	spin_lock_init (&retval->lock);
	retval->size = size;
	retval->flags = flags;
	retval->allocation = allocation;
	retval->blocks_per_page = allocation / size;
	init_waitqueue_head (&retval->waitq);

#ifdef CONFIG_PCIPOOL_DEBUG
	printk (KERN_DEBUG "pcipool create %s/%s size %d, %d/page (%d alloc)\n",
		pdev ? pdev->slot_name : NULL, retval->name, size,
		retval->blocks_per_page, allocation);
#endif

	return retval;
}

static struct pci_page *
kcompat_pci_pool_alloc_page (struct pci_pool *pool, int mem_flags)
{
	struct pci_page	*page;
	int		mapsize;

	mapsize = pool->blocks_per_page;
	mapsize = (mapsize + BITS_PER_LONG - 1) / BITS_PER_LONG;
	mapsize *= sizeof (long);

	page = (struct pci_page *) kmalloc (mapsize + sizeof *page, mem_flags);
	if (!page)
		return 0;
	page->vaddr = pci_alloc_consistent (pool->dev,
				pool->allocation, &page->dma);
	if (page->vaddr) {
		memset (page->bitmap, ~0, mapsize);	// bit set == free
		if (pool->flags & SLAB_POISON)
			memset (page->vaddr, POOL_POISON_BYTE, pool->allocation);
		list_add (&page->page_list, &pool->page_list);
	} else {
		kfree (page);
		page = 0;
	}
	return page;
}

static inline int
kcompat_pci_is_page_busy (int blocks, unsigned long *bitmap)
{
	while (blocks > 0) {
		if (*bitmap++ != ~0)
			return 1;
		blocks -= BITS_PER_LONG;
	}
	return 0;
}

static void
kcompat_pci_pool_free_page (struct pci_pool *pool, struct pci_page *page)
{
	dma_addr_t	dma = page->dma;

	if (pool->flags & SLAB_POISON)
		memset (page->vaddr, POOL_POISON_BYTE, pool->allocation);
	pci_free_consistent (pool->dev, pool->allocation, page->vaddr, dma);
	list_del (&page->page_list);
	kfree (page);
}

void
pci_pool_destroy (struct pci_pool *pool)
{
	unsigned long		flags;

#ifdef CONFIG_PCIPOOL_DEBUG
	printk (KERN_DEBUG "pcipool destroy %s/%s\n",
		pool->dev ? pool->dev->slot_name : NULL,
		pool->name);
#endif

	spin_lock_irqsave (&pool->lock, flags);
	while (!list_empty (&pool->page_list)) {
		struct pci_page		*page;
		page = list_entry (pool->page_list.next,
				struct pci_page, page_list);
		if (kcompat_pci_is_page_busy (pool->blocks_per_page, page->bitmap)) {
			printk (KERN_ERR "pci_pool_destroy %s/%s, %p busy\n",
				pool->dev ? pool->dev->slot_name : NULL,
				pool->name, page->vaddr);
			/* leak the still-in-use consistent memory */
			list_del (&page->page_list);
			kfree (page);
		} else
			kcompat_pci_pool_free_page (pool, page);
	}
	spin_unlock_irqrestore (&pool->lock, flags);
	kfree (pool);
}

void *
pci_pool_alloc (struct pci_pool *pool, int mem_flags, dma_addr_t *handle)
{
	unsigned long		flags;
	struct list_head	*entry;
	struct pci_page		*page;
	int			map, block;
	size_t			offset;
	void			*retval;

restart:
	spin_lock_irqsave (&pool->lock, flags);
	list_for_each (entry, &pool->page_list) {
		int		i;
		page = list_entry (entry, struct pci_page, page_list);
		/* only cachable accesses here ... */
		for (map = 0, i = 0;
				i < pool->blocks_per_page;
				i += BITS_PER_LONG, map++) {
			if (page->bitmap [map] == 0)
				continue;
			block = ffs (page->bitmap [map]);
			if ((i + block) <= pool->blocks_per_page) {
				block--;
				clear_bit (block, &page->bitmap [map]);
				offset = (BITS_PER_LONG * map) + block;
				offset *= pool->size;
				goto ready;
			}
		}
	}
	if (!(page = kcompat_pci_pool_alloc_page (pool, mem_flags))) {
		if (mem_flags == SLAB_KERNEL) {
			DECLARE_WAITQUEUE (wait, current);

			current->state = TASK_INTERRUPTIBLE;
			add_wait_queue (&pool->waitq, &wait);
			spin_unlock_irqrestore (&pool->lock, flags);

			schedule_timeout (POOL_TIMEOUT_JIFFIES);

			current->state = TASK_RUNNING;
			remove_wait_queue (&pool->waitq, &wait);
			goto restart;
		}
		retval = 0;
		goto done;
	}

	clear_bit (0, &page->bitmap [0]);
	offset = 0;
ready:
	retval = offset + page->vaddr;
	*handle = offset + page->dma;
done:
	spin_unlock_irqrestore (&pool->lock, flags);
	return retval;
}

static struct pci_page *
kcompat_pci_pool_find_page (struct pci_pool *pool, dma_addr_t dma)
{
	unsigned long		flags;
	struct list_head	*entry;
	struct pci_page		*page;

	spin_lock_irqsave (&pool->lock, flags);
	list_for_each (entry, &pool->page_list) {
		page = list_entry (entry, struct pci_page, page_list);
		if (dma < page->dma)
			continue;
		if (dma < (page->dma + pool->allocation))
			goto done;
	}
	page = 0;
done:
	spin_unlock_irqrestore (&pool->lock, flags);
	return page;
}

void
pci_pool_free (struct pci_pool *pool, void *vaddr, dma_addr_t dma)
{
	struct pci_page		*page;
	unsigned long		flags;
	int			map, block;

	if ((page = kcompat_pci_pool_find_page (pool, dma)) == 0) {
		printk (KERN_ERR "pci_pool_free %s/%s, %p/%x (bad dma)\n",
			pool->dev ? pool->dev->slot_name : NULL,
			pool->name, vaddr, dma);
		return;
	}
#ifdef	CONFIG_PCIPOOL_DEBUG
	if (((dma - page->dma) + (void *)page->vaddr) != vaddr) {
		printk (KERN_ERR "pci_pool_free %s/%s, %p (bad vaddr)/%x\n",
			pool->dev ? pool->dev->slot_name : NULL,
			pool->name, vaddr, dma);
		return;
	}
#endif

	block = dma - page->dma;
	block /= pool->size;
	map = block / BITS_PER_LONG;
	block %= BITS_PER_LONG;

#ifdef	CONFIG_PCIPOOL_DEBUG
	if (page->bitmap [map] & (1 << block)) {
		printk (KERN_ERR "pci_pool_free %s/%s, dma %x already free\n",
			pool->dev ? pool->dev->slot_name : NULL,
			pool->name, dma);
		return;
	}
#endif
	if (pool->flags & SLAB_POISON)
		memset (vaddr, POOL_POISON_BYTE, pool->size);

	spin_lock_irqsave (&pool->lock, flags);
	set_bit (block, &page->bitmap [map]);
	if (waitqueue_active (&pool->waitq))
		wake_up (&pool->waitq);
	else if (!kcompat_pci_is_page_busy (pool->blocks_per_page, page->bitmap))
		kcompat_pci_pool_free_page (pool, page);
	spin_unlock_irqrestore (&pool->lock, flags);
}

#endif	/* KCOMPAT_PCI_DEFINE_PCI_POOL */
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,4) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,41)
/* Make pci_pool_create consistent with 2.6 kernel (no 'flags' parameter) */
static inline struct pci_pool *
kcompat_pci_pool_create(const char *name, struct pci_dev *pdev, size_t size,
		size_t align, size_t allocation)
{
	return pci_pool_create(name, pdev, size, align, allocation, 0);
}
#undef pci_pool_create
#define pci_pool_create(name, pdev, size, align, allocation) \
	kcompat_pci_pool_create(name, pdev, size, align, allocation)
#endif

/* pci_disable_device was added in 2.4.4 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,4)
#ifndef HAVE_PCI_DISABLE_DEVICE	/* macro removed in 2.4.6. tested for backport. */
static inline void pci_disable_device(struct pci_dev *dev)
{
 	u16 pci_command;

	pci_read_config_word(dev, PCI_COMMAND, &pci_command);
	if (pci_command & PCI_COMMAND_MASTER) {
		pci_command &= ~PCI_COMMAND_MASTER;
		pci_write_config_word(dev, PCI_COMMAND, pci_command);
	}
}
#endif
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,4) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,13)
static inline dma_addr_t pci_map_page(struct pci_dev *hwdev, struct page *page,
		unsigned long offset, size_t size, int direction)
{
	return pci_map_single(hwdev, (char *)page_address(page) + offset,
			size, direction);
}

static inline void pci_unmap_page(struct pci_dev *hwdev, dma_addr_t dma_address,
		size_t size, int direction)
{
	pci_unmap_single(hwdev, dma_address, size, direction);
}
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,19) */

#ifndef offset_in_page	/* defined in 2.6.0 linux/mm.h */
#define offset_in_page(p)	((unsigned long)(p) & ~PAGE_MASK)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,53)
#include <linux/cache.h>

#ifndef L1_CACHE_SHIFT_MAX
#if defined(CONFIG_X86)
#define L1_CACHE_SHIFT_MAX	7
#else
#define L1_CACHE_SHIFT_MAX	L1_CACHE_SHIFT
#endif
#endif	/* L1_CACHE_SHIFT_MAX */

static inline int
dma_get_cache_alignment(void)
{
	return (1 << L1_CACHE_SHIFT_MAX);
}

#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,5,53) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
static inline int
pci_dma_mapping_error(dma_addr_t dma_addr)
{
	return 0;	/* Cheat! */
}

static inline void
pci_dma_sync_single_for_cpu(struct pci_dev *hwdev, dma_addr_t dma_handle,
		size_t size, int direction)
{
	if (direction == PCI_DMA_FROMDEVICE) {
		pci_dma_sync_single(hwdev, dma_handle, size, direction);
	}
}

static inline void
pci_dma_sync_single_for_device(struct pci_dev *hwdev, dma_addr_t dma_handle,
		size_t size, int direction)
{
	if (direction == PCI_DMA_TODEVICE) {
		pci_dma_sync_single(hwdev, dma_handle, size, direction);
	}
}
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
/* Make pci_dma_mapping_error() consistent with kernel version 2.6.27
 * (add 'pdev' parameter).
 */
static inline int kcompat_pci_dma_mapping_error(struct pci_dev *pdev,
		dma_addr_t dma_addr)
{
	return pci_dma_mapping_error(dma_addr);
}
#undef pci_dma_mapping_error
#define pci_dma_mapping_error(pdev, dma_addr) \
	kcompat_pci_dma_mapping_error(pdev, dma_addr)
#endif

/* Define a macro to get a pointer to the 'struct device' embedded in a 
 * 'struct pci_dev'.  For kernel versions prior to 2.5.3, the macro
 * yields a NULL pointer.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,3)
struct device;
#define KCOMPAT_PCI_TO_DEVICE_PTR(pdev)	((struct device *)0)
#else
#define KCOMPAT_PCI_TO_DEVICE_PTR(pdev)	(&(pdev)->dev)
#endif

/* Define Vital Product Data macros missing from 2.4 kernels. */
#ifndef PCI_VPD_ADDR
#define PCI_VPD_ADDR	2	/* Address to access (15 bits!) */
#define PCI_VPD_ADDR_MASK	0x7fff	/* Address mask */
#define PCI_VPD_ADDR_F	0x8000	/* Set 0 for write, 1 for read.  Inverted on completion. */
#define PCI_VPD_DATA	4	/* 32-bits of data set or returned here. */
#endif

/* Define PCI_DEVICE() and PCI_DEVICE_CLASS() macros missing prior to 2.4.23. */
#ifndef PCI_DEVICE
#define PCI_DEVICE(vend, dev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID
#endif
#ifndef PCI_DEVICE_CLASS
#define PCI_DEVICE_CLASS(dev_class, dev_class_mask) \
	.class = (dev_class), .class_mask = (dev_class_mask), \
	.vendor = PCI_ANY_ID, .device = PCI_ANY_ID, \
	.subvendor = PCI_ANY_ID, .subdevice = PCI_ANY_ID
#endif
/* Define PCI_DEVICE_SUB() macro missing prior to 3.8. */
#ifndef PCI_DEVICE_SUB
#define PCI_DEVICE_SUB(vend, dev, subvend, subdev) \
	.vendor = (vend), .device = (dev), \
	.subvendor = (subvend), .subdevice = (subdev)
#endif
/* Define PCI_VDEVICE() macro missing prior to 2.6.20. */
#ifndef PCI_VDEVICE
#define PCI_VDEVICE(vendor, device)		\
	PCI_VENDOR_ID_##vendor, (device),	\
	PCI_ANY_ID, PCI_ANY_ID, 0, 0
#endif

/* For kernel version 2.6.20 upwards, pci_enable_device is reference counted. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,20)
#define KCOMPAT_PCI_ENABLE_DEVICE_IS_REF_COUNTED
#endif

/*
 * Define pci_get_device(), pci_get_subsys(), pci_dev_get() and pci_dev_put()
 * for kernels before 2.5.73.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,73)
static inline struct pci_dev *
pci_get_device(unsigned int vendor, unsigned int device, struct pci_dev *from)
{
	return pci_find_device(vendor, device, from);
}

static inline struct pci_dev *
pci_get_subsys(unsigned int vendor, unsigned int device,
	       unsigned int ss_vendor, unsigned int ss_device,
	       struct pci_dev *from)
{
	return pci_find_subsys(vendor, device, ss_vendor, ss_device, from);
}

static inline struct pci_dev *pci_dev_get(struct pci_dev *pcidev)
{
	return pcidev;
}

static inline void pci_dev_put(struct pci_dev *pcidev)
{
}
#endif

#endif	/* KCOMPAT_PCI_H__INCLUDED */
