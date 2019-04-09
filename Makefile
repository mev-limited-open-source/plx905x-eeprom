#
# PLX PCI905x serial EEPROM driver.
#
# Written by Ian Abbott @ MEV Ltd. <ian.abbott@mev.co.uk>
# Copyright (C) 2002 MEV Limited.
# 
#     MEV Ltd.
#     Suite 8 Baxall Business Centre
#     Adswood Road Industrial Estate
#     Stockport
#     Cheshire
#     SK3 8LF
#     UNITED KINGDOM
# 
#     Tel: +44 (0)161 477 1898
#     Fax: +44 (0)161 718 3587
#     WWW: http://www.mev.co.uk/
#
# Most of this Makefile was written by Linus Torvalds, Alessandro Rubini,
# et al.
# 
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2 of the License, or (at your
# option) any later version.
# 
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
# 
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 675 Mass Ave, Cambridge, MA 02139, USA.
# 
# A copy of the GNU General Public License may be found in the file
# "COPYING".
#

#DEBUG = y
#DEBUGGER = y

TOPDIR = .
ifndef KERNELDIR
	KERNELDIR := $(shell echo "/lib/modules/`uname -r`/build")
endif
INCLUDEDIR = $(KERNELDIR)/include

ifeq ($(KERNELDIR)/.config,$(wildcard $(KERNELDIR),.config))
	include $(KERNELDIR)/.config
else
	MESSAGE := $(shell echo "WARNING: no .config file in $(KERNELDIR)")
endif

ifndef ARCH
	ARCH := $(shell uname -m | sed -e s/i.86/i386/ -e s/sun4u/sparc64/ \
		-e s/arm.*/arm/ -e s/sa110/arm/)
endif

AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
CPP	= $(CC) -E
AR	= $(CROSS_COMPILE)ar
NM	= $(CROSS_COMPILE)nm
STRIP	= $(CROSS_COMPILE)strip
OBJCOPY	= $(CROSS_COMPILE)objcopy
OBJDUMP	= $(CROSS_COMPILE)objdump

ARCHMAKEFILE = Makefile.$(ARCH)
ifeq ($(ARCHMAKEFILE),$(wildcard $(ARCHMAKEFILE)))
	include $(ARCHMAKEFILE)
endif

CFLAGS += -Wall -D__KERNEL__ -DMODULE -I$(INCLUDEDIR)

ifdef CONFIG_SMP
	CFLAGS += -D__SMP__ -DSMP
endif

ifdef CONFIG_MODVERSIONS
	CFLAGS += -DMODVERSIONS -include $(KERNELDIR)/include/linux/modversions.h
endif

VERSIONFILE = $(INCLUDEDIR)/linux/version.h
VERSION = $(shell awk -F\" '/REL/ {print $$2}' $(VERSIONFILE))
INSTALLDIR = /lib/modules/$(VERSION)/misc

ifeq ($(DEBUGGER),y)
	DEBFLAGS += -O -g
else
	DEBFLAGS += -O2
endif
ifeq ($(DEBUG),y)
	DEBFLAGS += -DPLX9050_DEBUG
endif

CFLAGS += $(DEBFLAGS)

TARGET = plx905x
OBJS = $(TARGET).o
SRC = plx905x_main.c

all: .depend $(TARGET).o

$(TARGET).o: $(SRC:.c=.o)
	$(LD) -r $^ -o $@

install:
	install -d $(INSTALLDIR)
	install -c $(TARGET).o $(INSTALLDIR)

clean:
	rm -f *.o *~ core .depend

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > $@

ifeq (.depend,$(wildcard .depend))
include .depend
endif

