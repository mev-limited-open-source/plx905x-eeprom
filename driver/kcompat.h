/*
 * MEV common driver compatibility code.
 *
 * Most of this code is taken from or based on source code from various
 * versions of the Linux kernel.
 *
 * Other parts written by MEV Ltd.
 */

/*
 * kcompat.h
 *
 * Kernel compatibility stuff for various drivers.
 *
 * Define KCOMPAT_DEFINE_NO_LLSEEK macro before including this file
 * to allow the file to define the function no_llseek with external linkage.
 * This is only done for kernel versions prior to 2.4.9.  The
 * KCOMPAT_DEFINE_NO_LLSEEK macro should only be defined in one .c file
 * of the driver.
 *
 * Define KCOMPAT_DEFINE_CLASS_DEVICE_CREATE macro in only one .c file of the
 * driver, but only if the driver calls class_device_create at all.  This is
 * required for compatibility with various 2.6.x kernel versions and is also
 * required to fake class_device support in earlier kernels.
 *
 * Define KCOMPAT_DEFINE_DEVICE_CREATE macro in only one .c file of the
 * driver, but only if the driver calls device_create at all.  If the driver
 * only uses device_create when KCOMPAT_NO_CLASS_DEVICE is set (indicating that
 * class_device_create cannot be used) then there is no need to define
 * KCOMPAT_DEFINE_DEVICE_CREATE.
 *
 * Define KCOMPAT_DEFINE_GET_JIFFIES_64 macro in only one .c file of the
 * driver, but only if the driver calls get_jiffies_64().  If kcompat.h
 * defines KCOMPAT_USING_OWN_GET_JIFFIES_64, the get_jiffies_64() results
 * will only be local to this driver, not global to the system and it is
 * necessary to ensure that get_jiffies_64() is called once to initialize
 * it and then at reasonable intervals (e.g. using a driver-global timer)
 * to ensure that jiffies does not wrap around more than once unnoticed.
 *
 * KCOMPAT_REQUEST_FIRMWARE_NOWAIT_HAS_GFP may be defined outside this
 * header file.  This needs to be set for some Red Hat 2.6.32 kernels
 * (2.6.32-131 onwards for RHEL 6.1 onwards).  If it hasn't been defined
 * already, this file will define it for kernel version 2.6.33 onwards.
 *
 * KCOMPAT_HAVE_COMPLETION_DONE and KCOMPAT_HAVE_TRY_WAIT_FOR_COMPLETION
 * may be defined outside this header file.  These need to be set for some
 * Red Hat 2.6.18 kernels (fro RHEL 5.x).  If they haven't beed defined
 * already, this file will define them for kernel version 2.6.27 onwards.
 */

#ifndef KCOMPAT_H__INCLUDED
#define KCOMPAT_H__INCLUDED

#include <linux/version.h>
#include <linux/types.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,0)
#error "*************************************************************"
#error " Sorry, this driver requires kernel version 2.4.0 or higher."
#error "*************************************************************"
#endif
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)) \
			   && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0))
#error "*************************************************************"
#error " Sorry, this driver does not support the 2.5 development kernel."
#error "*************************************************************"
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,4)
#include <linux/compiler.h>
#else
#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __builtin_expect(x, expected_value) (x)
#endif
#endif

#ifndef likely
#define likely(x)	__builtin_expect(!!(x),1)
#endif

#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x),0)
#endif

#ifndef __attribute_used__
#if __GNUC__ > 3
#define __attribute_used__	__attribute__((__used__))
#elif __GNUC__ == 3
#if __GNUC_MINOR >= 3
#define __attribute_used__	__attribute__((__used__))
#else
#define __attribute_used__	__attribute__((__unused__))
#endif
#elif __GNUC__ == 2
#define __attribute_used__	__attribute__((__unused__))
#else
#define __attribute_used__	/* not implemented */
#endif
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#if __GNUC__ >= 3
/* FIXME: Not sure which __GNUC_MINOR__ supports '_Bool'. */
typedef _Bool bool;
#else
/* FIXME: This might break, and doesn't have the C99 _Bool semantics. */
typedef int bool;
#endif	/* if __GNUC__ >= 3 */
#endif	/* if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19) */

#ifndef fallthrough
#define fallthrough	do {} while (0)  /* fallthrough */
#endif

#ifndef __user
#define __user
#endif

#ifndef __force
#define __force
#endif

#ifndef __iomem
#define __iomem
#endif

#ifndef __nocast
#define __nocast
#endif

/* Define little-endian and big-endian types for kernels prior to 2.6.9
 * when the '__bitwise' macro was defined. */
#ifndef __bitwise
#define __bitwise
typedef __u16 __bitwise __le16;
typedef __u16 __bitwise __be16;
typedef __u32 __bitwise __le32;
typedef __u32 __bitwise __be32;
typedef __u64 __bitwise __le64;
typedef __u64 __bitwise __be64;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
typedef unsigned __nocast gfp_t;
#endif

