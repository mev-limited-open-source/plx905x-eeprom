## $Id$
##
## additional m4 macros
##
## (C) 1999 Christoph Bartelmus (lirc@bartelmus.de)
##
## Modified by Ian Abbott, MEV Ltd. (abbotti@mev.co.uk) to include
## /lib/modules/`uname -r`/build in the kernel source search.  Also
## fixed underquoted definitions detected by aclocal 1.9.  Also added
## 'kernelrelease'.
##
## Original version from lirc-0.7.0.
##
## 2006-04-18 Ian Abbott: changed mechanism for determining 'kernelrelease',
## 'version' and 'patchlevel' and removed 'kernelcc'.  This is to cope with
## separate kernel source and kernel build directories better (as used by SuSE
## Linux for example).
##
## Renamed AC_PATH_KERNEL_SOURCE to AC_PATH_KERNEL_BUILD and
## AC_PATH_KERNEL_SOURCE_SEARCH to AC_PATH_KERNEL_BUILD_SEARCH
##
## 2006-07-13 Ian Abbott: changed mechanism for determining 'kernelrelease',
## 'version' and 'patchlevel'.  Uses the 'UTS_RELEASE' macro from
## "${kerneldir}/include/linux/utsrelease.h" if it exists (2.6.18), falling
## back to "${kerneldir}/include/linux/version.h" otherwise.
##
## 2009-12-18 Ian Abbott: changed mechanism for determining 'kernelrelease',
## 'version' and 'patchlevel'.  Uses the 'UTS_RELEASE' macro from
## "${kerneldir}/include/generated/utsrelease.h" if it exists (2.6.33),
## falling back to "${kerneldir}/include/linux/utsrelease.h" if it exists
## (2.6.18), falling back to "${kerneldir}/include/linux/version.h" otherwise.
##
## 2011-02-03 Ian Abbott: Add 'AC_LINUX_CHECK_LINUX_CONFIG_OPTION' macro.
##
## 2011-02-03 Ian Abbott: Renamed 'version' to 'kv_major'.  Renamed
## 'patchlevel' to 'kv_minor'.  Added 'kv_micro'.  Together, these form the
## first three numeric components of the kernel version.  Changed the
## mechanism for extracting these components from 'kernelrelease' to use
## expr.
##
## 2011-02-03 Ian Abbott: Added 'AC_CHECK_KERNEL_VERSION' macro to compare
## kernel version to a specified version and take action depending whether
## kernel version is below, same as, or above specified version.
##
## 2013-02-21 Ian Abbott: Updated 'AC_PATH_KERNEL_BUILD_SEARCH' to also look
## for "version.h" in "${kerneldir}/include/generated/uapi/linux/version.h"
## as it moved in kernel 3.7.
##
## 2013-10-10 Ian Abbott: Added 'AC_PATH_KERNEL_SOURCE' which determines the
## kernel source directory and sets "${kernelsrcdir}" using the "${kerneldir}"
## variable determined earlier by 'AC_PATH_KERNEL_BUILD'.  Note that the kernel
## source directory may be the same as the kernel build directory and may have
## been stripped down to just the include directories necessary for building
## external kernel modules.  Also defines 'AC_TRY_LINUX_SRC_DIR' for internal
## use.
##
## 2013-10-10 Ian Abbott: Added 'AC_LINUX_CHECK_REQUEST_FIRMWARE_NOWAIT_HAS_GFP'
## to check if request_firmware_nowait() in <linux/firmware.h> has the 'gfp'
## parameter.  This check is needed for some Red Hat 2.6.32 kernels (RHEL 6.1
## onwards) due to including API changes from 2.6.33 that cannot be detected
## while building.
##
## 2016-10-20 Ian Abbott: Changed 'AC_PATH_KERNEL_BUILD_SEARCH' to run '$CPP'
## on a 'conftest.c' file instead of feeding standard input to '/lib/cpp'
## because '/lib/cpp' might not exist, and added dependency on 'AC_PROG_CPP'.
## Changed 'AC_PATH_KERNEL_BUILD' to expand 'AC_PATH_KERNEL_BUILD' at most
## once, possibly using the cached value in 'ac_cv_have_kernel'.  Also added
## dependency on 'AC_PROG_CPP' to work around a problem.
##
## 2018-03-20 Ian Abbott: Added
## 'AC_LINUX_CHECK_HAVE_COMPLETION_DONE' to check if <linux/completion.h>
## declares the 'completion_done()' function.  Also added
## 'AC_LINUX_CHECK_HAVE_TRY_WAIT_FOR_COMPLETION' to check if
## <linux/completion.h> declares the 'try_wait_for_completion()' function.
## These checks are needed for some Red Hat 2.6.18 kernels due to the functions
## being back-ported from the 2.6.27 by Red Hat.
##
## 2019-01-03 Ian Abbott: Updated 'AC_PATH_KERNEL_SOURCE' for kernel version
## 4.20 as the test for separate Linux source and build directory broke.
##


