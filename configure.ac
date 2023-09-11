dnl $Id$

dnl Process this file with autoconf to produce a configure script.
dnl configure.ac, (C) 2015-2023 MEV Ltd.

AC_PREREQ([2.69])
AC_INIT([PLX PCI905x Serial EEPROM Driver],[2.1.2],[abbotti@mev.co.uk],[plx905x-eeprom])
AM_INIT_AUTOMAKE([foreign])
AM_MAINTAINER_MODE([enable])
AC_CONFIG_HEADERS([plx905x-eeprom-config.h])

dnl Set driver version from package version.
plx905x_eeprom_version_major=[`\
        expr $PACKAGE_VERSION : '\([0-9][0-9]*\)\.' '|' 0 + 0
        ` ]
plx905x_eeprom_version_minor=[`
        expr $PACKAGE_VERSION : '[0-9][0-9]*\.\([0-9][0-9]*\)' '|' 0 + 0
        ` ]
plx905x_eeprom_version_micro=[`
        expr $PACKAGE_VERSION : '[0-9][0-9]*\.[0-9][0-9]*\.\([0-9][0-9]*\)' '|' 0 + 0
        ` ]
plx905x_eeprom_version_nano=[`
        expr $PACKAGE_VERSION : '[0-9][0-9]*\.[0-9][0-9]*\.[0-9][0-9]*\.\([0-9][0-9]*\)' '|' 0 + 0
        ` ]
plx905x_eeprom_driver_name=[plx905x]
plx905x_eeprom_class_name=[plx905x]

dnl Set device prefixes for PLX905x EEPROM devices
plx905x_eeprom_device_prefix=[plx905x]

dnl Checks for programs.
AC_PROG_CC
AC_PATH_PROG(depmod, depmod, /sbin/depmod, $PATH:/sbin)

dnl Checks for libraries.

dnl Checks for header files.

dnl Checks for typedefs, structures, and compiler characteristics.

dnl Checks for library functions.

dnl Handle arguments

dnl Don't create nor destroy devices when installing and uninstalling
AC_ARG_ENABLE([sandboxed],
  [AS_HELP_STRING([--enable-sandboxed],
    [do not touch anything out of the install directory])],
  [enable_sandboxed="${enableval}"],
  [enable_sandboxed="no"])
AM_CONDITIONAL([SANDBOXED], [test "$enable_sandboxed" = "yes"])

AC_ARG_WITH([moduledir],
  [AS_HELP_STRING([--with-moduledir=DIR],
    [kernel modules in DIR [/lib/modules/KERNELRELEASE/extra]])],
  [moduledir="${withval}"],
  [moduledir="default"])

AC_ARG_WITH([major],
  [AS_HELP_STRING([--with-major=value],
    [specify the device major for the driver [0]])],
  [plx905x_major="${withval}"],
  [plx905x_major=0])

dnl Do kernel checks

dnl look for Linux kernel build directory
AC_PATH_KERNEL_BUILD

if test "$no_kernel" = "yes"; then
  AC_MSG_ERROR([kernel directory missing!])
fi

dnl look for Linux kernel source directory
AC_PATH_KERNEL_SOURCE

dnl check for (hopefully) supported kernel versions
dnl support 2.4.17 <= ver < 2.5, 2.6.0 <= ver  (for get_user_pages()).
AC_CHECK_KERNEL_VERSION([2.6],
			[support_kernel="maybe"], [support_kernel="yes"],
			[support_kernel="yes"])
if test "$support_kernel" = "maybe"; then
  AC_CHECK_KERNEL_VERSION([2.4],
			  [support_kernel="no"], [support_kernel="yes"],
			  [support_kernel="no"])
fi
if test "$support_kernel" != "yes"; then
  AC_MSG_ERROR([kernel version not supported!])
fi

dnl check kernel configuration options
AC_CHECK_LINUX_CONFIG_OPTION([CONFIG_PCI],
			     [have_pci="yes"], [have_pci="yes"],
			     [have_pci="no"])
if test "$have_pci" != "yes"; then
  AC_MSG_ERROR([kernel has no PCI support!])
fi

AC_LINUX_CHECK_HAVE_COMPLETION_DONE(
  [AC_DEFINE([KCOMPAT_HAVE_COMPLETION_DONE], [1],
	     [Define to 1 if completion_done() exists])
  ], [])

AC_LINUX_CHECK_HAVE_TRY_WAIT_FOR_COMPLETION(
  [AC_DEFINE([KCOMPAT_HAVE_TRY_WAIT_FOR_COMPLETION], [1],
	     [Define to 1 if try_wait_for_completion() exists])
  ], [])

dnl If moduledir is default, set to the actual default
if test "$moduledir" = "default"; then
  moduledir="/lib/modules/${kernelrelease}/extra"
fi

dnl tell the Makefiles what we decided
AC_SUBST(moduledir)

dnl tell the C code what we decided
AC_DEFINE_UNQUOTED(
  PLX905X_EEPROM_DRIVER_NAME, "${plx905x_eeprom_driver_name}",
  [Specify name of PLX PCI905x EEPROM driver])
AC_DEFINE_UNQUOTED(
  PLX905X_EEPROM_CLASS_NAME, "${plx905x_eeprom_class_name}",
  [Specify name of plx905x class])
AC_DEFINE_UNQUOTED(
  PLX905X_EEPROM_DRIVER_VERSION_MAJOR, ${plx905x_eeprom_version_major},
  [Specify major part of plx905x driver version])
AC_DEFINE_UNQUOTED(
  PLX905X_EEPROM_DRIVER_VERSION_MINOR, ${plx905x_eeprom_version_minor},
  [Specify minor part of plx905x driver version])
AC_DEFINE_UNQUOTED(
  PLX905X_EEPROM_DRIVER_VERSION_MICRO, ${plx905x_eeprom_version_micro},
  [Specify micro part of plx905x driver version])
AC_DEFINE_UNQUOTED(
  PLX905X_EEPROM_DRIVER_VERSION_NANO, ${plx905x_eeprom_version_nano},
  [Specify nano part of plx905x driver version])
AC_DEFINE_UNQUOTED(
  PLX905X_EEPROM_DEVICE_PREFIX, "${plx905x_eeprom_device_prefix}",
  [Specify prefix of plx905x device names])
AC_DEFINE_UNQUOTED(PLX905X_MAJOR, ${plx905x_major},
  [Specify the device major for the driver (0 to assign dynamically)])


AC_CONFIG_FILES([
  Makefile
  dkms.conf
  driver/Makefile
])
AC_OUTPUT