/* Define BITS_TO_LONGS(bits) macro if not defined by <linux/types.h> */
#ifndef BITS_TO_LONGS
#define BITS_TO_LONGS(bits) \
	(((bits) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#endif

/* Define DECLARE_BITMAP(name,bits) macro if not defined by <linux/types.h>. */
#ifndef DECLARE_BITMAP
#define DECLARE_BITMAP(name, bits) \
	unsigned long name[BITS_TO_LONGS(bits)]
#endif

#include <linux/init.h>

#ifndef __devinit
#define __devinit
#endif

#ifndef __devinitdata
#define __devinitdata
#endif

#ifndef __devinitconst
#define __devinitconst __devinitdata
#endif

#ifndef __devexit
#define __devexit
#endif

#ifndef __devexitdata
#define __devexitdata
#endif

#ifndef __devexitconst
#define __devexitconst __devexitdata
#endif

#ifndef __devexit_p
#define __devexit_p(x) x
#endif

/* Vanilla 2.6.19 kernel and Red Hat's 2.6.18 kernel do not have
 * <linux/config.h>.  There is no need to include it explicitly for 2.6.18
 * kernels anyway, as it is done with compiler command line options.  */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#define KCOMPAT_HAVE_LINUX_CONFIG_H
#include <linux/config.h>
#endif

#include <linux/kernel.h>

#ifndef KERN_CONT
#define KERN_CONT ""
#endif

/* If no __printf(a,b) macro, just define a dummy one instead of a GCC-specific
 * attribute checking one.  */
#ifndef __printf
#define __printf(a, b) /* nop */
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
static inline __printf(1, 2)
int kcompat_no_printk(const char *fmt, ...)
{
	return 0;
}
#undef no_printk
#define no_printk kcompat_no_printk
#endif

/* Driver code can define pr_fmt(fmt) itself to override this. */
#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

/*
 * Undefine the pr_ macros that use pr_fmt(fmt) for older kernel versions
 * so they will be redefined to use pr_fmt(fmt) below.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
#undef pr_emerg
#undef pr_alert
#undef pr_crit
#undef pr_err
#undef pr_warning
#undef pr_warn
#undef pr_notice
#undef pr_info
#undef pr_devel
#undef pr_debug
#undef pr_emerg_once
#undef pr_alert_once
#undef pr_crit_once
#undef pr_err_once
#undef pr_warn_once
#undef pr_info_once
#undef pr_devel_once
#undef pr_debug_once
#endif

/* Note: using old GCC variadic macro syntax for the following... */
#ifndef pr_emerg
#define pr_emerg(fmt, args...) \
	printk(KERN_EMERG pr_fmt(fmt), ##args)
#endif

#ifndef pr_alert
#define pr_alert(fmt, args...) \
	printk(KERN_ALERT pr_fmt(fmt), ##args)
#endif

#ifndef pr_crit
#define pr_crit(fmt, args...) \
	printk(KERN_CRIT pr_fmt(fmt), ##args)
#endif

#ifndef pr_err
#define pr_err(fmt, args...) \
	printk(KERN_ERR pr_fmt(fmt), ##args)
#endif

#ifndef pr_warning
#define pr_warning(fmt, args...) \
	printk(KERN_WARNING pr_fmt(fmt), ##args)
#endif

#ifndef pr_warn
#define pr_warn pr_warning
#endif

#ifndef pr_notice
#define pr_notice(fmt, args...) \
	printk(KERN_NOTICE pr_fmt(fmt), ##args)
#endif

#ifndef pr_info
#define pr_info(fmt, args...) \
	printk(KERN_INFO pr_fmt(fmt), ##args)
#endif

#ifndef pr_cont
/* This doesn't use pr_fmt(fmt) as it's a continuation of previous message. */
#define pr_cont(fmt, args...) \
	printk(KERN_CONT fmt, ##args)
#endif

#ifndef pr_devel
/* pr_devel() should produce zero code unless DEBUG is defined */
#ifdef DEBUG
#define pr_devel(fmt, args...) \
	printk(KERN_DEBUG pr_fmt(fmt), ##args)
#else
#define pr_devel(fmt, args...) \
	no_printk(KERN_DEBUG pr_fmt(fmt), ##args)
#endif
#endif

#ifndef pr_debug
#define pr_debug pr_devel
#endif

#ifndef printk_once
#define printk_once(fmt, args...)		\
({						\
	static int __print_once;		\
						\
	if (!__print_once) {			\
		__print_once = 1;		\
		printk(fmt, ##args);		\
	}					\
})
#endif

#ifndef pr_emerg_once
#define pr_emerg_once(fmt, args...) \
	printk_once(KERN_EMERG pr_fmt(fmt), ##args)
#endif

#ifndef pr_alert_once
#define pr_alert_once(fmt, args...) \
	printk_once(KERN_ALERT pr_fmt(fmt), ##args)
#endif

#ifndef pr_crit_once
#define pr_crit_once(fmt, args...) \
	printk_once(KERN_CRIT pr_fmt(fmt), ##args)
#endif

#ifndef pr_err_once
#define pr_err_once(fmt, args...) \
	printk_once(KERN_ERR pr_fmt(fmt), ##args)
#endif

#ifndef pr_warn_once
#define pr_warn_once(fmt, args...) \
	printk_once(KERN_WARNING pr_fmt(fmt), ##args)
#endif

#ifndef pr_notice_once
#define pr_notice_once(fmt, args...) \
	printk_once(KERN_NOTICE pr_fmt(fmt), ##args)
#endif

#ifndef pr_info_once
#define pr_info_once(fmt, args...) \
	printk_once(KERN_INFO pr_fmt(fmt), ##args)
#endif

#ifndef pr_cont_once
/* This doesn't use pr_fmt(fmt) as it's a continuation of previous message. */
#define pr_cont_once(fmt, args...) \
	printk_once(KERN_CONT fmt, ##args)
#endif

#ifndef pr_devel_once
/* pr_devel_once() should produce zero code unless DEBUG is defined */
#ifdef DEBUG
#define pr_devel_once(fmt, args...) \
	printk_once(KERN_DEBUG pr_fmt(fmt), ##args)
#else
#define pr_devel_once(fmt, args...) \
	no_printk(KERN_DEBUG pr_fmt(fmt), ##args)
#endif
#endif

#ifndef pr_debug_once
#define pr_debug_once pr_devel_once
#endif

/* TODO: support rate-limited printk stuff. */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,20)
#define dump_stack() do {} while(0)
#endif

#ifndef BUG_ON
#define BUG_ON(condition) do { if (unlikely((condition)!=0)) BUG(); } while(0)
#endif

#ifndef WARN_ON
#define WARN_ON(condition) do { \
	if (unlikely((condition)!=0)) { \
		printk("Badness in %s at %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
		dump_stack(); \
	} \
} while(0)
#endif

/* Define __ALIGN_MASK(x,mask) macro if not defined by <linux/kernel.h>. */
#ifndef __ALIGN_MASK
#define __ALIGN_MASK(x, mask)	(((x) + (mask)) & ~(mask))
/* Redefine ALIGN(x,a) to use __ALIGN_MASK(x,mask) and typeof(x).  This change
 * was made in the kernel at the same time __ALIGN_MASK was added. */
/* Warning: this uses GCC extensions! */
#undef ALIGN
#define ALIGN(x, a)	__ALIGN_MASK(x, (typeof(x))(a) - 1)
#endif	/* ifndef __ALIGN_MASK */

/* Define PTR_ALIGN(p,a) if not defined by <linux/kernel.h>. */
#ifndef PTR_ALIGN
#define PTR_ALIGN(p, a)	((typeof(p))ALIGN((unsigned long)(p), (a)))
#endif

/* Defined IS_ALIGNED(x,a) if not defined by <linux/kernel.h>. */
#ifndef IS_ALIGNED
#define IS_ALIGNED(x, a)	(((x) & ((typeof(x))(a) - 1)) == 0)
#endif

/* Define DIV_ROUND_UP(n,d) if not defined by <linux/kernel.h>. */
#ifndef DIV_ROUND_UP
#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#endif

/* (Re)define roundup(x,y) and rounddown(x,y) to evaluate their arguments
 * once for kernel versions prior to 2.6.37. */
/* Warning: this uses GCC extensions! */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
#undef roundup
#define roundup(x, y) (				\
{						\
	const typeof(y) __y = (y);		\
	(((x) + (__y - 1)) / __y) * __y;	\
}						\
)

#undef rounddown
#define rounddown(x, y) (			\
{						\
	typeof(x) __x = (x);			\
	__x - (__x % (y));			\
}						\
)

#endif

/* Define FIELD_SIZEOF(t, f) if not defined by <linux/kernel.h>. */
#ifndef FIELD_SIZEOF
#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))
#endif

/* Define __round_mask(x,y) if not defined by <linux/kernel.h> and redefine
 * round_up(x,y) and round_down(x,y) to use it. */
/* Warning: this uses GCC extensions! */
#ifndef __round_mask
#define __round_mask(x, y)	((__typeof__(x))((y) - 1))
#undef round_up
#define round_up(x, y)	((((x) - 1) | __round_mask(x, y)) + 1)
#undef round_down
#define round_down(x, y)	((x) & ~__round_mask(x, y))
#endif

/*
 * min()/max() were first defined in <linux/kernel.h> around 2.4.9,
 * but they were really min_t() and max_t().  They weren't used much by
 * other kernel headers except <net/tcp.h> which we can probably assume
 * won't be included.  (If <net/tcp.h> is included, the build won't work
 * for kernel version 2.4.9 unless <net/tcp.h> is included before this file.)
 */
#ifndef min_t
#undef min
#define min(x, y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void)(&_x == &_y);		\
	_x < _y ? _x : _y; })

#define min_t(type, x, y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x : __y; })
#endif

#ifndef max_t
#undef max
#define max(x, y) ({ \
	const typeof(x) _x = (x);	\
	const typeof(y) _y = (y);	\
	(void)(&_x == &_y);		\
	_x > _y ? _x : _y; })

#define max_t(type, x, y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x : __y; })
#endif

/*
 * BIT(nr) etc. were first defined in kernel 2.6.24.
 */
#ifndef BIT
#define BIT(nr)			(1UL << (nr))
#endif
#ifndef BIT_MASK
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#endif
#ifndef BIT_WORD
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#endif
#ifndef BITS_TO_LONGS
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP((nr), BITS_PER_LONG)
#endif
#ifndef BITS_PER_BYTE
#define BITS_PER_BYTE		8
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,25)
#define MODULE_PARM_short(x)	MODULE_PARM(x, "h")
#define MODULE_PARM_ushort(x)	MODULE_PARM(x, "h")
#define MODULE_PARM_int(x)	MODULE_PARM(x, "i")
#define MODULE_PARM_uint(x)	MODULE_PARM(x, "i")
#define MODULE_PARM_long(x)	MODULE_PARM(x, "l")
#define MODULE_PARM_ulong(x)	MODULE_PARM(x, "l")
#define MODULE_PARM_bool(x)	MODULE_PARM(x, "i")
#define module_param(x,y,z)	MODULE_PARM_##y(x)
#else
#include <linux/moduleparam.h>
#endif

/*
 * Before kernel 2.6.31, bool module parameters were backed by
 * 'int' or 'unsigned int' variables.
 *
 * From kernel 2.6.31 to kernel 3.2, bool module parameters can also be
 * backed by 'bool' variables.
 *
 * For kernel 3.3, a bool module parameter should be backed
 * by a 'bool' variable and using 'int' or 'unsigned int' produces a
 * warning but is still supported.  There is also a 'bint' module parameter
 * type for compatibility with some drivers.
 *
 * From kernel 3.4 onwards, a bool module parameter must be backed by a
 * 'bool' variable.
 *
 * Define KCOMPAT_MODULE_PARAM_BOOL_IS_INT if an 'int' or 'unsigned int'
 * variable needs to be used.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,31)
#define KCOMPAT_MODULE_PARAM_BOOL_IS_INT
#endif

/* Define S_IRUGO etc. */
#include <linux/stat.h>

#include <linux/module.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#define KCOMPAT_HAVE_LINUX_DEVICE_H
#include <linux/device.h>
#endif

#ifndef MODULE_LICENSE
#define MODULE_LICENSE(x)
#endif

#ifndef MODULE_VERSION
#define MODULE_VERSION(x)
#endif

#ifndef MODULE_ALIAS_CHARDEV_MAJOR
#define MODULE_ALIAS_CHARDEV_MAJOR(x)
#endif

#ifndef MODULE_FIRMWARE
#define MODULE_FIRMWARE(x)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,48)
static inline int try_module_get(struct module *module)
{
	if (!module)
		return 1;
	__MOD_INC_USE_COUNT(module);
	return 1;
}
static inline void module_put(struct module *module)
{
	if (!module)
		return;
	__MOD_DEC_USE_COUNT(module);
}
#endif

#ifndef container_of
/* This uses GCC extensions! */
#define container_of(ptr, type, member) ({ \
		const typeof(((type *)0)->member) *__mptr = (ptr); \
		(type *)((char *)__mptr - offsetof(type, member)); })
#endif

#include <linux/slab.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,14)
/* Some RHEL4 2.6.9 kernels have kzalloc.  Redefine to avoid warnings about
 * static declaration following non-static declaration. */
#undef kzalloc
#define kzalloc kcompat_kzalloc
static inline void *kcompat_kzalloc(size_t size, unsigned int flags)
{
	void *ret = kmalloc(size, flags);
	if (ret) {
		memset(ret, 0, size);
	}
	return ret;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
static inline void *kcalloc(size_t n, size_t size, int flags)
{
	if (n != 0 && size > INT_MAX / n)
		return NULL;
	return kzalloc(n * size, flags);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
/* Some RHEL 2.6.32 kernels have kmalloc_array.  Redefine to avoid warnings
 * about static declaration following non-static declaration. */
#undef kmalloc_array
#define kmalloc_array kcompat_kmalloc_array
static inline void *kcompat_kmalloc_array(size_t n, size_t size, gfp_t flags)
{
	if (n != 0 && size > ULONG_MAX / n)
		return NULL;
	return kmalloc(n * size, flags);
}
#endif

/* The 'struct kref' interface was introduced in 2.6.5 and has changed a 
 * few times since. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5)
#include <asm/atomic.h>

struct kref {
	atomic_t refcount;
};

static inline void kref_init(struct kref *kref)
{
	atomic_set(&kref->refcount, 1);
}

static inline void kref_get(struct kref *kref)
{
	atomic_inc(&kref->refcount);
}

static inline int kref_put(struct kref *kref, void (*release)(struct kref *))
{
	if (atomic_dec_and_test(&kref->refcount)) {
		release(kref);
		return 1;
	}
	return 0;
}

#else
#define KCOMPAT_HAVE_LINUX_KREF_H

/* Prior to 2.6.9, kref_init() had a second parameter pointing to a
 * 'release' function.  Redefine it without the second parameter, setting
 * 'release' to a dummy non-null value. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
static inline void kcompat_kref_init(struct kref *kref)
{
	kref_init(kref, (void (*)(struct kref *))kfree);
}

#undef kref_init
#define kref_init(kref)	kcompat_kref_init(kref)
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9) */

/* Prior to 2.6.9, kref_put() never had a second parameter pointing to a
 * 'release' function.  Prior to 2.6.12, it never returned an integer value.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12)
static inline int kcompat_kref_put(struct kref *kref,
		void (*release)(struct kref *))
{
	if (atomic_dec_and_test(&kref->refcount)) {
		release(kref);
		return 1;
	}
	return 0;
}

#undef kref_put
#define kref_put(kref, release) kcompat_kref_put(kref, release)
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,12) */

#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,5) */

/* <linux/rbtree.h> stuff. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,10)
#define KCOMPAT_HAVE_LINUX_RBTREE_H
#endif

#ifndef KCOMPAT_HAVE_LINUX_RBTREE_H
/*
 * TODO: emulate the red-black tree stuff for kernel versions prior to 2.4.10.
 * That's a job for when we've got too much spare time on our hands!
 */
#else
/* Since we have <linux/rbtree.h> we can at least implement some of the
 * simpler backwards compatibility stuff. */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,36)
/*
 * Mirror the existing 'struct rb_node_s' and 'struct rb_root_s' as
 * 'struct rb_node' and 'struct 'rb_root'.  Then #define rb_node_s as rb_node
 * and rb_root_s as rb_root.  This may cause some warnings about incompatible
 * pointer types, but the code should still work.  It's the best we can do
 * unless we add the rb_node_t and rb_root_t typedefs and require our driver
 * code to use them instead of the struct rb_node and struct rb_root.
 */
struct rb_node {
	struct rb_node *rb_parent;
	int rb_color;
	struct rb_node *rb_right;
	struct rb_node *rb_left;
};

struct rb_root {
	struct rb_node *rb_node;
};

#define rb_node_s rb_node
#define rb_root_s rb_root

#endif	/* if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,36) */

#include <linux/rbtree.h>

/* .rb_parent and .rb_color were combined into .rb_parent_color in 2.6.18 */
#ifndef rb_parent
#define rb_parent(r)	((r)->rb_parent)
#define rb_color(r)	((r)->rb_color)
#define rb_is_red(r)	(!rb_color(r))
#define rb_is_black(r)	rb_color(r)
#define rb_set_red(r)	do { (r)->rb_color = RB_RED; } while (0)
#define rb_set_black(r)	do { (r)->rb_color = RB_BLACK; } while (0)

static inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
	rb->rb_parent = p;
}

static inline void rb_set_color(struct rb_node *rb, int color)
{
	rb->rb_color = color;
}
#endif	/* ifndef rb_parent */

#ifndef RB_EMPTY_ROOT
#define RB_EMPTY_ROOT(root)	((root)->rb_node == NULL)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
/* undefine buggy RB_EMPTY_NODE */
#undef RB_EMPTY_NODE
#endif

#ifndef RB_EMPTY_NODE
#define RB_EMPTY_NODE(node)	(rb_parent(node) == node)
#endif

#ifndef RB_CLEAR_NODE
#define RB_CLEAR_NODE(node)	(rb_set_parent(node, node))
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
static inline void rb_init_node(struct rb_node *rb)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
	rb_set_color(rb, 0);
#else
	rb->rb_parent_color = 0;
#endif
	rb->rb_right = NULL;
	rb->rb_left = NULL;
	RB_CLEAR_NODE(rb);
}
#endif

#endif	/* ifndef KCOMPAT_HAVE_LINUX_RBTREE_H */

/* <linux/list.h> stuff. */
#include <linux/list.h>

#ifndef LIST_POISON1
#define LIST_POISON1	((void *)0x00100100)
#define LIST_POISON2	((void *)0x00200200)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
/*
 * Kernel 2.6.0 changed the parameter of list_empty() to be a pointer to const.
 */
static inline int kcompat_list_empty(const struct list_head *head)
{
	return head->next == head;
}

#undef list_empty
#define list_empty(head) kcompat_list_empty(head)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,38)
static inline void __list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
static inline void list_replace(struct list_head *old,
				struct list_head *new)
{
	new->next = old->next;
	new->next->prev = new;
	new->prev = old->prev;
	new->prev->next = new;
}

static inline void list_replace_init(struct list_head *old,
					struct list_head *new)
{
	list_replace(old, new);
	INIT_LIST_HEAD(old);
}

static inline int list_is_last(const struct list_head *list,
				const struct list_head *head)
{
	return list->next == head;
}
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,20)) || \
	((LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)) && \
	 (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,22)))
static inline void list_move(struct list_head *list, struct list_head *head)
{
	__list_del_entry(list);
	list_add(list, head);
}

static inline void list_move_tail(struct list_head *list,
				  struct list_head *head)
{
	__list_del_entry(list);
	list_add_tail(list, head);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
static inline int list_empty_careful(const struct list_head *head)
{
	struct list_head *next = head->next;
	return (next == head) && (next == head->prev);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34)
static inline void list_rotate_left(struct list_head *head)
{
	struct list_head *first;

	if (!list_empty(head)) {
		first = head->next;
		list_move_tail(first, head);
	}
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
static inline int list_is_singular(const struct list_head *head)
{
	return !list_empty(head) && (head->next == head->prev);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
static inline void __list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	struct list_head *new_first = entry->next;
	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

static inline void list_cut_position(struct list_head *list,
		struct list_head *head, struct list_head *entry)
{
	if (list_empty(head))
		return;
	if (list_is_singular(head) &&
		(head->next != entry && head != entry))
		return;
	if (entry == head)
		INIT_LIST_HEAD(list);
	else
		__list_cut_position(list, head, entry);
}

static inline void __kcompat_list_splice(const struct list_head *list,
					 struct list_head *prev,
					 struct list_head *next)
{
	struct list_head *first = list->next;
	struct list_head *last = list->prev;

	first->prev = prev;
	prev->next = first;

	last->next = next;
	next->prev = last;
}

#undef __list_splice
#define __list_splice(list, prev, next)	__kcompat_list_splice(list, prev, next)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,4,20)) || \
	((LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)) && \
	 (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,25)))
static inline void list_splice_init(struct list_head *list,
				    struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head, head->next);
		INIT_LIST_HEAD(list);
	}
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,20)
static inline void list_splice(const struct list_head *list,
				struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head, head->next);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
static inline void list_splice_tail(struct list_head *list,
				struct list_head *head)
{
	if (!list_empty(list))
		__list_splice(list, head->prev, head);
}

static inline void list_splice_tail_init(struct list_head *list,
					 struct list_head *head)
{
	if (!list_empty(list)) {
		__list_splice(list, head->prev, head);
		INIT_LIST_HEAD(list);
	}
}
#endif

#ifndef list_first_entry
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)
#endif

#ifndef __list_for_each
#define __list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)
#endif

#ifndef list_for_each_prev
#define list_for_each_prev(pos, head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)
#endif

#ifndef list_for_each_safe
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)
#endif

#ifndef list_for_each_prev_safe
#define list_for_each_prev_safe(pos, n, head) \
	for (pos = (head)->prev, n = pos->prev; \
	     pos != (head); \
	     pos = n, n = pos->prev)
#endif

#ifndef list_for_each_entry
#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))
#endif

#ifndef list_for_each_entry_reverse
#define list_for_each_entry_reverse(pos, head, member)			\
	for (pos = list_entry((head)->prev, typeof(*pos), member);	\
	     &pos->member != (head);	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))
#endif

#ifndef list_prepare_entry
#define list_prepare_entry(pos, head, member) \
	((pos) ? : list_entry(head, typeof(*pos), member))
#endif

#ifndef list_for_each_entry_continue
#define list_for_each_entry_continue(pos, head, member)			\
	for (pos = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))
#endif

#ifndef list_for_each_entry_continue_reverse
#define list_for_each_entry_continue_reverse(pos, head, member)		\
	for (pos = list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head);	\
	     pos = list_entry(pos->member.prev, typeof(*pos), member))
#endif

#ifndef list_for_each_entry_from
#define list_for_each_entry_from(pos, head, member)			\
	for (; &pos->member != (head);	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))
#endif

#ifndef list_for_each_entry_safe
#define list_for_each_entry_safe(pos, n, head, member)			\
	for (pos = list_entry((head)->next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))
#endif

#ifndef list_for_each_entry_safe_continue
#define list_for_each_entry_safe_continue(pos, n, head, member)		\
	for (pos = list_entry(pos->member.next, typeof(*pos), member),	\
		n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))
#endif

#ifndef list_for_each_entry_safe_from
#define list_for_each_entry_safe_from(pos, n, head, member)		\
	for (n = list_entry(pos->member.next, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.next, typeof(*n), member))
#endif

#ifndef list_for_each_entry_safe_reverse
#define list_for_each_entry_safe_reverse(pos, n, head, member)		\
	for (pos = list_entry((head)->prev, typeof(*pos), member),	\
		n = list_entry(pos->member.prev, typeof(*pos), member);	\
	     &pos->member != (head);					\
	     pos = n, n = list_entry(n->member.prev, typeof(*n), member))
#endif

#ifndef list_safe_reset_next
#define list_safe_reset_next(pos, n, member)				\
	n = list_entry(pos->member.next, typeof(*pos), member)
#endif

/* hlist stuff was started in kernel 2.5.64 */
#ifndef HLIST_HEAD_INIT

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define HLIST_HEAD_INIT { .first = NULL }
#define HLIST_HEAD(name) struct hlist_head name = {  .first = NULL }
#define INIT_HLIST_HEAD(ptr) ((ptr)->first = NULL)
static inline void INIT_HLIST_NODE(struct hlist_node *h)
{
	h->next = NULL;
	h->pprev = NULL;
}

static inline int hlist_unhashed(const struct hlist_node *h)
{
	return !h->pprev;
}

static inline int hlist_empty(const struct hlist_head *h)
{
	return !h->first;
}

static inline void __hlist_del(struct hlist_node *n)
{
	struct hlist_node *next = n->next;
	struct hlist_node **pprev = n->pprev;
	*pprev = next;
	if (next)
		next->pprev = pprev;
}

static inline void hlist_del(struct hlist_node *n)
{
	__hlist_del(n);
	n->next = LIST_POISON1;
	n->pprev = LIST_POISON2;
}

static inline void hlist_del_init(struct hlist_node *n)
{
	if (!hlist_unhashed(n)) {
		__hlist_del(n);
		INIT_HLIST_NODE(n);
	}
}

static inline void hlist_add_head(struct hlist_node *n, struct hlist_head *h)
{
	struct hlist_node *first = h->first;
	n->next = first;
	if (first)
		first->pprev = &n->next;
	h->first = n;
	n->pprev = &h->first;
}

/* next must be != NULL */
static inline void hlist_add_before(struct hlist_node *n,
					struct hlist_node *next)
{
	n->pprev = next->pprev;
	n->next = next;
	next->pprev = &n->next;
	*(n->pprev) = n;
}

#define hlist_entry(ptr, type, member) container_of(ptr,type,member)

#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)

#endif	/* ifndef HLIST_HEAD_INIT */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,71)
static inline void hlist_add_after(struct hlist_node *n,
					struct hlist_node *next)
{
	next->next = n->next;
	n->next = next;
	next->pprev = &n->next;

	if(next->next)
		next->next->pprev  = &next->next;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
static inline void hlist_add_fake(struct hlist_node *n)
{
	n->pprev = &n->next;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
static inline void hlist_move_list(struct hlist_head *old,
				   struct hlist_head *new)
{
	new->first = old->first;
	if (new->first)
		new->first->pprev = &new->first;
	old->first = NULL;
}
#endif

#ifndef hlist_for_each_safe
#define hlist_for_each_safe(pos, n, head) \
	for (pos = (head)->first; pos && ({ n = pos->next; 1; }); \
	     pos = n)
#endif

#ifndef hlist_for_each_entry
#define hlist_for_each_entry(tpos, pos, head, member)			 \
	for (pos = (head)->first;					 \
	     pos &&							 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)
#endif

#ifndef hlist_for_each_entry_continue
#define hlist_for_each_entry_continue(tpos, pos, member)		 \
	for (pos = (pos)->next;						 \
	     pos &&							 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)
#endif

#ifndef hlist_for_each_entry_from
#define hlist_for_each_entry_from(tpos, pos, member)			 \
	for (; pos &&							 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = pos->next)
#endif

#ifndef hlist_for_each_entry_safe
#define hlist_for_each_entry_safe(tpos, pos, n, head, member)		 \
	for (pos = (head)->first;					 \
	     pos && ({ n = pos->next; 1; }) &&				 \
		({ tpos = hlist_entry(pos, typeof(*tpos), member); 1;}); \
	     pos = n)
#endif


#ifndef BUS_ID_SIZE
#define BUS_ID_SIZE	20
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
#include <linux/devfs_fs_kernel.h>
#endif

#ifndef DEVFS_FL_DEFAULT
#define DEVFS_FL_DEFAULT	0
#endif

#ifdef DEVFS_SPECIAL_CHR
#define DEVFS_SPECIAL_CHR	0
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,30)
#ifdef register_chrdev
#undef register_chrdev
#endif
#define register_chrdev		devfs_register_chrdev
#ifdef unregister_chrdev
#undef unregister_chrdev
#endif
#define unregister_chrdev	devfs_unregister_chrdev
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,70)) \
			   && (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18))