dnl check for kernel build directory (may or may not be kernel source directory)

AC_DEFUN([AC_PATH_KERNEL_BUILD_SEARCH],
[
  AC_REQUIRE([AC_PROG_CPP])
  kerneldir=missing
  kernelext=ko
  no_kernel=yes
  kernelrelease=missing
  kv_major=0
  kv_minor=0
  kv_micro=0

  if test "`uname`" != "Linux"; then
    kerneldir="not running Linux"
  else
    for dir in "${ac_kerneldir}" "/lib/modules/`uname -r`/build" \
        "/usr/src/kernel-source-`uname -r`" "/usr/src/linux-`uname -r`" /usr/src/linux; do
      if test -d "$dir"; then
        kerneldir="`dirname \"$dir/Makefile\"`"/
        no_kernel=no
        break
      fi;
    done
  fi

  if test x${no_kernel} != xyes; then
    if test ! -f "${kerneldir}/Makefile"; then
      kerneldir="no Makefile found"
      no_kernel=yes
    elif test ! -f "${kerneldir}/include/linux/version.h" -a \
              ! -f "${kerneldir}/include/generated/uapi/linux/version.h"; then
      kerneldir="no version.h found"
      no_kernel=yes
    else
      if test -f "${kerneldir}/include/generated/utsrelease.h"; then
        utsverfile="generated/utsrelease.h"
      elif test -f "${kerneldir}/include/linux/utsrelease.h"; then
        utsverfile="linux/utsrelease.h"
      else
        utsverfile="linux/version.h"
      fi
      echo UTS_RELEASE | cat "${kerneldir}/include/${utsverfile}" - > conftest.c
      kernelrelease=`$CPP -I "${kerneldir}/include" conftest.c \
        | tail -n 1 | tr -d '" '`
      kv_major=`expr "${kernelrelease}" : '\(@<:@@<:@:digit:@:>@@:>@\+\)'`
      kv_minor=`expr "${kernelrelease}" : '@<:@@<:@:digit:@:>@@:>@\+\.\(@<:@@<:@:digit:@:>@@:>@\+\)'`
      kv_micro=`expr "${kernelrelease}" : '@<:@@<:@:digit:@:>@@:>@\+\.@<:@@<:@:digit:@:>@@:>@\+\.\(@<:@@<:@:digit:@:>@@:>@\+\)'`
      : ${kv_major:=0} ${kv_minor:=0} ${kv_micro:=0}
      if test ${kv_major} -le 2 -a ${kv_minor} -lt 5; then
        kernelext=o
      fi
      rm -f ${ac_pkss_makefile}
    fi
  fi
  ac_cv_have_kernel="no_kernel=${no_kernel} \
                kerneldir=\"${kerneldir}\" \
                kernelext=\"${kernelext}\" \
                kernelrelease=\"${kernelrelease}\""
]
)

AC_DEFUN([AC_PATH_KERNEL_BUILD],
[
  AC_REQUIRE([AC_PROG_CPP])
  AC_MSG_CHECKING(for Linux kernel build directory)

  AC_ARG_WITH(kerneldir,
    [  --with-kerneldir=DIR    kernel build in DIR (sets KERNELRELEASE)], 
    [ac_kerneldir=${withval}],
    [ac_kerneldir=""]
  )
  
  AC_CACHE_VAL(ac_cv_have_kernel,AC_PATH_KERNEL_BUILD_SEARCH)

  eval "$ac_cv_have_kernel"

  AC_SUBST(kerneldir)
  AC_SUBST(kernelext)
  AC_SUBST(kernelrelease)
  AC_MSG_RESULT(${kerneldir})
]
)