#define KCOMPAT_HAVE_DEVFS_26
typedef struct devfs_entry *devfs_handle_t;
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#define KCOMPAT_HAVE_DEVFS_24
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
/* Fake device class interface.  Make it similar to 2.6.15. */

struct device;

struct class_device {
	struct list_head node;
	struct kref kref;
	struct class *class;
	dev_t devt;
	void *class_data;
	void (*release)(struct class_device *cd);
	char class_id[BUS_ID_SIZE];
};

struct class {
	struct list_head children;
	struct semaphore sem;
	struct kref kref;
	void (*release)(struct class_device *cd);
	void (*class_release)(struct class *cls);
};

#undef class_get
#define class_get(cls) kcompat_class_get(cls)
extern struct class *kcompat_class_get(struct class *cls);

#undef class_put
#define class_put(cls) kcompat_class_put(cls)
extern void kcompat_class_put(struct class *cls);

#undef class_register
#define class_register(cls) kcompat_class_register(cls)
extern int kcompat_class_register(struct class *cls);

#undef class_unregister
#define class_unregister(cls) kcompat_class_unregister(cls)
extern void kcompat_class_unregister(struct class *cls);

/* Note: class_create() macro is (re)defined below. */
extern struct class *kcompat_class_create(struct module *owner,
		const char *name);

#undef class_destroy
#define class_destroy(cls) kcompat_class_destroy(cls)
extern void kcompat_class_destroy(struct class *cls);

#undef class_device_initialize
#define class_device_initialize(cd) kcompat_class_device_initialize(cd)
extern void kcompat_class_device_initialize(struct class_device *cd);

#undef class_device_get
#define class_device_get(cd) kcompat_class_device_get(cd)
extern struct class_device *kcompat_class_device_get(struct class_device *cd);

#undef class_device_put
#define class_device_put(cd) kcompat_class_device_put(cd)
extern void kcompat_class_device_put(struct class_device *cd);

#undef class_device_add
#define class_device_add(cd) kcompat_class_device_add(cd)
extern int kcompat_class_device_add(struct class_device *cd);

#undef class_device_del
#define class_device_del(cd) kcompat_class_device_del(cd)
extern void kcompat_class_device_del(struct class_device *cd);

#undef class_device_register
#define class_device_register(cd) kcompat_class_device_register(cd)
extern int kcompat_class_device_register(struct class_device *cd);

#undef class_device_unregister
#define class_device_unregister(cd) kcompat_class_device_unregister(cd)
extern void kcompat_class_device_unregister(struct class_device *cd);

#undef class_device_create
#define class_device_create(cls, parent, devt, device, fmt...) \
	kcompat_class_device_create(cls, parent, devt, device, fmt)
extern struct class_device *kcompat_class_device_create(struct class *cls,
		struct class_device *parent, dev_t devt, struct device *device,
		const char *fmt, ...);

#undef class_device_destroy
#define class_device_destroy(cls, devt) kcompat_class_device_destroy(cls, devt)
extern void kcompat_class_device_destroy(struct class *cls, dev_t devt);

static inline void class_set_devdata(struct class_device *cd, void *data)
{
	cd->class_data = data;
}

static inline void *class_get_devdata(struct class_device *cd)
{
	return cd->class_data;
}

#define KCOMPAT_GET_CLASS_DEVICE_DEVT(cd) ((cd)->devt)

#ifdef KCOMPAT_DEFINE_CLASS_DEVICE_CREATE

struct class *kcompat_class_get(struct class *cls)
{
	if (cls)
		kref_get(&cls->kref);

	return cls;
}

static void kcompat_class_kref_release(struct kref *kref)
{
	struct class *cls = container_of(kref, struct class, kref);

	if (cls->class_release)
		cls->class_release(cls);
}

void kcompat_class_put(struct class *cls)
{
	if (cls)
		kref_put(&cls->kref, kcompat_class_kref_release);
}

int kcompat_class_register(struct class *cls)
{
	INIT_LIST_HEAD(&cls->children);
	init_MUTEX(&cls->sem);
	kref_init(&cls->kref);
	return 0;
}

void kcompat_class_unregister(struct class *cls)
{
	kcompat_class_put(cls);
}

static void kcompat_class_create_release(struct class *cls)
{
	kfree(cls);
}

static void kcompat_class_device_create_release(struct class_device *cd)
{
	kfree(cd);
}

struct class *kcompat_class_create(struct module *owner, const char *name)
{
	struct class *cls;
	int retval;

	cls = kzalloc(sizeof(*cls), GFP_KERNEL);
	if (!cls) {
		retval = -ENOMEM;
		goto error;
	}

	cls->class_release = kcompat_class_create_release;
	cls->release = kcompat_class_device_create_release;

	retval = kcompat_class_register(cls);
	if (retval)
		goto error;

	return cls;

error:
	kfree(cls);
	return ERR_PTR(retval);
}

void kcompat_class_destroy(struct class *cls)
{
	if (cls == NULL || IS_ERR(cls))
		return;

	kcompat_class_unregister(cls);
}

void kcompat_class_device_initialize(struct class_device *cd)
{
	kref_init(&cd->kref);
	INIT_LIST_HEAD(&cd->node);
}

struct class_device *kcompat_class_device_get(struct class_device *cd)
{
	if (cd)
		kref_get(&cd->kref);

	return cd;
}

static void kcompat_class_device_kref_release(struct kref *kref)
{
	struct class_device *cd = container_of(kref, struct class_device, kref);
	struct class *cls = cd->class;

	if (cd->release)
		cd->release(cd);
	else if (cls->release)
		cls->release(cd);
}

void kcompat_class_device_put(struct class_device *cd)
{
	if (cd)
		kref_put(&cd->kref, kcompat_class_device_kref_release);
}

int kcompat_class_device_add(struct class_device *cd)
{
	struct class *parent_class = NULL;
	int error = -EINVAL;

	cd = kcompat_class_device_get(cd);
	if (!cd)
		return -EINVAL;

	if (!strlen(cd->class_id))
		goto register_done;

	parent_class = kcompat_class_get(cd->class);
	if (!parent_class)
		goto register_done;

	error = 0;

	down(&parent_class->sem);
	list_add_tail(&cd->node, &parent_class->children);
	up(&parent_class->sem);

register_done:
	if (error)
		kcompat_class_put(parent_class);

	kcompat_class_device_put(cd);
	return error;
}

void kcompat_class_device_del(struct class_device *cd)
{
	struct class *parent_class = cd->class;

	if (parent_class) {
		down(&parent_class->sem);
		list_del_init(&cd->node);
		up(&parent_class->sem);
	}

	kcompat_class_put(parent_class);
}

int kcompat_class_device_register(struct class_device *cd)
{
	kcompat_class_device_initialize(cd);
	return kcompat_class_device_add(cd);
}

void kcompat_class_device_unregister(struct class_device *cd)
{
	kcompat_class_device_del(cd);
	kcompat_class_device_put(cd);
}

struct class_device *kcompat_class_device_create(struct class *cls,
		struct class_device *parent, dev_t devt, struct device *device,
		const char *fmt, ...)
{
	va_list args;
	struct class_device *cd = NULL;
	int retval = -ENODEV;

	if (cls == NULL || IS_ERR(cls))
		goto error;

	cd = kzalloc(sizeof(*cd), GFP_KERNEL);
	if (!cd) {
		retval = -ENOMEM;
		goto error;
	}

	cd->devt = devt;
	cd->class = cls;
	cd->release = kcompat_class_device_create_release;

	va_start(args, fmt);
	vsnprintf(cd->class_id, BUS_ID_SIZE, fmt, args);
	va_end(args);
	retval = kcompat_class_device_register(cd);
	if (retval)
		goto error;

	return cd;

error:
	kfree(cd);
	return ERR_PTR(retval);
}

void kcompat_class_device_destroy(struct class *cls, dev_t devt)
{
	struct class_device *cd = NULL;
	struct class_device *cd_tmp;

	down(&cls->sem);
	list_for_each_entry(cd_tmp, &cls->children, node) {
		if (cd_tmp->devt == devt) {
			cd = cd_tmp;
			break;
		}
	}
	up(&cls->sem);

	if (cd)
		kcompat_class_device_unregister(cd);
}

#endif	/* KCOMPAT_DEFINE_CLASS_DEVICE_CREATE */

/*
 * Define kcompat_class_create(owner, name) as a macro that calls the function
 * of the same name.  This will be tested for later.
 */
#define kcompat_class_create(owner, name) kcompat_class_create(owner, name)

#else	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) */
#include <linux/device.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
#define KCOMPAT_HAVE_STRUCT_CLASS
#define KCOMPAT_HAVE_STRUCT_CLASS_DEVICE
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,12)
/* Test for devt in struct class_device. */
#define KCOMPAT_HAVE_CLASS_DEVICE_DEVT
#else
/* This matches struct simple_dev in drivers/base/class-simple.c */
struct kcompat_simple_dev {
	struct list_head node;
	dev_t dev;
	struct class_device class_dev;
};
#define kcompat_to_simple_dev(d) \
	container_of(d, struct kcompat_simple_dev, class_dev)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)

/* This matches struct class_simple in drivers/base/class-simple.c */
struct kcompat_class_simple {
#ifndef KCOMPAT_HAVE_CLASS_DEVICE_DEVT
	struct class_device_attribute attr;
#endif
	struct class class;
};
#define kcompat_to_class_simple(d) \
	container_of(d, struct kcompat_class_simple, class)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,2)
/* struct class_simple and struct class_device_simple do not exist. */
struct class_simple;
/*
 * Define the functions for class_simple and class_device_simple if required.
 * All this is mostly copied from drivers/base/class-simple.c, but renamed.
 */
extern struct kcompat_class_simple *kcompat_class_simple_create(
		struct module *, char *);
extern void kcompat_class_simple_destroy(struct kcompat_class_simple *);
extern struct class_device *kcompat_class_simple_device_add(
		struct kcompat_class_simple *, dev_t, struct device *,
		const char *, ...);
extern void kcompat_class_simple_device_remove(dev_t);

#undef class_simple_create
#define class_simple_create(owner, name) \
	(struct class_simple *)kcompat_class_simple_create(owner, name)
#undef class_simple_destroy
#define class_simple_destroy(cs) \
	kcompat_class_simple_destroy((struct kcompat_class_simple *)cs)
#undef class_simple_device_add
/* Note: this uses a GNU GCC extension. */
#define class_simple_device_add(cs, dev, device, fmt...) \
	kcompat_class_simple_device_add((struct kcompat_class_simple *)cs, \
			dev, device, fmt)
#undef class_simple_device_remove
#define class_simple_device_remove(dev) kcompat_class_simple_device_remove(dev)

#ifdef KCOMPAT_DEFINE_CLASS_DEVICE_CREATE
static LIST_HEAD(kcompat_simple_dev_list);
static spinlock_t kcompat_simple_dev_list_lock = SPIN_LOCK_UNLOCKED;

static void kcompat_release_simple_dev(struct class_device *class_dev)
{
	struct kcompat_simple_dev *s_dev = kcompat_to_simple_dev(class_dev);
	kfree(s_dev);
}

static ssize_t kcompat_show_dev(struct class_device *class_dev, char *buf)
{
	struct kcompat_simple_dev *s_dev = kcompat_to_simple_dev(class_dev);
	return print_dev_t(buf, s_dev->dev);
}

static void kcompat_class_simple_release(struct class *class)
{
	struct kcompat_class_simple *cs = kcompat_to_class_simple(class);
	kfree(cs);
}

struct kcompat_class_simple *kcompat_class_simple_create(struct module *owner,
		char *name)
{
	struct kcompat_class_simple *cs;
	int retval;

	cs = kmalloc(sizeof(*cs), GFP_KERNEL);
	if (!cs) {
		retval = -ENOMEM;
		goto error;
	}
	memset(cs, 0, sizeof(*cs));
	cs->class.name = name;
	/* Note: cs->class.class_release does not exist in this kernel. */
	/* cs->class.class_release = kcompat_class_simple_release; */
	cs->class.release = kcompat_release_simple_dev;
	cs->attr.attr.name = "dev";
	cs->attr.attr.mode = S_IRUGO;
	cs->attr.attr.owner = owner;
	cs->attr.show = kcompat_show_dev;
	cs->attr.store = NULL;
	retval = class_register(&cs->class);
	if (retval) {
		goto error;
	}
	return cs;
error:
	kfree(cs);
	return ERR_PTR(retval);
}

void kcompat_class_simple_destroy(struct kcompat_class_simple *cs)
{
	if ((cs == NULL) || (IS_ERR(cs))) {
		return;
	}
	class_unregister(&cs->class);
	/* Note: because cs->class.class_release does not exist, it has to
	 * be freed here.  Possibly a little unsafe.  */
	kfree(cs);
}

struct class_device *kcompat_class_simple_device_add(
		struct kcompat_class_simple *cs, dev_t dev,
		struct device *device, const char *fmt, ...)
{
	va_list args;
	struct kcompat_simple_dev *s_dev = NULL;
	int retval;

	if ((cs == NULL) || (IS_ERR(cs))) {
		retval = -ENODEV;
		goto error;
	}
	s_dev = kmalloc(sizeof(*s_dev), GFP_KERNEL);
	if (!s_dev) {
		retval = -ENOMEM;
		goto error;
	}
	memset(s_dev, 0, sizeof(*s_dev));
	s_dev->dev = dev;
	s_dev->class_dev.dev = device;
	s_dev->class_dev.class = &cs->class;
	va_start(args, fmt);
	vsnprintf(s_dev->class_dev.class_id, BUS_ID_SIZE, fmt, args);
	va_end(args);
	retval = class_device_register(&s_dev->class_dev);
	if (retval) {
		goto error;
	}
	class_device_create_file(&s_dev->class_dev, &cs->attr);
	spin_lock(&kcompat_simple_dev_list_lock);
	list_add(&s_dev->node, &kcompat_simple_dev_list);
	spin_unlock(&kcompat_simple_dev_list_lock);
	return &s_dev->class_dev;
error:
	kfree(s_dev);
	return ERR_PTR(retval);
}

void kcompat_class_simple_device_remove(dev_t dev)
{
	struct kcompat_simple_dev *s_dev = NULL;
	struct list_head *tmp;
	int found = 0;

	spin_lock(&kcompat_simple_dev_list_lock);
	list_for_each(tmp, &kcompat_simple_dev_list) {
		s_dev = list_entry(tmp, struct kcompat_simple_dev, node);
		if (s_dev->dev == dev) {
			found = 1;
			break;
		}
	}
	if (found) {
		list_del(&s_dev->node);
		spin_unlock(&kcompat_simple_dev_list_lock);
		class_device_unregister(&s_dev->class_dev);
	} else {
		spin_unlock(&kcompat_simple_dev_list_lock);
	}
}

#endif /* KCOMPAT_DEFINE_CLASS_DEVICE_CREATE */
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,2) */
/* Define KCOMPAT_GET_CLASS_DEVICE_DEVT(csdev) to get the device number
 * associated with the struct class_device. */
#ifdef KCOMPAT_HAVE_CLASS_DEVICE_DEVT
#define KCOMPAT_GET_CLASS_DEVICE_DEVT(csdev)	((csdev)->devt)
#else
#define KCOMPAT_GET_CLASS_DEVICE_DEVT(csdev)	\
	(kcompat_to_simple_dev((csdev))->dev)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
/* For kernel versions prior to 2.6.13, map onto the class_simple interface. */
static inline struct class_simple *kcompat_class_to_class_simple(
		struct class *cs)	/* Helper function. */
{
	if ((cs == NULL) || IS_ERR(cs)) {
		return (struct class_simple *)cs;
	} else {
		return (struct class_simple *)kcompat_to_class_simple(cs);
	}
}

static inline struct class *
kcompat_class_create(struct module *owner, char *name)
{
	struct kcompat_class_simple *csimple;

	csimple = (struct kcompat_class_simple *)class_simple_create(owner,
			name);
	if (IS_ERR(csimple)) {
		return (struct class *)csimple;
	} else {
		return &csimple->class;
	}
}

/*
 * Define kcompat_class_create(owner, name) as a macro that calls the function
 * of the same name.  This will be tested for later.
 */
#define kcompat_class_create(owner, name) kcompat_class_create(owner, name)

/* Note: class_create() macro is (re)defined below. */
#define class_destroy(cs) \
	class_simple_destroy(kcompat_class_to_class_simple(cs))

#else	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13) */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
static inline struct class *
kcompat_class_create(struct module *owner, const char *name)
{
	/*
	 * Harmlessly cast away const qualifier to match function prototype for
	 * kernel versions prior to 2.6.19.
	 */
	return class_create(owner, (char *)name);
}

/*
 * Define kcompat_class_create(owner, name) as a macro that calls the function
 * of the same name.  This will be tested for later.
 */
#define kcompat_class_create(owner, name) kcompat_class_create(owner, name)

#else	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) */
/*
 * N.B. Red Hat back-ported the 6.4 class_create() function to later 5.14
 * kernels.  Detect this by the fact that class_create will not be defined as
 * a macro in that case.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,4,0) && defined(class_create)
/* Define kcompat_class_create() macro like the existing class_create(). */
/* Note: This uses a GNU GCC extension! */
#define kcompat_class_create(owner, name)	\
({						\
	static struct lock_class_key __key;	\
	__class_create(owner, name, &__key);	\
})
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(6,4,0) && defined(class_create) */
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) */
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
/* Make earlier device class interface look like 2.6.15. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
/* Note: class_device_create macro uses GNU GCC extension! */
/* Note: The 2nd parameter of class_device_create is ignored! */
#define class_device_create(cs, parent, dev, device, fmt...) \
	class_simple_device_add(kcompat_class_to_class_simple(cs), \
			dev, device, fmt)
#define class_device_destroy(cs, dev) \
	class_simple_device_remove(dev)

static inline void kcompat_class_device_unregister(struct class_device *csdev)
{
	if (csdev) {
		dev_t devt;

#ifdef KCOMPAT_HAVE_CLASS_DEVICE_DEVT
		devt = csdev->devt;
#else
		devt = kcompat_to_simple_dev(csdev)->dev;
#endif
		class_simple_device_remove(devt);
	}
}
#undef class_device_unregister
#define class_device_unregister(csdev) kcompat_class_device_unregister(csdev)
#else	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13) */
/* For kernel version 2.6.13 and 2.6.14, define or declare a pointer to the
 * original class_device_create function and redefine the class_device_create
 * macro to call the original function through this pointer.  */
#ifdef KCOMPAT_DEFINE_CLASS_DEVICE_CREATE
struct class_device *(*kcompat_class_device_create)(struct class *,
		dev_t, struct device *, char *, ...)
= class_device_create;
#else
extern struct class_device *(*kcompat_class_device_create)(struct class *,
		dev_t, struct device *, char *, ...);
#endif
/* Redefine class_device_create to call the old function via the pointer. */
/* Note: class_device_create macro uses GNU GCC extension! */
/* Note: The 2nd parameter of class_device_create is ignored! */
#undef class_device_create
#define class_device_create(cs, parent, dev, device, fmt...) \
	kcompat_class_device_create(cs, dev, device, fmt)
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13) */
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15) */
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0) */

/*
 * Kernel version 6.4 removes the module owner parameter from class_create().
 * Red Hat back-ported this to its later 5.14 kernels.  The macro
 * kcompat_class_create(owner, name) will have been defined above if we need
 * to redefine class_create() for kernel versions before 6.4.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,4,0) && defined(kcompat_class_create)
/*
 * Redefine class_create() to use single parameter and use THIS_MODULE as the
 * owner.
 */
#undef class_create
#define class_create(name) kcompat_class_create(THIS_MODULE, name)

#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(6,4,0) && defined(kcompat_class_create) */

#if defined(CLASS_ATTR) || defined(CLASS_ATTR_RO)
/* Can use attributes. */
/*
 * Note: The CLASS_ATTR() macro was removed in kernel 4.13.  Driver code
 * should use the CLASS_ATTR_RO() and CLASS_ATTR_RW() macros instead which
 * were introduced in kernel 3.11 and defined below for compatibility with
 * older kernels.
 */
#define KCOMPAT_HAVE_CLASS_ATTR
#endif

#ifdef __ATTR_NULL
/* Can use list of attributes. */
#define KCOMPAT_USE_ATTR_LIST
#endif

#ifdef __ATTR
#ifndef __ATTR_RW
#define __ATTR_RW(_name) __ATTR(_name, (S_IWUSR | S_IRUGO), \
			_name##_show, _name##_store)
#endif
#endif

#ifdef KCOMPAT_HAVE_CLASS_ATTR
#ifndef CLASS_ATTR_RO
#ifdef __ATTR_RO
#define CLASS_ATTR_RO(_name) \
	struct class_attribute class_attr_##_name = __ATTR_RO(_name)
#else
#define CLASS_ATTR_RO(_name)						\
	struct class_attribute class_attr_##_name = {			\
		.attr = { .name = __stringify(_name),			\
			  .mode = S_IRUGO },				\
		.show = _name##_show,					\
	}
#endif
#endif
#ifndef CLASS_ATTR_RW
#ifdef __ATTR_RW
#define CLASS_ATTR_RW(_name) \
	struct class_attribute class_attr_##_name = __ATTR_RW(_name)
#else
#define CLASS_ATTR_RW(_name)						\
	struct class_attribute class_attr_##_name = {			\
		.attr = { .name = __stringify(_name),			\
			.mode = (S_IWUSR | S_IRUGO) },			\
		.show = _name##_show,					\
	}
#endif
#endif
#endif

#ifdef DEVICE_ATTR
#ifndef DEVICE_ATTR_RO
#ifdef __ATTR_RO
#define DEVICE_ATTR_RO(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_RO(_name)
#else
#define DEVICE_ATTR_RO(_name)						\
	struct device_attribute dev_attr_##_name = {			\
		.attr = { .name = __stringify(_name),			\
			  .mode = S_IRUGO },				\
		.show = _name##_show,					\
	}
#endif
#endif
#ifndef DEVICE_ATTR_RW
#ifdef __ATTR_RW
#define DEVICE_ATTR_RW(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_RW(_name)
#else
#define DEVICE_ATTR_RW(_name)						\
	struct device_attribute dev_attr_##_name = {			\
		.attr = { .name = __stringify(_name),			\
			.mode = (S_IWUSR | S_IRUGO) },			\
		.show = _name##_show,					\
	}
#endif
#endif
#endif

/*
 * 'struct class_device' was removed in 2.6.26.  Have to use alternate code.
 * This affects lots of stuff.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
#define KCOMPAT_NO_CLASS_DEVICE
#endif

/*
 * 'device_create()' appeared in 2.6.18, but don't use it until 2.6.19
 * because the 2.6.18 version breaks if 'parent' is NULL.  Also, the 'fmt'
 * parameter was made a const pointer in 2.6.19.
 *
 * The API changed in 2.6.27.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
#define KCOMPAT_HAVE_DEVICE_CREATE
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
/* 'device_create()' is missing the 'drvdata' parameter. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26)
/* For 2.6.26 we can just substitute with 'device_create_drvdata()'. */
#undef device_create
/* Note: device_create macro uses a GNU GCC extension. */
#define device_create(cls, parent, devt, drvdata, fmt...) \
	device_create_drvdata(cls, parent, devt, drvdata, fmt)
#else	/* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26) */
/* For 2.6.19 to 2.6.25, define or declare a pointer to the original
 * 'device_create()' function and redefine the 'device_create()' macro to
 * call this function through the pointer. */
#ifdef KCOMPAT_DEFINE_DEVICE_CREATE
struct device *(*kcompat_device_create_2_6_19)(struct class *,
		struct device *, dev_t, const char *, ...) = device_create;
#else	/* KCOMPAT_DEFINE_DEVICE_CREATE */
extern struct device *(*kcompat_device_create_2_6_19)(struct class *,
		struct device *, dev_t, const char *, ...);
#endif	/* KCOMPAT_DEFINE_DEVICE_CREATE */
/* Redefine device_create to call the old function via the pointer, and then
 * call dev_set_drvdata on the returned struct device.  Note that this suffers
 * from a race condition. */
/* Note: device_create macro uses a GNU GCC extension. */
#undef device_create
#define device_create(cls, parent, devt, drvdata, fmt...)		\
({									\
	struct device *__ret =						\
		kcompat_device_create_2_6_19(cls, parent, devt, fmt);	\
	if (__ret) {							\
		dev_set_drvdata(__ret, drvdata);			\
	}								\
	__ret;								\
})
#endif	/* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26) */
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27) */
#endif	/* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19) */

/*
 * The 'dev_groups' member of 'struct class' appeared in 3.11 and the
 * 'dev_attrs' member was removed in 3.12.  Define
 * 'KCOMPAT_USE_CLASS_DEV_GROUPS' if the dev_groups member should be
 * used.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,11,0)
#define KCOMPAT_USE_CLASS_DEV_GROUPS
#endif

/*
 * `csdev_printk(level, cd, format, ...)` and friends are our own
 * invention as a convenient replacement for `dev_printk` and friends
 * for "class" devices.
 *
 * If `KCOMPAT_NO_CLASS_DEVICE` was set earlier (for kernel 2.6.26 and
 * later), the `cd` parameter is a `struct device *` and the macros
 * just use `dev_printk(level, cd, format, ...)`.
 *
 * If `KCOMPAT_NO_CLASS_DEVICE` was not set (for kernels 2.6.25 and
 * earlier), the `cd` parameter is a `struct class_device *` and we pass
 * the `class_id[]` string in the `struct class_device` through to
 * `printk`.
 */