dnl check if the given candidate path for a linux source tree is usable
AC_DEFUN([AC_TRY_LINUX_SRC_DIR],
	[AC_MSG_CHECKING(for Linux source in $1)

	if test -f "$1/Makefile" ; then
		result=yes
		$2
	else
		result="not found"
		$3
	fi

	AC_MSG_RESULT($result)
])

dnl get the kernel source directory
dnl Uses ${kerneldir} set by AC_PATH_KERNEL_BUILD_SEARCH() earlier.
dnl Sets ${kernelsrcdir}.
dnl
dnl Note: Callers should only rely on the 'include' directory being available
dnl in the kernel source directory as it may be stripped down to the basics
dnl for building external kernel modules.

AC_DEFUN([AC_PATH_KERNEL_SOURCE],
[
	AC_REQUIRE([AC_PATH_KERNEL_BUILD])
	kernelsrcdir=missing
	AC_ARG_WITH([linuxsrcdir],
		[AC_HELP_STRING([--with-linuxsrcdir=DIR],
			[specify path to Linux source directory])],
		[kernelsrcdir="${withval}"],
		[kernelsrcdir=default])

	if test "${kernelsrcdir}" != "default" ; then
		AC_TRY_LINUX_SRC_DIR([${kernelsrcdir}], ,
		AC_MSG_ERROR([Linux source dir not found]) )
	fi

	if test "${kernelsrcdir}" = "default" ; then
		AC_MSG_CHECKING(for separate Linux source and build directory)
		dir=`sed -n -e 's/^KERNELSRC *:= *\(.*\)/\1/p' "${kerneldir}/Makefile"`
		if test -z "$dir"; then
			# 2.6.25
			dir=`sed -n -e 's/^MAKEARGS *:= *-C *\([[^[:space:]]]*\).*/\1/p' "${kerneldir}/Makefile"`
		fi
		if test -z "$dir"; then
			# 4.20
			dir=`sed -n -e '/^__sub-make:$/,/^$/s/.* -C *\([[^[:space:]]]*\).*/\1/p' "${kerneldir}/Makefile"`
		fi
		if test -z "$dir"; then
			AC_MSG_RESULT([no])
			kernelsrcdir="${kerneldir}"
		else
			AC_MSG_RESULT([yes])
			case "$dir" in
			.*) dir="${kerneldir}/$dir" ;;
			esac
			AC_TRY_LINUX_SRC_DIR([${dir}], [kernelsrcdir=${dir}], )
		fi
	fi

	if test "${kernelsrcdir}" = "default" ; then
		AC_MSG_ERROR([Linux source directory not found])
	fi

	AC_SUBST(kernelsrcdir)
])

dnl Check the kernel .config file for option $1.
dnl Do $2 if the config option is set to 'y' (yes).
dnl Do $3 if the config option is set to 'm' (module).
dnl Do $4 if the config option is neither of the above (no).
dnl Uses ${kerneldir} set by AC_PATH_KERNEL_BUILD_SEARCH() earlier.

AC_DEFUN([AC_CHECK_LINUX_CONFIG_OPTION],
[
  AC_MSG_CHECKING([Linux config option $1])
  result=`sed -n 's/^$1=\(y\|m\)$/\1/p' ${kerneldir}/.config 2>/dev/null`
  case $result in
  y)
    result=yes
    $2
    ;;
  m)
    result=module
    $3
    ;;
  *)
    result=no
    $4
    ;;
  esac
  AC_MSG_RESULT([$result])
]
)

dnl Check the kernel version against $1.
dnl Do $2 if kernel version below $1.
dnl Do $3 if kernel version matches $1.
dnl Do $4 if kernel version above $1.
dnl Uses ${kv_major}, ${kv_minor}, ${kv_micro} set by
dnl AC_PATH_KERNEL_BUILD_SEARCH() earlier.