#ifdef KCOMPAT_NO_CLASS_DEVICE
#define csdev_printk dev_printk
#else
static inline const char *kcompat_csdev_name(struct class_device *cd)
{
	return cd ? cd->class_id : "(null)";
}
#define csdev_printk(level, cd, format, arg...)	\
	printk(level "%s: " format, kcompat_csdev_name(cd), ## arg)
#endif	/* KCOMPAT_NO_CLASS_DEVICE */

#define csdev_emerg(cd, format, arg...) \
	csdev_printk(KERN_EMERG, cd, format, ## arg)
#define csdev_alert(cd, format, arg...) \
	csdev_printk(KERN_ALERT, cd, format, ## arg)
#define csdev_crit(cd, format, arg...) \
	csdev_printk(KERN_CRIT, cd, format, ## arg)
#define csdev_err(cd, format, arg...) \
	csdev_printk(KERN_ERR, cd, format, ## arg)
#define csdev_warn(cd, format, arg...) \
	csdev_printk(KERN_WARNING, cd, format, ## arg)
#define csdev_notice(cd, format, arg...) \
	csdev_printk(KERN_NOTICE, cd, format, ## arg)
#define csdev_info(cd, format, arg...) \
	csdev_printk(KERN_INFO, cd, format, ## arg)
#define csdev_info(cd, format, arg...) \
	csdev_printk(KERN_INFO, cd, format, ## arg)

#ifdef DEBUG
#define csdev_dbg(cd, format, arg...) \
	csdev_printk(KERN_DEBUG, cd, format, ## arg)
#else
#define csdev_dbg(cd, format, arg...)				\
({								\
	if (0)							\
		csdev_printk(KERN_DEBUG, cd, format, ## arg);	\
	0;							\
})
#endif

#ifdef VERBOSE_DEBUG
#define csdev_vdbg csdev_dbg
#else
#define csdev_vdbg(cd, format, arg...)				\
({								\
	if (0)							\
		csdev_printk(KERN_DEBUG, cd, format, ## arg);	\
	0;							\
})
#endif

#include <linux/fs.h>

/*
 * HAVE_COMPAT_IOCTL and HAVE_UNLOCKED_IOCTL were first defined in
 * kernel version 2.6.11 but removed in kernel version 5.9.
 *
 * Ensure we keep them defined even for kernel version 5.9 onwards.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,11)
#undef HAVE_COMPAT_IOCTL
#define HAVE_COMPAT_IOCTL 1
#undef HAVE_UNLOCKED_IOCTL
#define HAVE_UNLOCKED_IOCTL 1
#endif

#ifndef CONFIG_COMPAT
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)
#ifdef __x86_64__
#define CONFIG_COMPAT	1
#endif
#endif
#endif

#ifdef CONFIG_COMPAT
#ifndef HAVE_COMPAT_IOCTL
#include <asm/ioctl32.h>
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,44)
#include <linux/ioctl32.h>
#endif
#endif /* HAVE_COMPAT_IOCTL */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,53)
typedef __u32 compat_size_t;
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,66)
typedef __u32 compat_uptr_t;
static inline void *compat_ptr(compat_uptr_t uptr)
{
	return (void *)(unsigned long)uptr;
}
#endif
#ifdef __x86_64__
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,70)
static __inline__ void *compat_alloc_user_space(long len)
{
	struct pt_regs *regs = (struct pt_regs *)((char *)current->thread.rsp0
			- sizeof(struct pt_regs));
	return (char *)regs->rsp - len;
}
#endif
#endif	/* __x86_64__ */
/* What about other architectures? */
#endif

/* The 'ioctl' member of 'struct file_operations' has been removed from
 * the 2.6.36 kernel onwards. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,36)
#define KCOMPAT_HAVE_FOP_IOCTL
#endif

/* The 'struct dentry *' parameter of the fsync file operation has been
 * removed from the 2.6.35 kernel onwards. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35)
#define KCOMPAT_FOP_FSYNC_HAS_DENTRY
#endif

/* The fsync file operation was changed in 3.1.0 kernel to push down the
 * calling of filemap_write_and_wait() and the taking on the inode i_mutex,
 * if required.  Parameters 'start' and 'end' were added.  */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,1,0)
#define KCOMPAT_FOP_FSYNC_HAS_START_END
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,51)
#include <linux/compat.h>
#endif

/*
 * The file_dentry() inline function was added in kernel version 4.6.0.
 * Emulate it for earlier kernels.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,6,0)
#undef file_dentry
#define file_dentry(f) kcompat_file_dentry(f)
static inline struct dentry *file_dentry(const struct file *f)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
	return f->f_dentry;
#else
	return f->f_path.dentry;
#endif
}
#endif

/*
 * The file_inode() inline function was added in kernel 3.9.0.
 * Emulate it for earlier kernels.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0)
#undef file_inode
#define file_inode(f)	kcompat_file_inode(f)
static inline struct inode *file_inode(struct file *f)
{
	return file_dentry(f)->d_inode;
}
#endif

#ifndef SEEK_SET
#define SEEK_SET	0
#endif

#ifndef SEEK_CUR
#define SEEK_CUR	1
#endif

#ifndef SEEK_END
#define SEEK_END	2
#endif

/*
 * The fixed_size_llseek() helper was added in kernel 3.11.0.
 * Emulate it for earlier kernels.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)
static inline loff_t fixed_size_llseek(struct file *file, loff_t offset,
				       int whence, loff_t size)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	mutex_lock(&file_inode(file)->i_mutex);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_lock(&file->f_mapping->host->i_mutex);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,2)
	down(&file->f_mapping->host->i_sem);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,3)
	down(&file_inode(file)->i_mapping->host->i_sem);
#endif
	switch (whence) {
	case SEEK_END:
		offset += size;
		break;
	case SEEK_CUR:
		if (offset == 0) {
			offset = file->f_pos;
			goto end;
		}
		offset += file->f_pos;
		break;
	case SEEK_SET:
		break;
	default:
		offset = -EINVAL;
		goto end;
	}
	if (offset < 0 || offset > size) {
		offset = -EINVAL;
		goto end;
	}
	if (offset != file->f_pos) {
		file->f_pos = offset;
		/* XXX should we set file->f_version? */
	}
end:
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
	mutex_unlock(&file_inode(file)->i_mutex);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
	mutex_unlock(&file->f_mapping->host->i_mutex);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,2)
	up(&file->f_mapping->host->i_sem);
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,3)
	up(&file_inode(file)->i_mapping->host->i_sem);
#endif
	return offset;
}
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,6,0)
static inline loff_t fixed_size_llseek(struct file *file, loff_t offset,
				       int whence, loff_t size)
{
	switch (whence) {
	case SEEK_END:
		offset += size;
		whence = SEEK_SET;
		/* fall through */
	case SEEK_SET:
	case SEEK_CUR:
		return generic_file_llseek_size(file, offset, whence, size);
	default:
		return -EINVAL;
	}
}
#elif LINUX_VERSION_CODE < KERNEL_VERSION(3,11,0)
static inline loff_t fixed_size_llseek(struct file *file, loff_t offset,
				       int whence, loff_t size)
{
	switch (whence) {
	case SEEK_SET:
	case SEEK_CUR:
	case SEEK_END:
		return generic_file_llseek_size(file, offset, whence,
						size, size);
	default:
		return -EINVAL;
	}
}
#endif

/*
 * The imajor() and iminor() inline functions were added in kernel version
 * 2.6.0.  Emulate them for earlier kernel versions.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
static inline unsigned int kcompat_iminor(struct inode *inode)
{
	return MINOR(inode->i_rdev);
}
#undef iminor
#define iminor(inode) kcompat_iminor(inode)

static inline unsigned int kcompat_imajor(struct inode *inode)
{
	return MAJOR(inode->i_rdev);
}
#undef imajor
#define imajor(inode) kcompat_imajor(inode)
#endif

#include <linux/interrupt.h>
#ifndef IRQ_RETVAL
/* Define types and macros so we can use:
 *   return IRQ_NONE;
 *   return IRQ_HANDLED;
 *   return IRQ_RETVAL(x);
 */
typedef void irqreturn_t;
#define IRQ_NONE
#define IRQ_HANDLED
#define IRQ_RETVAL(x)
#endif

/* New interrupt flags. */
#ifndef IRQF_SHARED
#define IRQF_DISABLED		SA_INTERRUPT
#define IRQF_SAMPLE_RANDOM	SA_SAMPLE_RANDOM
#define IRQF_SHARED		SA_SHIRQ
#define IRQF_PROBE_SHARED	SA_PROBEIRQ
#define IRQF_PERCPU		SA_PERCPU
#ifdef SA_TRIGGER_MASK
#define IRQF_TRIGGER_NONE	0
#define IRQF_TRIGGER_LOW	SA_TRIGGER_LOW
#define IRQF_TRIGGER_HIGH	SA_TRIGGER_HIGH
#define IRQF_TRIGGER_FALLING	SA_TRIGGER_FALLING
#define IRQF_TRIGGER_RISING	SA_TRIGGER_RISING
#define IRQF_TRIGGER_MASK	SA_TRIGGER_MASK
#else
#define IRQF_TRIGGER_NONE	0
#define IRQF_TRIGGER_LOW	0
#define IRQF_TRIGGER_HIGH	0
#define IRQF_TRIGGER_FALLING	0
#define IRQF_TRIGGER_RISING	0
#define IRQF_TRIGGER_MASK	0
#endif
#endif

/* Define KCOMPAT_IRQ_HANDLER_HAS_REGS if IRQ handler has third
 * 'struct pt_regs *' parameter. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
#define KCOMPAT_IRQ_HANDLER_HAS_REGS
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
typedef unsigned long resource_size_t;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,6) || \
	LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0)
#undef tasklet_hi_enable
#define tasklet_hi_enable(t)	tasklet_enable(t)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,9)

/* The function no_llseek won't exist unless KCOMPAT_DEFINE_NO_LLSEEK is
 * defined in one of the .c files before including kcompat.h. */
loff_t no_llseek(struct file *file, loff_t offset, int origin);

#ifdef KCOMPAT_DEFINE_NO_LLSEEK

/* Define no_llseek with external linkage. */

#include <linux/errno.h>

loff_t no_llseek(struct file *file, loff_t offset, int origin)
{
	return -ESPIPE;
}

#endif	/* KCOMPAT_DEFINE_NO_LLSEEK */

#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,4,9) */

/* nonseekable_open is called from the open routine to disable seek operations.
 * The llseek operation should be set to no_llseek.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)
static inline int nonseekable_open(struct inode *inode, struct file *file)
{
	return 0;
}
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8) */

/*
 * For kernel version 4.11 and later, various stuff to do with signal
 * handling has moved from <linux/sched.h> to <linux/sched/signal.h>.
 */
#include <linux/sched.h>

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
#define KCOMPAT_HAVE_LINUX_SCHED_SIGNAL_H
#endif

#ifdef KCOMPAT_HAVE_LINUX_SCHED_SIGNAL_H
#include <linux/sched/signal.h>
#endif

/* wait_event_timeout and wait_event_interruptible_timeout */
#include <linux/wait.h>

/*
 * wait_queue_t type was renamed to wait_event_entry_t in kernel 4.13.
 * Define the new name for use with older kernels.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,13,0)
typedef wait_queue_t wait_queue_entry_t;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,39)
#define DEFINE_WAIT(name)	DECLARE_WAITQUEUE(name, current)
#define init_wait(wait)		init_waitqueue_entry((wait), current)

static inline void prepare_to_wait(wait_queue_head_t *q,
		wait_queue_entry_t *wait, int state)
{
	__set_current_state(state);
	add_wait_queue(q, wait);
}

static inline void prepare_to_wait_exclusive(wait_queue_head_t *q,
		wait_queue_entry_t *wait, int state)
{
	__set_current_state(state);
	add_wait_queue_exclusive(q, wait);
}

static inline void finish_wait(wait_queue_head_t *q, wait_queue_entry_t *wait)
{
	__set_current_state(TASK_RUNNING);
	remove_wait_queue(q, wait);
}
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,5,39) */

#ifndef wait_event
#define __wait_event(wq, condition)					\
do {									\
	wait_queue_entry_t __wait;					\
	init_waitqueue_entry(&__wait, current);				\
									\
	add_wait_queue(&wq, &__wait);					\
	for (;;) {							\
		set_current_state(TASK_UNINTERRUPTIBLE);		\
		if (condition)						\
			break;						\
		schedule();						\
	}								\
	current->state = TASK_RUNNING;					\
	remove_wait_queue(&wq, &__wait);				\
} while (0)

#define wait_event(wq, condition)					\
do {									\
	if (condition)							\
		break;							\
	__wait_event(wq, condition);					\
} while (0)
#endif	/* wait_event */

#ifndef wait_event_interruptible
#define __wait_event_interruptible(wq, condition, ret)			\
do {									\
	wait_queue_entry_t __wait;					\
	init_waitqueue_entry(&__wait, current);				\
									\
	add_wait_queue(&wq, &__wait);					\
	for (;;) {							\
		set_current_state(TASK_INTERRUPTIBLE);			\
		if (condition)						\
			break;						\
		if (!signal_pending(current)) {				\
			schedule();					\
			continue;					\
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	current->state = TASK_RUNNING;					\
	remove_wait_queue(&wq, &__wait);				\
} while (0)

/* Note: this uses a GCC extension! */
#define wait_event_interruptible(wq, condition)				\
({									\
	int __ret = 0;							\
	if (!(condition))						\
		__wait_event_interruptible(wq, condition, __ret);	\
	__ret;								\
})
#endif	/* wait_event_interruptible */

#ifndef wait_event_timeout
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8)
#define __wait_event_timeout(wq, condition, ret)			\
do {									\
	wait_queue_entry_t __wait;					\
	init_waitqueue_entry(&__wait, current);				\
									\
	add_wait_queue(&wq, &__wait);					\
	for (;;) {							\
		set_current_state(TASK_UNINTERRUPTIBLE);		\
		if (condition)						\
			break;						\
		schedule_timeout(ret);					\
		if (!ret)						\
			break;						\
	}								\
	current->state = TASK_RUNNING;					\
	remove_wait_queue(&wq, &__wait);				\
} while (0)
#else
#define __wait_event_timeout(wq, condition, ret)			\
do {									\
	DEFINE_WAIT(__wait);						\
									\
	for (;;) {							\
		prepare_to_wait(&wq, &__wait, TASK_UNINTERRUPTIBLE);	\
		if (condition)						\
			break;						\
		schedule_timeout(ret);					\
		if (!ret)						\
			break;						\
	}								\
	finish_wait(&wq, &__wait);					\
} while (0)
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,8) */
/* Note: this uses a GCC extension! */
#define wait_event_timeout(wq, condition, timeout)			\
({									\
	long __ret = timeout;						\
	if (!(condition))						\
		__wait_event_timeout(wq, condition, __ret);		\
	__ret;								\
})
#endif	/* wait_event_timeout */

#ifndef wait_event_interruptible_timeout
#define __wait_event_interruptible_timeout(wq, condition, ret)		\
do {									\
	wait_queue_entry_t __wait;					\
	init_waitqueue_entry(&__wait, current);				\
									\
	add_wait_queue(&wq, &__wait);					\
	for (;;) {							\
		set_current_state(TASK_INTERRUPTIBLE);			\
		if (condition)						\
			break;						\
		if (!signal_pending(current)) {				\
			ret = schedule_timeout(ret);			\
			if (!ret)					\
				break;					\
			continue;					\
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	current->state = TASK_RUNNING;					\
	remove_wait_queue(&wq, &__wait);				\
} while (0)

/* Note: this uses a GCC extension! */
#define wait_event_interruptible_timeout(wq, condition, timeout)	\
({									\
	long __ret = timeout;						\
	if (!(condition))						\
		__wait_event_interruptible_timeout(wq, condition, __ret); \
	__ret;								\
})
#endif	/* wait_event_interruptible_timeout */

#ifndef wait_event_interruptible_exclusive
#define __wait_event_interruptible_exclusive(wq, condition, ret)	\
do {									\
	wait_queue_entry_t __wait;					\
	init_waitqueue_entry(&__wait, current);				\
									\
	add_wait_queue_exclusive(&wq, &__wait);				\
	for (;;) {							\
		set_current_state(TASK_INTERRUPTIBLE);			\
		if (condition)						\
			break;						\
		if (!signal_pending(current)) {				\
			schedule();					\
			continue;					\
		}							\
		ret = -ERESTARTSYS;					\
		break;							\
	}								\
	current->state = TASK_RUNNING;					\
	remove_wait_queue(&wq, &__wait);				\
} while (0)

/* Note: this uses a GCC extension! */
#define wait_event_interruptible_exclusive(wq, condition)		\
({									\
	int __ret = 0;							\
	if (!(condition))						\
		__wait_event_interruptible_exclusive(wq, condition, __ret); \
	__ret;								\
})
#endif /* wait_event_interruptible_exclusive */

/* Define KCOMPAT_HAVE_LINUX_COMPLETION_H if kernel has <linux/completion.h>. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,7)
#define KCOMPAT_HAVE_LINUX_COMPLETION_H
#endif

#ifdef KCOMPAT_HAVE_LINUX_COMPLETION_H
#include <linux/completion.h>
#else

struct completion {
	unsigned int done;
	wait_queue_head_t wait;
};

#define COMPLETION_INITIALIZER(work) \
	{ 0, __WAIT_QUEUE_HEAD_INITIALIZER((work).wait) }

#define DECLARE_COMPLETION(work) \
	struct completion work = COMPLETION_INITIALIZER(work)

#define INIT_COMPLETION(x)	((x).done = 0)

static inline void init_completion(struct completion *x)
{
	x->done = 0;
	init_waitqueue_head(&x->wait);
}

static inline void wait_for_completion(struct completion *x)
{
	spin_lock_irq(&x->wait.lock);
	if (!x->done) {
		DECLARE_WAITQUEUE(wait, current);

		wait.flags |= WQ_FLAG_EXCLUSIVE;
		__add_wait_queue_tail(&x->wait, &wait);
		do {
			__set_current_state(TASK_UNINTERRUPTIBLE);
			spin_unlock_irq(&x->wait.lock);
			schedule();
			spin_lock_irq(&x->wait.lock);
		} while (!x->done);
		__remove_wait_queue(&x->wait, &wait);
	}
	x->done--;
	spin_unlock_irq(&x->wait.lock);
}

static inline void complete(struct completion *x)
{
	unsigned long flags;

	spin_lock_irqsave(&x->wait.lock, flags);
	x->done++;
	spin_unlock_irqrestore(&x->wait.lock, flags);
	wake_up(&x->wait);
}

#endif	/* KCOMPAT_HAVE_LINUX_COMPLETION_H */

/* reinit_completion(x) was added in kernel version 3.13.0, replacing the
 * INIT_COMPLETION(x) macro that did the same thing.  */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,13,0)
static inline void reinit_completion(struct completion *x)
{
	x->done = 0;
}
#endif

/* complete_all(x) was added in kernel version 2.5.51. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,51)
static inline void complete_all(struct completion *x)
{
	unsigned long flags;

	spin_lock_irqsave(&x->wait.lock, flags);
	x->done += UINT_MAX / 2;
	spin_unlock_irqrestore(&x->wait.lock, flags);
	wake_up_all(&x->wait);
}
#endif

/* wait_for_completion_interruptible(x),
 * wait_for_completion_timeout(x, timeout), and
 * wait_for_completion_interruptible_timeout(x, timeout) were added in
 * kernel kernel 2.6.11.  */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
static inline unsigned long
wait_for_completion_timeout(struct completion *x, unsigned long timeout)
{
	spin_lock_irq(&x->wait.lock);
	if (!x->done) {
		DECLARE_WAITQUEUE(wait, current);

		wait.flags |= WQ_FLAG_EXCLUSIVE;
		__add_wait_queue_tail(&x->wait, &wait);
		do {
			__set_current_state(TASK_UNINTERRUPTIBLE);
			spin_unlock_irq(&x->wait.lock);
			timeout = schedule_timeout(timeout);
			spin_lock_irq(&x->wait.lock);
			if (!timeout) {
				__remove_wait_queue(&x->wait, &wait);
				goto out;
			}
		} while (!x->done);
		__remove_wait_queue(&x->wait, &wait);
	}
	x->done--;
out:
	spin_unlock_irq(&x->wait.lock);
	return timeout;
}

static inline int wait_for_completion_interruptible(struct completion *x)
{
	int ret = 0;

	spin_lock_irq(&x->wait.lock);
	if (!x->done) {
		DECLARE_WAITQUEUE(wait, current);

		wait.flags |= WQ_FLAG_EXCLUSIVE;
		__add_wait_queue_tail(&x->wait, &wait);
		do {
			if (signal_pending(current)) {
				ret = -ERESTARTSYS;
				__remove_wait_queue(&x->wait, &wait);
				goto out;
			}
			__set_current_state(TASK_INTERRUPTIBLE);
			spin_unlock_irq(&x->wait.lock);
			schedule();
			spin_lock_irq(&x->wait.lock);
		} while (!x->done);
		__remove_wait_queue(&x->wait, &wait);
	}
	x->done--;
out:
	spin_unlock_irq(&x->wait.lock);

	return ret;
}

static inline long
wait_for_completion_interruptible_timeout(struct completion *x,
		unsigned long timeout)
{
	spin_lock_irq(&x->wait.lock);
	if (!x->done) {
		DECLARE_WAITQUEUE(wait, current);

		wait.flags |= WQ_FLAG_EXCLUSIVE;
		__add_wait_queue_tail(&x->wait, &wait);
		do {
			if (signal_pending(current)) {
				timeout = -ERESTARTSYS;
				__remove_wait_queue(&x->wait, &wait);
				goto out;
			}
			__set_current_state(TASK_INTERRUPTIBLE);
			spin_unlock_irq(&x->wait.lock);
			timeout = schedule_timeout(timeout);
			spin_lock_irq(&x->wait.lock);
			if (!timeout) {
				__remove_wait_queue(&x->wait, &wait);
				goto out;
			}
		} while (!x->done);
		__remove_wait_queue(&x->wait, &wait);
	}
	x->done--;
out:
	spin_unlock_irq(&x->wait.lock);
	return timeout;
}
#endif

#ifndef COMPLETION_INITIALIZER_ONSTACK
#define COMPLETION_INITIALIZER_ONSTACK(work) \
	({ init_completion(&work); work; })
#endif

#ifndef DECLARE_COMPLETION_ONSTACK
#ifdef CONFIG_LOCKDEP
#define DECLARE_COMPLETION_ONSTACK(work) \
	struct completion work = COMPLETION_INITIALIZER_ONSTACK(work)
#else
#define DECLARE_COMPLETION_ONSTACK(work) DECLARE_COMPLETION(work)
#endif
#endif

/* KCOMPAT_HAVE_TRY_WAIT_FOR_COMPLETION may have been defined outside this file
 * for compatibility with some Red Hat 2.6.18 kernels. */
#ifndef KCOMPAT_HAVE_TRY_WAIT_FOR_COMPLETION
/* Define KCOMPAT_HAVE_TRY_WAIT_FOR_COMPLETION for kernel version 2.6.27
 * onwards. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define KCOMPAT_HAVE_TRY_WAIT_FOR_COMPLETION
#endif
#endif

#ifndef KCOMPAT_HAVE_TRY_WAIT_FOR_COMPLETION
/* Define try_wait_for_completion() for compatibility. */
/* Note: original return type is 'bool' but it might not be defined. */
static inline int try_wait_for_completion(struct completion *x)
{
	int ret = 1;

	spin_lock_irq(&x->wait.lock);
	if (!x->done)
		ret = 0;
	else
		x->done--;
	spin_unlock_irq(&x->wait.lock);
	return ret;
}
#endif

/* KCOMPAT_HAVE_COMPLETION_DONE may have been defined outside this file
 * for compatibility with some Red Hat 2.6.18 kernels. */
#ifndef KCOMPAT_HAVE_COMPLETION_DONE
/* Define KCOMPAT_HAVE_COMPLETION_DONE for kernel version 2.6.27 onwards. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define KCOMPAT_HAVE_COMPLETION_DONE
#endif
#endif

#ifndef KCOMPAT_HAVE_COMPLETION_DONE
/* Define completion_done() for compatibility. */
/* Note: original return type is 'bool' but it might not be defined. */
static inline int completion_done(struct completion *x)
{
	int ret = 1;

	spin_lock_irq(&x->wait.lock);
	if (!x->done)
		ret = 0;
	spin_unlock_irq(&x->wait.lock);
	return ret;
}
#endif

/*
 * TODO: add support for wait_for_completion_killable() and friends, but
 * they cannot be supported properly before kernel version 2.6.25.
 *
 * Could also add support for wait_for_completion_io() and friends, but
 * they are only used by the block layer so not really needed for drivers.
 */

/* msleep, ssleep and a few conversions between jiffies and standard time
 * units. */
#include <linux/delay.h>
#include <linux/time.h>

#ifndef MSEC_PER_SEC
#define MSEC_PER_SEC	1000L
#endif
#ifndef USEC_PER_MSEC
#define USEC_PER_MSEC	1000L
#endif
#ifndef NSEC_PER_USEC
#define NSEC_PER_USEC	1000L
#endif
#ifndef NSEC_PER_MSEC
#define NSEC_PER_MSEC	1000000L
#endif
#ifndef USEC_PER_SEC
#define USEC_PER_SEC	1000000L
#endif
#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC	1000000000L
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,29) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0) && \
	 LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7))
static inline unsigned int jiffies_to_msecs(const unsigned long j)
{
	if ((HZ <= MSEC_PER_SEC) && !(MSEC_PER_SEC % HZ)) {
		return (MSEC_PER_SEC / HZ) * j;
	} else if ((HZ > MSEC_PER_SEC) && !(HZ % MSEC_PER_SEC)) {
		/* gcc emits 'warning: division by zero' here, but this code
		 * is unreachable in that case. */
		return (j + (HZ / MSEC_PER_SEC) - 1) / (HZ / MSEC_PER_SEC);
	} else {
		return (j * MSEC_PER_SEC) / HZ;
	}
}

static inline unsigned long msecs_to_jiffies(const unsigned int m)
{
	if (m > jiffies_to_msecs(MAX_JIFFY_OFFSET)) {
		return MAX_JIFFY_OFFSET;
	}
	if ((HZ <= MSEC_PER_SEC) && !(MSEC_PER_SEC % HZ)) {
		return (m + (MSEC_PER_SEC / HZ) - 1) / (MSEC_PER_SEC / HZ);
	} else if ((HZ > MSEC_PER_SEC) && !(HZ % MSEC_PER_SEC)) {
		return m * (HZ / MSEC_PER_SEC);
	} else {
		return (m * HZ + MSEC_PER_SEC - 1) / MSEC_PER_SEC;
	}
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,29) || \
	(LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0) && \
	 LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9))
static inline unsigned int jiffies_to_usecs(const unsigned long j)
{
	if ((HZ <= USEC_PER_SEC) && !(USEC_PER_SEC % HZ)) {
		return (USEC_PER_SEC / HZ) * j;
	} else if ((HZ > USEC_PER_SEC) && !(HZ % USEC_PER_SEC)) {
		/* gcc emits 'warning: division by zero' here, but this code
		 * is unreachable in that case. */
		return (j + (HZ / USEC_PER_SEC) - 1) / (HZ / USEC_PER_SEC);
	} else {
		return (j * USEC_PER_SEC) / HZ;
	}
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,11)
static inline unsigned long usecs_to_jiffies(const unsigned int u)
{
	if (u > jiffies_to_usecs(MAX_JIFFY_OFFSET)) {
		return MAX_JIFFY_OFFSET;
	}
	if ((HZ <= USEC_PER_SEC) && !(USEC_PER_SEC % HZ)) {
		return (u + (USEC_PER_SEC / HZ) - 1) / (USEC_PER_SEC / HZ);
	} else if ((HZ > USEC_PER_SEC) && !(HZ % USEC_PER_SEC)) {
		return u * (HZ / USEC_PER_SEC);
	} else {
		return (u * HZ + USEC_PER_SEC - 1) / USEC_PER_SEC;
	}
}
#endif

#ifndef time_after
#define time_after(a, b)	((long)(b) - (long)(a) < 0)
#endif

#ifndef time_before
#define time_before(a, b)	time_after(b, a)
#endif

#ifndef time_after_eq
#define time_after_eq(a, b)	((long)(a) - (long)(b) >= 0)
#endif

#ifndef time_before_eq
#define time_before_eq(a, b)	time_after_eq(b, a)
#endif

#ifndef time_in_range
#define time_in_range(a, b, c)	(time_after_eq(a, b) && time_before_eq(a, c))
#endif

#ifndef time_in_range_open
#define time_in_range_open(a, b, c) (time_after_eq(a, b) && time_before(a, c))
#endif

#ifndef time_after64
#define time_after64(a, b)	((__s64)(b) - (__s64)(a) < 0)
#endif

#ifndef time_before64
#define time_before64(a, b)	time_after64(b, a)
#endif

#ifndef time_after_eq64
#define time_after_eq64(a, b)	((__s64)(a) - (__s64)(b) >= 0)
#endif

#ifndef time_before_eq64
#define time_before_eq64(a, b)	time_after_eq64(b, a)
#endif

#ifndef time_in_range64
#define time_in_range64(a, b, c) (time_after_eq64(a, b) && time_before_eq(a, c))
#endif


#ifndef time_is_before_jiffies
#define time_is_before_jiffies(a)	time_after(jiffies, a)
#endif

#ifndef time_is_before_jiffies64
#define time_is_before_jiffies64(a)	time_after64(get_jiffies_64(), a)
#endif

#ifndef time_is_after_jiffies
#define time_is_after_jiffies(a)	time_before(jiffies, a)
#endif

#ifndef time_is_after_jiffies64
#define time_is_after_jiffies64(a)	time_before64(get_jiffies_64(), a)
#endif

#ifndef time_is_before_eq_jiffies
#define time_is_before_eq_jiffies(a)	time_after_eq(jiffies, a)
#endif

#ifndef time_is_before_eq_jiffies64
#define time_is_before_eq_jiffies64(a)	time_after_eq64(get_jiffies_64(), a)
#endif

#ifndef time_is_after_eq_jiffies
#define time_is_after_eq_jiffies(a)	time_before_eq(jiffies, a)
#endif

#ifndef time_is_after_eq_jiffies64
#define time_is_after_eq_jiffies64(a)	time_before_eq64(get_jiffies_64(), a)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,7)
static inline void kcompat_msleep(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs);

	while (timeout) {
		set_current_state(TASK_UNINTERRUPTIBLE);
		timeout = schedule_timeout(timeout);
	}
}
#undef msleep
#define msleep(msecs)	kcompat_msleep(msecs)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
static inline long msleep_interruptible(unsigned int msecs)
{
	unsigned long timeout = msecs_to_jiffies(msecs);

	while (timeout && !signal_pending(current)) {
		set_current_state(TASK_INTERRUPTIBLE);
		timeout = schedule_timeout(timeout);
	}
	return jiffies_to_msecs(timeout);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,9)
static inline void kcompat_ssleep(unsigned int secs)
{
	msleep(secs * 1000);
}
#undef ssleep
#define ssleep(secs)	kcompat_ssleep(secs)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)) || \
	((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)) && \
	 (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,25)))
static inline void kcompat_set_normalized_timespec(struct timespec *ts,
		time_t sec, long nsec)
{
	while (nsec >= NSEC_PER_SEC) {
		nsec -= NSEC_PER_SEC;
		++sec;
	}
	while (nsec < 0) {
		nsec += NSEC_PER_SEC;
		--sec;
	}
	ts->tv_sec = sec;
	ts->tv_nsec = nsec;
}
/* Redefine set_normalized_timespec() as a macro calling the above. */
#undef set_normalized_timespec
#define set_normalized_timespec(ts, sec, nsec) \
	kcompat_set_normalized_timespec(ts, sec, nsec)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,48)
static inline int timespec_equal(const struct timespec *a,
		const struct timespec *b)
{
	return (a->tv_sec == b->tv_sec) && (a->tv_nsec == b->tv_nsec);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
static inline struct timespec kcompat_timespec_sub(struct timespec lhs,
		struct timespec rhs)
{
	struct timespec ts_delta;

	set_normalized_timespec(&ts_delta, lhs.tv_sec - rhs.tv_sec,
			lhs.tv_nsec - rhs.tv_nsec);
	return ts_delta;
}
/* Redefine timespec_sub() as a macro calling the above. */
#undef timespec_sub
#define timespec_sub(lhs, rhs)	kcompat_timespec_sub(lhs, rhs)
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,18)
static inline void timespec_add_ns(struct timespec *a, u64 ns)
{
	ns += a->tv_nsec;
	while (unlikely(ns >= NSEC_PER_SEC)) {
		ns -= NSEC_PER_SEC;
		a->tv_sec++;
	}
}
#endif

/* get_jiffies_64() */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,60)

/* Provide get_jiffies_64() for older kernels.  32-bit kernels require
 * special attention. */

#if (BITS_PER_LONG == 32)

/* Provide our own get_jiffies_64() for older kernels.  Note that this
 * has to be called at regular intervals (at least every MAX_JIFFY_OFFSET)
 * to avoid missing time, and the returned time is local to the driver,
 * not system-global. */

/* Driver can test this to see if using local get_jiffies_64(). */
#define KCOMPAT_USE_OWN_GET_JIFFIES_64

#ifdef KCOMPAT_DEFINE_GET_JIFFIES_64
u64 kcompat_get_jiffies_64(void)
{
	static spinlock_t lock = SPIN_LOCK_UNLOCKED;
	static unsigned long old_jiffies = 0, high_jiffies = 0;
	unsigned long new_jiffies;
	unsigned long flags;
	u64 ret_jiffies_64;

	spin_lock_irqsave(&lock, flags);
	new_jiffies = jiffies;
	if (new_jiffies < old_jiffies) {
		++high_jiffies;
	}
	old_jiffies = new_jiffies;
	ret_jiffies_64 = ((u64)high_jiffies << 32) | new_jiffies;
	spin_unlock_irqrestore(&lock, flags);
	return ret_jiffies_64;
}
#else	/* KCOMPAT_DEFINE_GET_JIFFIES_64 */
extern u64 kcompat_get_jiffies_64(void);
#endif	/* KCOMPAT_DEFINE_GET_JIFFIES_64 */

#else	/* (BITS_PER_LONG == 32) */

/* Assume jiffies is 64-bit already. */
static inline u64 kcompat_get_jiffies_64(void)
{
	return jiffies;
}

#endif	/* (BITS_PER_LONG == 32) */

#undef get_jiffies_64
#define get_jiffies_64()	kcompat_get_jiffies_64()

#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,5,60) */

#include <linux/fs.h>	/* Defines DEFINE_MUTEX() if it exists (2.6.16) */

#ifndef DEFINE_MUTEX
/* Use semaphore instead of mutex for kernel version < 2.6.16 */
#define mutex				semaphore
#define DEFINE_MUTEX(m)			DECLARE_MUTEX(m)
#define mutex_init(m)			init_MUTEX(m)
#define mutex_destroy(m)		do {} while(0)
#define mutex_lock(m)			down(m)
#define mutex_lock_interruptible(m)	down_interruptible(m)
#define mutex_trylock(m)		(!down_trylock(m))
#define mutex_unlock(m)			up(m)
#endif

/* Check for DEFINE_TIMER and TIMER_INITIALIZER. */
#include <linux/timer.h>

#ifndef TIMER_INITIALIZER
#define TIMER_INITIALIZER(_function, _expires, _data) {		\
		.function = (_function),			\
		.expires = (_expires),				\
		.data = (_data),				\
	}
#endif

#ifndef DEFINE_TIMER
#define DEFINE_TIMER(_name, _function, _expires, _data)		\
	struct timer_list _name =				\
		TIMER_INITIALIZER(_function, _expires, _data)
#endif


/* Check for ktime API. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)
#define KCOMPAT_HAVE_KTIME
#endif

#ifdef KCOMPAT_HAVE_KTIME
#include <linux/ktime.h>

/*
 * Prior to kernel version 2.6.20, ktime_to_ns(kt) may have returned a u64
 * value on 32-bit CPUs unless the CONFIG_KTIME_SCALAR switch is defined.
 * Later kernels always return an s64 value.
 *
 * Redefine it if necessary to always return an s64 value.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,20)
static inline s64 kcompat_ktime_to_ns(const ktime_t kt)
{
	return (s64)ktime_to_ns(kt);
}
#undef ktime_to_ns
#define ktime_to_ns(kt) kcompat_ktime_to_ns(kt)
#endif

#endif /* ifdef KCOMPAT_HAVE_KTIME */

/* Check for existence of hrtimer API and API changes. */
/*#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,16)) || defined(DEFINE_KTIME)*/
/* hrtimer API wasn't exported until 2.6.17 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17)
#define KCOMPAT_HAVE_HRTIMER		/* Have hrtimer API. */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17)) || defined(HRTIMER_INACTIVE)
#define KCOMPAT_HAVE_HRTIMER_SLEEPER	/* Have hrtimer_sleeper API. */

/* The usage of 'struct hrtimer' and its callback function changed in
 * 2.6.17.
 * Prior to 2.6.17, 'struct hrtimer' had a settable 'data' member which
 * was passed as the parameter to the callback function specified by the
 * 'function' member (i.e. ret = timer->function(timer->data);).
 * In 2.6.17, there is no 'data' member and the pointer to the actual
 * timer is passed in the callback function (i.e.
 * ret = timer->function(timer);).  The callback function is supposed to
 * use the container_of() macro to get a pointer to the containing structure.
 *
 * Define KCOMPAT_HRTIMER_FUNCTION_CONTAINER to indicate that the new version
 * should be used.
 */
#define KCOMPAT_HRTIMER_FUNCTION_CONTAINER

#else	/* !(LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17)) */