AC_DEFUN([AC_CHECK_KERNEL_VERSION],
[
  AC_MSG_CHECKING([Kernel version against $1])
  a=`expr "$1" : '\(@<:@@<:@:digit:@:>@@:>@\+\)'`
  b=`expr "$1" : '@<:@@<:@:digit:@:>@@:>@\+\.\(@<:@@<:@:digit:@:>@@:>@\+\)'`
  c=`expr "$1" : '@<:@@<:@:digit:@:>@@:>@\+\.@<:@@<:@:digit:@:>@@:>@\+\.\(@<:@@<:@:digit:@:>@@:>@\+\)'`
  if test "$a"; then
    if test $kv_major -gt $a; then
      result=above
    elif test $kv_major -lt $a; then
      result=below
    else
      if test "$b"; then
        if test $kv_minor -gt $b; then
          result=above
        elif test $kv_minor -lt $b; then
          result=below
        else
          if test "$c"; then
            if test $kv_micro -gt $c; then
              result=above
            elif test $kv_micro -lt $c; then
              result=below
            else
              result=same
            fi
          else
            result=same
          fi
        fi
      else
        result=same
      fi
    fi
  else
    result=same
  fi
  case $result in
  below)
    $2
    ;;
  same)
    $3
    ;;
  above)
    $4
    ;;
  esac
  AC_MSG_RESULT([$result])
]
)

dnl Check if the kernel's request_firmware_nowait() function has the gfp
dnl parameter.  This was added in mainline kernel 2.6.33 but Red Hat
dnl back-ported it to their 2.6.32-131 kernel (RHEL 6.1).
dnl
dnl AC_LINUX_CHECK_REQUEST_FIRMWARE_NOWAIT_HAS_GFP([ACTION-IF-FOUND],
dnl                                                [ACTION-IF-NOT-FOUND])
dnl
dnl Uses ${kernelsrcdir}.
AC_DEFUN([AC_LINUX_CHECK_REQUEST_FIRMWARE_NOWAIT_HAS_GFP],
[
	AC_REQUIRE([AC_PROG_EGREP])
	AC_REQUIRE([AC_PATH_KERNEL_SOURCE])
	AC_MSG_CHECKING([${kernelsrcdir} for gfp parameter in request_firmware_nowait()])
	$EGREP -q gfp_t "${kernelsrcdir}/include/linux/firmware.h"
	if (($?)); then
		AC_MSG_RESULT([no])
		$2
	else
		AC_MSG_RESULT([yes])
		$1
	fi
])

dnl Check if the completion_done() function exists.  This was added in mainline
dnl kernel 2.6.27 but Red Hat back-ported it to some of their 2.6.18 kernels
dnl (RHEL 5.x).
dnl
dnl AC_LINUX_CHECK_HAVE_COMPLETION_DONE([ACTION-IF-FOUND],
dnl                                     [ACTION-IF-NOT-FOUND])
dnl
dnl Uses ${kernelsrcdir}.
AC_DEFUN([AC_LINUX_CHECK_HAVE_COMPLETION_DONE],
[
	AC_REQUIRE([AC_PROG_EGREP])
	AC_REQUIRE([AC_PATH_KERNEL_SOURCE])
	AC_MSG_CHECKING([${kernelsrcdir} for completion_done()])
	$EGREP -q 'completion_done\(' "${kernelsrcdir}/include/linux/completion.h"
	if (($?)); then
		AC_MSG_RESULT([no])
		$2
	else
		AC_MSG_RESULT([yes])
		$1
	fi
])

dnl Check if the try_wait_for_completion() function exists.  This was added
dnl in mainline kernel 2.6.27 but Red Hat back-ported it to some of their
dnl 2.6.18 kernels (RHEL 5.x).
dnl
dnl AC_LINUX_CHECK_HAVE_TRY_WAIT_FOR_COMPLETION([ACTION-IF-FOUND],
dnl                                             [ACTION-IF-NOT-FOUND])
dnl
dnl Uses ${kernelsrcdir}.
AC_DEFUN([AC_LINUX_CHECK_HAVE_TRY_WAIT_FOR_COMPLETION],
[
	AC_REQUIRE([AC_PROG_EGREP])
	AC_REQUIRE([AC_PATH_KERNEL_SOURCE])
	AC_MSG_CHECKING([${kernelsrcdir} for try_wait_for_completion()])
	$EGREP -q 'try_wait_for_completion\(' "${kernelsrcdir}/include/linux/completion.h"
	if (($?)); then
		AC_MSG_RESULT([no])
		$2
	else
		AC_MSG_RESULT([yes])
		$1
	fi
])