/* The prototype of hrtimer_forward() changed in 2.6.17.  The second parameter
 * was added to pass the current time.  Remap this function for older kernels
 * to ignore the second parameter.  The second parameter is normally set to
 * timer->base->get_time(). */
static inline unsigned long kcompat_hrtimer_forward(struct hrtimer *timer,
		ktime_t now, ktime_t interval)
{
	return hrtimer_forward(timer, interval);
}
#undef hrtimer_forward
#define hrtimer_forward(timer, now, interval) \
	kcompat_hrtimer_forward(timer, now, interval)

#endif	/* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17) */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)
/* Have the hrtimer_nanosleep_restart() function. */
#define KCOMPAT_HAVE_HRTIMER_NANOSLEEP_RESTART
#endif

/* Return type of timer expiry function changed in 2.6.21. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,21)
typedef enum hrtimer_restart kcompat_hrtimer_return_t;
#else
typedef int kcompat_hrtimer_return_t;
#endif

/* Timer mode constants renamed in 2.6.21 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21)
#define HRTIMER_MODE_ABS	HRTIMER_ABS
#define HRTIMER_MODE_REL	HRTIMER_REL
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,21) */

#endif /* LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,17) */


#include <asm/io.h>	/* Defines mmiowb() if it exists (2.6.10) */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)) && !defined(mmiowb)

/* Define missing mmiowb() */

#if defined(CONFIG_MIPS)

/* MIPS - this depends on MIPS II instruction set. */
#define mmiowb()	asm volatile ("sync" ::: "memory")

#elif defined(CONFIG_IA64)

/* IA64 - this depends on the platform. */

#ifdef CONFIG_IA64_SGI_SN2
/* SGI SN2 platform */
#include <asm/sn/io.h>
/* Note: this needs at least kernel 2.4.22. */
#define platform_mmiowb	sn_mmiob
#endif

#ifndef platform_mmiowb
/* Generic platform */
#define platform_mmiowb	__ia64_mf_a
#endif

#define mmiowb()	platform_mmiowb()

#else

/* Other architectures. */
#define mmiowb()	do {} while (0)
#endif

#endif

/*
 * mmiowb() was removed altogether in kernel 5.2, so define an empty version
 * if not already defined.
 */
#ifndef mmiowb
#define mmiowb()	do {} while (0)
#endif

#include <linux/string.h>

/* Define kstrdup() for kernels prior to 2.6.13. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,13)
static inline char *kcompat_kstrdup(const char *s, gfp_t gfp)
{
	size_t len;
	char *buf;

	if (!s)
		return NULL;

	len = strlen(s) + 1;
	buf = kmalloc(len, gfp);
	if (buf)
		memcpy(buf, s, len);
	return buf;
}
#undef kstrdup
#define kstrdup(s, gfp) kcompat_kstrdup(s, gfp)
#endif

/* Define kstrndup() for kernels prior to 2.6.23. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)
static inline char *kcompat_kstrndup(const char *s, size_t max, gfp_t gfp)
{
	size_t len;
	char *buf;

	if (!s)
		return NULL;

	len = strnlen(s, max);
	buf = kmalloc(len + 1, gfp);
	if (buf) {
		memcpy(buf, s, len);
		buf[len] = '\0';
	}
	return buf;
}
#undef kstrndup
#define kstrndup(s, max, gfp) kcompat_kstrndup(s, max, gfp)
#endif

/* Define kmemdup() for kernels prior to 2.6.19. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,19)
static inline void *kcompat_kmemdup(const void *src, size_t len, gfp_t gfp)
{
	void *p;

	p = kmalloc(len, gfp);
	if (p)
		memcpy(p, src, len);
	return p;
}
#undef kmemdup
#define kmemdup(src, len, gfp) kcompat_kmemdup(src, len, gfp)
#endif

/* Define spin_trylock_irqsave(), spin_trylock_irq(), and spin_trylock_bh()
 * if they are missing.  WARNING: These use GCC extensions! */

#ifndef spin_trylock_bh
#define spin_trylock_bh(lock) \
({ \
	local_bh_disable(); \
	spin_trylock(lock) ? 1 : ({local_bh_enable(); 0; }); \
})
#endif

#ifndef spin_trylock_irq
#define spin_trylock_irq(lock) \
({ \
	local_irq_disable(); \
	spin_trylock(lock) ? 1 : ({local_irq_enable(); 0; }); \
})
#endif

#ifndef spin_trylock_irqsave
#define spin_trylock_irqsave(lock, flags) \
({ \
	local_irq_save(flags); \
	spin_trylock(lock) ? 1 : ({local_irq_restore(flags); 0; }); \
})
#endif

/* Define KCOMPAT_HAVE_LINUX_FIRMWARE_H if kernel has <linux/firmware.h>. */
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,73)) || \
		((LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,23)) && \
		 (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))))
#define KCOMPAT_HAVE_LINUX_FIRMWARE_H
#endif

#ifdef KCOMPAT_HAVE_LINUX_FIRMWARE_H
#include <linux/firmware.h>

/* In kernel version 5.14, FW_ACTION_HOTPLUG and FW_ACTION_NOHOTPLUG changed
 * names to FW_ACTION_UEVENT and FW_ACTION_NOUEVENT, respectively.
 *
 * If FW_ACTION_HOTPLUG is defined, define FW_ACTION_UEVENT and
 * FW_ACTION_NOUEVENT to expand to the old names.
 */
#ifdef FW_ACTION_HOTPLUG
#define FW_ACTION_UEVENT FW_ACTION_HOTPLUG
#define FW_ACTION_NOUEVENT FW_ACTION_NOHOTPLUG
#endif

/* Define KCOMPAT_REQUEST_FIRMWARE_NOWAIT_HAS_UEVENT if
 * request_firmware_nowait() has an extra parameter (parameter 2) called
 * 'hotplug' or 'uevent' of type 'int' between the 'module' and firmware 'name'
 * parameters.  This is true for kernel 2.6.14 onwards, and is associated
 * with the #define values FW_ACTION_NOHOTPLUG and FW_ACTION_HOTPLUG added
 * in the same kernel version.  Note that for kernel version 2.6.39 onwards,
 * the 'uevent' parameter changed type from 'int' to 'bool'.
 *
 * In kernel version 5.14, FW_ACTION_HOTPLUG and FW_ACTION_NOHOTPLUG changed
 * names to FW_ACTION_UEVENT and FW_ACTION_NOUEVENT, respectively.
 *
 * We defined FW_ACTION_UEVENT (and FW_ACTION_NOUEVENT) if FW_ACTION_HOTPLUG
 * is defined, so test whether that macro is defined.
 */
#ifdef FW_ACTION_UEVENT
#define KCOMPAT_REQUEST_FIRMWARE_NOWAIT_HAS_UEVENT
#endif
/* KCOMPAT_REQUEST_FIRMWARE_NOWAIT_HAS_GFP may have been defined outside this
 * file for compatibility with some Red Hat 2.6.32 kernels. */
#ifndef KCOMPAT_REQUEST_FIRMWARE_NOWAIT_HAS_GFP
/* Define KCOMPAT_REQUEST_FIRMWARE_NOWAIT_HAS_GFP if request_firmware_nowait()
 * has an extra parameter (parameter 5) called 'gfp' of type 'gfp_t' between
 * the 'device' and 'context' parameters.  This is true for kernel 2.6.33
 * onwards.  */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,33)
#define KCOMPAT_REQUEST_FIRMWARE_NOWAIT_HAS_GFP
#endif
#endif
/* Define KCOMPAT_REQUEST_FIRMWARE_NOWAIT_OWNS_FIRMWARE if the caller of
 * request_firmware_nowait() is responsible for releasing the firmware
 * passed to the callback function using release_firmware().  Note that
 * if KCOMPAT_REQUEST_FIRMWARE_NOWAIT_OWNS_FIRMWARE is not defined, the
 * caller should assume the passed in firmware structure is invalid on
 * return from the callback function.
 *
 * NOTE: this should be defined for mainline kernel version 2.6.33 onwards
 * but Red Hat backported it to some of their 2.6.32 kernels.  This appears
 * to be the same kernels for which KCOMPAT_REQUEST_FIRMWARE_NOWAIT_HAS_GFP
 * is defined (for RHEL 6.1 onwards), so assume if that is set we should set
 * this too.  */
#ifdef KCOMPAT_REQUEST_FIRMWARE_NOWAIT_HAS_GFP
#define KCOMPAT_REQUEST_FIRMWARE_NOWAIT_OWNS_FIRMWARE
#endif
#else
/* Define dummy firmware support. */
struct firmware {
	size_t size;
	const u8 *data;
};

static inline int request_firmware(const struct firmware **fw,
		const char *name, const char *device)
{
	return -EINVAL;
}

static inline int request_firmware_nowait(struct module *module,
		const char *name, const char *device, void *context,
		void (*cont)(const struct firmware *fw, void *context))
{
	return -EINVAL;
}

static inline void release_firmware(const struct firmware *fw)
{
}
#endif

/* Define KCOMPAT_REQUEST_FIRMWARE_USES_DEVICE_STRING if need to specify the
 * "device" parameter as a string for request_firmware() and
 * request_firmware_nowait().  This is for 2.4 kernels. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,73)
#define KCOMPAT_REQUEST_FIRMWARE_USES_DEVICE_STRING
#endif

/* <linux/smp_lock.h> and the Big Kernel Lock were removed in 2.6.39. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,39)
#define KCOMPAT_HAVE_LINUX_SMP_LOCK_H
#endif

#include <asm/processor.h>	/* for cpu_relax() */

/*
 * cpu_relax() was introduced as a macro sometime around 2.4.11 but
 * changed to a inline function sometime in the 2.6 series.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
#ifndef cpu_relax
#define cpu_relax()	do {} while (0)
#endif
#endif

#include <linux/mm.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,0,0)
typedef unsigned long __nocast vm_flags_t;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(6,3,0) && !defined(VM_LOCKED_MASK)
static inline void vm_flags_init(struct vm_area_struct *vma, vm_flags_t flags)
{
	vma->vm_flags = flags;
}

static inline void vm_flags_reset(struct vm_area_struct *vma, vm_flags_t flags)
{
	vm_flags_init(vma, flags);
}

static inline void vm_flags_set(struct vm_area_struct *vma, vm_flags_t flags)
{
	vma->vm_flags |= flags;
}

static inline void vm_flags_clear(struct vm_area_struct *vma, vm_flags_t flags)
{
	vma->vm_flags &= ~flags;
}

static inline void __vm_flags_mod(struct vm_area_struct *vma,
				  vm_flags_t set, vm_flags_t clear)
{
	vm_flags_init(vma, (vma->vm_flags | set) & ~clear);
}

static inline void vm_flags_mod(struct vm_area_struct *vma,
				vm_flags_t set, vm_flags_t clear)
{
	__vm_flags_mod(vma, set, clear);
}
#endif	/* LINUX_VERSION_CODE < KERNEL_VERSION(6,3,0) && !defined(VM_LOCKED_MASK) */

#ifndef PG_locked
#include <linux/page-flags.h>
#endif

#ifndef offset_in_page
#define offset_in_page(p)	((unsigned long)(p) & ~PAGE_MASK)
#endif

#include <linux/pagemap.h>	/* for lock_page() */

/*
 * set_page_dirty_lock() was added in kernel 2.5.57.  Fake it before that.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,57)
static inline int set_page_dirty_lock(struct page *page)
{
	int ret;

	lock_page(page);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,12)
	set_page_dirty(page);
	ret = 1;	/* ought to be 0 if page was already marked dirty */
#else
	ret = set_page_dirty(page);
#endif
	unlock_page(page);
	return ret;
}
#endif

#ifndef page_private
#define page_private(page) ((page)->private)
#endif

/*
 * compound_head() added in Linux kernel version 2.6.22.  Before that,
 * page->private held the head of compound pages.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
static inline struct page *compound_head(struct page *page)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,60)
	if (unlikely(PageCompound(page))) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,6)
		page = (struct page *)page_private(page);
#else
		page = (struct page *)page->lru.next;
#endif	/* if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,6) */
	}
#endif	/* if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,60) */
	return page;
}
#endif	/* if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22) */

/* struct scatterlist stuff. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
#define KCOMPAT_HAVE_LINUX_SCATTERLIST_H
#endif

#ifdef KCOMPAT_HAVE_LINUX_SCATTERLIST_H
#include <linux/scatterlist.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,2,0)
#define KCOMPAT_HAVE_ASM_SCATTERLIST_H
#endif

#ifdef KCOMPAT_HAVE_ASM_SCATTERLIST_H
#include <asm/scatterlist.h>
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,15)
static inline void sg_set_buf(struct scatterlist *sg, void *buf,
		unsigned int buflen)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,13)
	sg->address = buf;
#else
	sg->page = virt_to_page(buf);
	sg->offset = offset_in_page(buf);
#endif
	sg->length = buflen;
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
static inline void sg_init_one(struct scatterlist *sg, void *buf,
		unsigned int buflen)
{
	memset(sg, 0, sizeof(*sg));
	sg_set_buf(sg, buf, buflen);
}
#endif

/* The for_each_sg macro and a bunch of inline functions was added in 2.6.24.
 * We can implement some of those functions, but not all of them.
 * In particular, chaining lists together and marking the end of the list are
 * not supported. */
#ifndef for_each_sg

/* Note: this version of sg_next() doesn't return NULL at the end! */
static inline struct scatterlist *sg_next(struct scatterlist *sg)
{
	return sg + 1;
}

static inline struct scatterlist *sg_last(struct scatterlist *sgl,
		unsigned int nents)
{
	return &sgl[nents - 1];
}

/* Note: this version of sg_init_table() doesn't mark the end of the list! */
static inline void sg_init_table(struct scatterlist *sgl, unsigned int nents)
{
	memset(sgl, 0, sizeof(*sgl) * nents);
}

#define for_each_sg(sglist, sg, nr, __i) \
	for (__i = 0, sg = (sglist); __i < (nr); __i++, sg = sg_next(sg))

static inline void sg_assign_page(struct scatterlist *sg, struct page *page)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,13)
	/* This won't work on HIGHMEM pages! */
	sg->address = page_address(page) + offset_in_page(sg->address);
#else
	sg->page = page;
#endif
}

static inline void sg_set_page(struct scatterlist *sg, struct page *page,
		unsigned int len, unsigned int offset)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,13)
	/* This won't work on HIGHMEM pages! */
	sg->address = page_address(page) + offset;
#else
	sg_assign_page(sg, page);
	sg->offset = offset;
#endif
	sg->length = len;
}

static inline struct page *sg_page(struct scatterlist *sg)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,13)
	return virt_to_page(sg->address);
#else
	return sg->page;
#endif
}

static inline dma_addr_t sg_phys(struct scatterlist *sg)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,13)
	return __pa(sg->address);
#else
	return page_to_phys(sg->page) + sg->offset;
#endif
}

static inline void *sg_virt(struct scatterlist *sg)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,4,13)
	return sg->address;
#else
	return page_address(sg_page(sg)) + sg->offset;
#endif
}

/*
 * Note: we aren't attempting to define sg_chain_ptr(), sg_is_chain(),
 * sg_is_last(), sg_mark_end() or sg_chain().  You can check if SG_MAGIC
 * is defined to determine whether they are available.
 */

#endif	/* ifndef for_each_sg */

/*
 * <linux/hash.h> stuff.
 */

/* <linux/hash.h> first appeared in kernel 2.5.7.
 * Define KCOMPAT_HAVE_LINUX_HASH_H if the header can be included.  */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,7)
#define KCOMPAT_HAVE_LINUX_HASH_H
#include <linux/hash.h>
#endif

#include <asm/types.h>

/* GOLDEN_RATIO_PRIME_32 and GOLDEN_RATIO_PRIME_64 were first defined in
 * kernel 2.6.25, along with the hash_32() and hash_64() inline functions.
 *
 * GOLDEN_RATIO_32 and GOLDEN_RATIO_64 were first defined in kernel 4.6,
 * and the hash_64() function was modified to use GOLDEN_RATIO_64.
 *
 * GOLDEN_RATIO_PRIME_32 and GOLDEN_RATIO_PRIME_64 were removed in
 * kernel 4.7, and the hash_32() function was modified to use GOLDEN_RATIO_32.
 */
#if !defined(GOLDEN_RATIO_PRIME_32) && !defined(GOLDEN_RATIO_32)
/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define GOLDEN_RATIO_PRIME_32 0x9e370001UL

static inline u32 hash_32(u32 val, unsigned int bits)
{
	/* On some cpus multiply is faster, on others gcc will do shifts */
	u32 hash = val * GOLDEN_RATIO_PRIME_32;

	/* High bits are more random, so use them. */
	return hash >> (32 - bits);
}

#endif
#if !defined(GOLDEN_RATIO_PRIME_64) && !defined(GOLDEN_RATIO_64)
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001UL

static inline u64 hash_64(u64 val, unsigned int bits)
{
	u64 hash = val;

	/*  Sigh, gcc can't optimise this alone like it does for 32 bits. */
	u64 n = hash;
	n <<= 18;
	hash -= n;
	n <<= 33;
	hash -= n;
	n <<= 3;
	hash += n;
	n <<= 3;
	hash -= n;
	n <<= 4;
	hash += n;
	n <<= 2;
	hash += n;

	/* High bits are more random, so use them. */
	return hash >> (64 - bits);
}
#endif

/* GOLDEN_RATIO_PRIME will have been defined by <linux/hash.h> if it has been
 * #include'd above, as will the hash_long() and hash_ptr() inline functions. */
#ifndef GOLDEN_RATIO_PRIME

#if BITS_PER_LONG == 32
#define GOLDEN_RATIO_PRIME GOLDEN_RATIO_PRIME_32
#define hash_long(val, bits) hash_32(val, bits)
#elif BITS_PER_LONG == 64
#define GOLDEN_RATIO_PRIME GOLDEN_RATIO_PRIME_64
#define hash_long(val, bits) hash_64(val, bits)
#else
#error Wordsize not 32 or 64
#endif

static inline unsigned long hash_ptr(const void *ptr, unsigned int bits)
{
	return hash_long((unsigned long)ptr, bits);
}

#endif	/* ifndef GOLDEN_RATIO_PRIME */

/* hash32_ptr() was added in kernel 3.7.0. */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,7,0)
static inline u32 hash32_ptr(const void *ptr)
{
	unsigned long val = (unsigned long)ptr;

#if BITS_PER_LONG == 64
	val ^= (val >> 32);
#endif
	return (u32)val;
}
#endif

/*
 * N.B. The buffer hashing functions arch_fast_hash() and arch_fast_hash2()
 * from kernel 3.14.0 onwards have not been defined here yet for earlier
 * kernels.
 */

/*
 * struct vm_operations_struct changes.
 */

/* Define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_POPULATE
 * if struct vm_operations_struct has the 'populate' handler.
 * This was added in 2.5.46 and removed in 2.6.23.  */
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,46)) && \
		(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,23)))
#define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_POPULATE
#endif

/* Define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_NOPAGE
 * if struct vm_operations_struct has the 'nopage' handler.
 * This was removed in 2.6.26.  */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
#define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_NOPAGE
/* Define KCOMPAT_VM_OPERATIONS_STRUCT_NOPAGE_HAS_TYPE if the final
 * parameter of the 'nopage' handler is an 'int *' used to return the type
 * of fault (e.g. VM_FAULT_MINOR).  This is true for kernel 2.6.1 onwards
 * (until the nopage handler was removed in 2.6.26).  Before 2.6.1 the
 * parameter was an 'int' called 'unused'.  */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,1)
#define KCOMPAT_VM_OPERATIONS_STRUCT_NOPAGE_HAS_TYPE
#endif
#endif

/* (Don't care about 'set_policy' and 'get_policy' handlers added in 2.6.7
 * or the 'migrate' handler added in 2.6.18 as they only exist if
 * CONFIG_NUMA is defined.)  */

/* Define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_PAGE_MKWRITE
 * if struct vm_operations_struct has the 'page_mkwrite' handler.
 * This was added in 2.6.18.  */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
#define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_PAGE_MKWRITE
#endif

/* Define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_NOPFN
 * if struct vm_operations_struct has the 'nopfn' handler.
 * This was added in 2.6.19 and removed in 2.6.27.  */
#if ((LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,19)) && \
		(LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)))
#define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_NOPFN
#endif

/* Define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_FAULT
 * if struct vm_operations_struct has the 'fault' handler.
 * This was added in 2.6.23 at the same time as 'struct vm_fault' was
 * added and the FAULT_FLAG_WRITE and FAULT_FLAG_NONLLINEAR macros were
 * defined.  */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,23)
#define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_FAULT
#endif

/* Define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_ACCESS
 * if struct vm_operations_struct has the 'access' handler.
 * This was added in 2.6.27.  */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define KCOMPAT_HAVE_VM_OPERATIONS_STRUCT_ACCESS
#endif

/*
 * For kernel versions prior to 2.6.28, drivers that use the fasync file
 * operation need to call the fasync file operation in their release
 * file operation with fd set to -1 if the FASYNC flag is set in
 * file->f_flags (and can optionally do it FASYNC flag isn't set).
 *
 * Calling it during release might cause problems for some recent
 * kernels, so define KCOMPAT_NEED_FASYNC_ON_RELEASE for kernel versions
 * prior to 2.6.28 so drivers can compile the code in conditionally.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,28)
#define KCOMPAT_NEED_FASYNC_ON_RELEASE
#endif

/*
 * get_user_pages() was added in kernel 2.5.2 and backported to kernel 2.4.17.
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,17) && \
     LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0)) || \
    LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,2)
#define KCOMPAT_HAVE_GET_USER_PAGES
#endif

#ifdef KCOMPAT_HAVE_GET_USER_PAGES
/*
 * get_user_pages_fast() was added in kernel 2.6.27.  Fake it before that
 * if we have get_user_pages().
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27)
static inline int get_user_pages_fast(unsigned long start, int nr_pages,
				      int write, struct page **pages)
{
	int rc;

	down_read(&current->mm->mmap_sem);
	rc = get_user_pages(current, current->mm, start, nr_pages, write, 0,
			    pages, NULL);
	up_read(&current->mm->mmap_sem);
	return rc;
}
#endif

/*
 * __get_user_pages_fast() was added in kernel 2.6.31.
 * Don't know how to emulate it for earlier kernels.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,31)
#define KCOMPAT_HAVE___GET_USER_PAGES_FAST
#endif

/*
 * __get_user_pages() was added in kernel 2.6.39.
 * Don't know how to emulate it for earlier kernels.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,39)
#define KCOMPAT_HAVE___GET_USER_PAGES
#endif

/*
 * TODO: Deal with changes to parameters of get_user_pages()
 * in kernel version 4.9.
 */

/*
 * In kernel version 5.2, the third parameter of get_user_pages_fast() changed
 * from `int write` to `unsigned int gup_flags`.  Common values passed are
 * `0` for read-only mappings or `FOLL_WRITE` for read-write mappings.
 *
 * `FOLL_WRITE` was first defined by <linux/mm.h> in kernel version 2.6.15.
 * Define it if missing.
 */
#ifndef FOLL_WRITE
#define FOLL_WRITE	0x01
#endif

/*
 * Kernel version 5.6 introduced pin_user_pages(), pin_user_pages_fast(),
 * etc., unpin_user_page(), unpin_user_pages(), and
 * unpin_user_pages_dirty_lock().  The 'FOLL_PIN' macro was defined at the
 * same time, so check for that when checking for backports.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,6,0) && !defined(FOLL_PIN)
/*
 * For now, only define pin_user_pages_fast().
 *
 * TODO: define pin_user_pages() and perhaps others.
 */

static inline long pin_user_pages_fast(unsigned long start, int nr_pages,
				       unsigned int gup_flags,
				       struct page **pages)
{
	return get_user_pages_fast(start, nr_pages, gup_flags, pages);
}

static inline void unpin_user_page(struct page *page)
{
	put_page(page);
}

static inline void unpin_user_pages(struct page **pages, unsigned long npages)
{
	unsigned long index;

	for (index = 0; index < npages; index++)
		unpin_user_page(pages[index]);
}

static inline void unpin_user_pages_dirty_lock(struct page **pages,
					       unsigned long npages,
					       bool make_dirty)
{
	unsigned long index;

	if (!make_dirty) {
		unpin_user_pages(pages, npages);
		return;
	}

	for (index = 0; index < npages; index++) {
		struct page *page = compound_head(pages[index]);

		if (!PageDirty(page))
			set_page_dirty_lock(page);
		unpin_user_page(page);
	}
}

#endif	/* if LINUX_VERSION_CODE<KERNEL_VERSION(5,6,0) && !defined(FOLL_PIN) */

#endif	/* ifdef KCOMPAT_HAVE_GET_USER_PAGES */

/*
 * In <linux/highmem.h>, single argument forms of kmap_atomic() and
 * kunmap_atomic() were added in kernel 2.6.37, although the two argument
 * forms continued to work up to and including kernel 3.5.
 *
 * Define kcompat_kmap_atomic() and kcompat_kunmap_atomic() to take two
 * arguments and call kmap_atomic() and kunmap_atomic() with the appropriate
 * number of arguments.  Need to define these as macros as the typical values
 * for the second argument were removed in kernel 3.6.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,37)
#define kcompat_kmap_atomic(page, type)		kmap_atomic(page, type)
#define kcompat_kunmap_atomic(addr, type)	kunmap_atomic(addr, type)
#else
#define kcompat_kmap_atomic(page, type)		kmap_atomic(page)
#define kcompat_kunmap_atomic(addr, type)	kunmap_atomic(addr)
#endif

/*
 * <linux/of_reserved_mem.h> was added in kernel version 3.15.
 * It allows contiguous, reserved memory regions to be defined in the
 * device tree.
 *
 * The of_reserved_mem_device_init() and of_reserved_mem_device_release()
 * functions were not added until kernel version 3.17.
 *
 * From kernel version 3.18 onwards, of_reserved_mem_device_init() returns
 * an 'int' rather than void.
 */

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,15,0)
#define KCOMPAT_HAVE_LINUX_OF_RESERVED_MEM_H
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,17,0)
#define KCOMPAT_HAVE_OF_RESERVED_MEM_DEVICE_INIT
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,18,0)
#define KCOMPAT_OF_RESERVED_MEM_DEVICE_INIT_RETURNS_INT
#endif

/*
 * dma_mmap_coherent() is generally present (though not necessarily functional)
 * in kernel version 3.6 onwards.  But ARM and PowerPC have had it since at
 * least kernel version 3.0 (and probably before), which is all we care about
 * here.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0) || \
	defined(CONFIG_ARM) || defined(CONFIG_PPC)
#define KCOMPAT_HAVE_DMA_MMAP_COHERENT
#endif

/* <linux/uaccess.h> was added in kernel version 2.6.18, and should be
 * included in preference to <asm/uaccess.h>.  In particular, copy_to_user()
 * and copy_from_user() were moved to <linux/uaccess.h> in kernel version
 * 4.12. */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,18)
#define KCOMPAT_HAVE_LINUX_UACCESS_H
#endif

#ifdef KCOMPAT_HAVE_LINUX_UACCESS_H
#include <linux/uaccess.h>
#else
#include <asm/uaccess.h>
#endif
/*
 * Kernel 5.0 removed VERIFY_READ and VERIFY_WRITE and removed the first
 * parameter of access_ok() which was set to VERIFY_READ or VERIFY_WRITE.
 * That has been redundant since kernel 2.5.70, and even then it was only
 * checked for kernels that support old 386 processors.
 *
 * access_ok() may be used internally by macros in <asm/uaccess.h>, so we
 * cannot just redefine it.
 *
 * Define kcompat_access_ok() to be used insread of access_ok().  For
 * kernels prior to 5.0, this will call access_ok() with 3 parameters, and
 * will always pass VERIFY_WRITE as the first parameter.  This will fail
 * for old 386 processors if the memory region is not in fact writable.
 * For kernels 5.0 and later, it willcall access_ok() with 2 parameters.
 */
#ifdef VERIFY_WRITE
/* Pre 5.0 kernel. */
static inline int kcompat_access_ok(const void __user *addr, size_t size)
{
	/* Always use VERIFY_WRITE.  Most architectures ignore it. */
	return access_ok(VERIFY_WRITE, addr, size);
}
#else
static inline int kcompat_access_ok(const void __user *addr, size_t size)
{
	return access_ok(addr, size);
}
#endif

/*
 * Simplified atomic barriers introduced in kernel version 3.16.
 */

#ifndef smp_mb__before_atomic
#define smp_mb__before_atomic()	smp_mb()
#endif

#ifndef smp_mb__after_atomic
#define smp_mb__after_atomic()	smp_mb()
#endif

/*
 * The constants and return values for the 'poll' file operation changed in
 * kernel version 4.16, although for most architectures the constants have
 * the same numeric values.
 *
 * Define KCOMPAT_EPOLLIN etc. to be used instead of POLLIN for 'poll' file
 * operations, and define the 'kcompat_poll_t' return type for 'poll'
 * file operations.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,16,0)
#define KCOMPAT_EPOLLIN		EPOLLIN
#define KCOMPAT_EPOLLPRI	EPOLLPRI
#define KCOMPAT_EPOLLOUT	EPOLLOUT
#define KCOMPAT_EPOLLERR	EPOLLERR
#define KCOMPAT_EPOLLHUP	EPOLLHUP
#define KCOMPAT_EPOLLNVAL	EPOLLNVAL
#define KCOMPAT_EPOLLRDNORM	EPOLLRDNORM
#define KCOMPAT_EPOLLRDBAND	EPOLLRDBAND
#define KCOMPAT_EPOLLWRNORM	EPOLLWRNORM
#define KCOMPAT_EPOLLWRBAND	EPOLLWRBAND
#define KCOMPAT_EPOLLMSG	EPOLLMSG
#define KCOMPAT_EPOLLRDHUP	EPOLLRDHUP
typedef __poll_t kcompat_poll_t;
#else
#define KCOMPAT_EPOLLIN		POLLIN
#define KCOMPAT_EPOLLPRI	POLLPRI
#define KCOMPAT_EPOLLOUT	POLLOUT
#define KCOMPAT_EPOLLERR	POLLERR
#define KCOMPAT_EPOLLHUP	POLLHUP
#define KCOMPAT_EPOLLNVAL	POLLNVAL
#define KCOMPAT_EPOLLRDNORM	POLLRDNORM
#define KCOMPAT_EPOLLRDBAND	POLLRDBAND
#define KCOMPAT_EPOLLWRNORM	POLLWRNORM
#define KCOMPAT_EPOLLWRBAND	POLLWRBAND
#define KCOMPAT_EPOLLMSG	POLLMSG
#define KCOMPAT_EPOLLRDHUP	POLLRDHUP
typedef unsigned int kcompat_poll_t;
#endif

/*
 * dma_pool functions available from kernel version 2.6.3 onwards.
 * Prior to that, the pci_pool functions need to be used (for PCI devices).
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,3)
#define KCOMPAT_HAVE_LINUX_DMAPOOL_H
#include <linux/dmapool.h>
#endif

/*
 * DMA API available from kernel verison 2.5.53 onwards.
 */
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,53)
#define KCOMPAT_HAVE_LINUX_DMA_MAPPING_H
#endif

#ifdef KCOMPAT_HAVE_LINUX_DMA_MAPPING_H
#include <linux/dma-mapping.h>

/*
 * Redefine dma_alloc_coherent() to zero the allocated memory for earlier
 * kernel versions like it does for kernel version 5.0.
 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,2,0)

static inline void *kcompat_dma_alloc_coherent(struct device *dev, size_t size,
					       dma_addr_t *dma_handle,
					       gfp_t flags)
{
	void *ret = dma_alloc_coherent(dev, size, dma_handle, flags);
	if (ret)
		memset(ret, 0, size);
	return ret;
}
#undef dma_alloc_coherent
#define dma_alloc_coherent kcompat_dma_alloc_coherent

#elif LINUX_VERSION_CODE < KERNEL_VERSION(5,0,0)

#undef dma_alloc_coherent
#define dma_alloc_coherent dma_zalloc_coherent

#endif
#endif

#endif	/* KCOMPAT_H__INCLUDED */
