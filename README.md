PLX PCI905x Serial EEPROM Driver
================================

COPYRIGHT AND LICENSE
---------------------

PLX PCI905x Serial EEPROM Driver.
Written by Ian Abbott @ MEV Limited <abbotti@mev.co.uk>.
Copyright (C) 2002, 2019 MEV Limited.

   > MEV Limited                                                  <br>
   > Building 67                                                  <br>
   > Europa Business Park                                         <br>
   > Bird Hall Lane                                               <br>
   > STOCKPORT                                                    <br>
   > SK3 0XA                                                      <br>
   > UNITED KINGDOM                                               <br>

   > Tel: +44 (0)161 477 1898                                     <br>
   > WWW: <https://www.mev.co.uk/>                                <br>

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
675 Mass Ave, Cambridge, MA 02139, USA.

A copy of the GNU General Public License may be found in the file
"COPYING".

As the copyright holder, MEV Limited reserves the right to re-use
(either directly or by license to third parties) those parts of the
program written by MEV Limited or its employees in other programs not
covered by the GNU General Public License.

Please note that PLX Technology, Inc. have no connection to this driver.


INTRODUCTION
------------

This is a Linux driver kernel module that provides read/write random
access to the serial EEPROM of various PLX PCI interface chips.

The driver currently supports the PCI9030, PCI9050, PCI9052 and PCI9054
(revision A or later) chips.  The driver may also support the PCI9056,
PCI9060, PCI9080 and PCI9656, but this is so far untested.

For the PCI9050 and PCI9052, the serial EEPROM is presented as a file
128 bytes long.

For the PCI9030, PCI9054, PCI9056 and PCI9656, the serial EEPROM is
presented as a file of length 256 or 512 bytes, depending on a module
parameter.  The default is 256 bytes.  If the module parameter indicates
that the file should be 512 bytes long, but a 2048-bit (256-byte) serial
EEPROM is fitted, the upper 256 bytes will alias the lower 256 bytes.

For the PCI9060 and PCI9080, the serial EEPROM is presented as a file of
length 128 or 256 bytes, depending on a module parameter.  There is no
default value, as the operations required for each size of serial EEPROM
is different in this case and the size cannot be determined
programmatically.

Each 16-bit word of the serial EEPROM appears as two bytes in
little-endian order; even offsets address the least significant byte of
the 16-bit word and odd offsets address the most significant byte.
Reads and writes may start on an even or odd offset and the number of
bytes transferred may be even or odd.

The driver only supports a single device at a time.  Module parameters
are used to select the device on driver load.


USAGE
-----

### Building

#### Prerequisites

A normal development environment is required to build the driver kernel
modules and other software.  Depending on the Linux distribution, a set
of properly configured Linux kernel sources may need to be installed to
match the running kernel.  Some distributions such as Fedora and Debian
include enough of the kernel sources in the binary kernel package to
allow external modules to be built.  Other distributions may require
more work to set up a suitable environment for building kernel modules.

The driver may be built for kernel 2.6.0 or later.  (It may also build
for kernel 2.4.x.)  The kernel configuration
must support loadable modules and must support the PCI bus.

#### Build configuration

Before the software is built, the sources need to be configured by
running the `./configure` shell script.  This has a number of
command-line options, mostly for tuning the installation locations, but
also for controlling which kernel to build the modules for and to set
the default device major number.

To get a list of configuration options, run:

    ./configure --help

To configure the software, run:

    ./configure [option...]

where `[option...]` is the optional configuration command-line options
described below.

Currently, the only file installed by the package is the kernel
modules.  The kernel
module is installed in the `/lib/modules/${kernelrelease}/extra`
directory by default (where `${kernelrelease}` depends on which kernel
the modules are being built for).

The kernel modules need to be built against a specific kernel build
directory, usually the kernel build directory for the currently running
kernel.  Depending on how the kernel was built, the kernel build
directory may also be the kernel source directory.  By default, the
configure script will try a number of possible locations for the kernel
build directory, but will try a user-specified kernel build directory
first, if specified.  The following configuration option specifies a
kernel build directory:

* `--with-kerneldir=DIR`

> Specifies the first kernel build directory to try.  This sets
  `${kernelrelease}`.

The configure script tries possible kernel build locations in the
following order:

* directory specified using `--with-kerneldir=DIR`
* ``/lib/modules/`uname -r`/build``
* ``/usr/src/kernel-source-`uname -r` ``
* ``/usr/src/linux-`uname -r` ``
* `/usr/src/linux`

The first location above that exists is assumed to be a valid kernel
build directory.  Note that `` `uname -r` `` expands to the kernel
release string for the currently running kernel.

Note that the kernel build directory must be properly configured for the
kernel that the modules are to be built for, otherwise the modules will
either fail to compile or fail to load.

The default installation location for the kernel modules is
`/lib/modules/${kernelrelease}/extra`, where `${kernelrelease}` is
determined from the the files at the kernel build directory.  The
following configuration option specifies a different installation
location for the kernel modules:

* `--with-moduledir=DIR`

> Sets the location to install the kernel modules to `DIR`
  \[`/lib/modules/${kernelrelease}/extra`\]

The driver needs to be assigned a device major number.  It can be told
to use a specific device major number or can be told to use a
dynamically assigned device major number when it is loaded.  If the
driver is told to use device major number 0, it will use a dynamically
assigned device major number instead.  The following configuration
option can be used to specify a default device major number:

* `--with-major=value`

> Specify the device major number for the driver \[0\]

Whatever device major number is chosen during configuration can be
overridden when the module is loaded using a module parameter `major` so
it is okay to leave the device major number set to 0 during
configuration even if you intend the driver to use a specific device
major number.

Device major numbers from 240 to 254 are reserved for local use.
Dynamically assigned device major numbers are assigned from 254
downwards, so if you wish to use a specific device major number, 240 is
a good one to choose unless it is already in use.  Check the
`/proc/devices` file (in the "Character devices" section) to see which
device major numbers are currently in use.  To find a good unused device
major number, start at 240 and work upwards.

#### Building

Once the sources have been configured, the software can be built using
the following command:

    make

Running make in the top-level source directory will build everything.
Running make in a subdirectory will build everything in that
subdirectory and below.  This is often used to rebuild the driver for a
different kernel without having to rebuild everything else.

### Installation

NOTE: Installation is optional.  The driver module built as
`driver/plx905x.ko` may be loaded without installation.

After building, the software can be installed by running the following
command as a super-user:

    su    # run this to become super-user if necessary.
    make install

#### Reinstallation Following A Kernel Update

The kernel modules are built and installed for a specific kernel
determined when the software is configured.  If a new kernel is
installed, it is necessary to rebuild and reinstall the kernel modules
for that kernel.  To do this, it is first necessary to enter the driver
subdirectory and run make clean, then change back to the top-level
directory:

    cd driver
    make clean
    cd ..

Alternatively, from the top-level directory, this does the same thing as
the above sequence:

    make -C driver clean

It is then necessary to reconfigure the sources for the new kernel
version.  Use the same configuration command-line options as before,
except for the `--with-kerneldir=DIR` option, if used.

A shortcut to run `./configure` with *exactly* the same command-line
options as previously used is to run:

    ./config.status --recheck

Running `./config-status --recheck` only applies if the
`--with-kerneldir` option was not used previously (otherwise the
software will be configured for the old kernel build directory) and if
the new kernel build directory is the first location that will be found
by the configure script (typically the ``/lib/modules/`uname -r`/build``
directory).  Otherwise, it is necessary to run `./configure` with the
correct options.

After reconfiguring the software with `./configure` or `./config.status
--recheck`, the driver kernel modules can be rebuilt by entering the
driver subdirectory and running `make`:

    cd driver
    make clean   # if not done previously
    make

After rebuilding the driver for the new kernel, the kernel modules can
be installed for the new kernel by running `make install` as a
super-user in the driver directory:

    # already in driver subdirectory
    su    # run this to become super-user if necessary.
    make install

### Preparing to load

Next become root and load the module with any optional parameters
required (see below).  If any other drivers are using the PCI card, it
may be necessary to unload them first using `rmmod` if the driver is a
kernel module.  (If the driver is not a kernel module, you may need to
rebuild the kernel without support for the PCI card in question.)

Before loading the module, identify the PCI card using the `lspci`
command.  The left hand column shows the positions of installed PCI
devices in 'domain:bus:slot.func' form, where 'domain' is four hex digits,
'bus' is two hex digits, 'slot'
is two hex digits in the range 00 to 1f, and 'func' is one hex digit in
the range 0 to 7.  The driver does not support multi-function PCI
devices, so the 'func' number will only be 0 for PCI devices of interest
to the driver.  The remaining columns identify the device using text
strings.  If `lspci -n` is used, the remaining columns will identify the
device numerically by class and a 'vendor:device' pair, where 'vendor'
is the PCI vendor ID and 'device' is the PCI device ID, both expressed
as four hex digits.  More detailed information including PCI subsystem
ID information can be obtained by running `lspci -v` or `lspci -vn`.
The latter version will show the subsystem vendor ID and device ID in
hex.

### Loading

If the module has not been installed by `make install`, the module that
has been built as `driver/plx905x.ko` can be loaded by a super user
using the `insmod` command as follows:

    cd driver
    insmod plx905x.ko [param=value] ...

where `[param=value] ...` is the start-up parameters for the module used
to select the PCI device to be used and set the major device number for
the driver.

If the module has been installed by `make install`, the module can be
loaded by a super user using the `modprobe` command:

    modprobe plx905x [param=value] ...

The following parameters may be used.  Values specified on the command
line after the `param=` will be treated as hex if they begin with `0x`,
octal if they begin with `0` or decimal if they begin with `1` through
`9`.  Typically, PCI IDs are specified in hex.  Bus and slot numbers may
be specified in hex to match the bus and slot numbers shown by `lspci`
or may be specified in decimal.

* `major=n` -- This sets the major device number of the driver.  The default
  value is 0 which causes the major device number to be assigned
  dynamically.

* `bus=n` -- This selects the PCI bus number of the device, range 0 to 255.
  The default is 0.

* `slot=n` -- This selects the PCI slot number of the device, range 0 to 31.
  The default is 0.

* `vendor=n` --  This selects the PCI vendor ID of the device.  The default is
  -1, which means "any" PCI vendor ID, but see below.

* `device=n` -- This selects the PCI device ID of the device.  The default is
  -1, which means "any" PCI device ID, but see below.

* `subvendor=n` --  This selects the PCI subsystem vendor ID of the device.
  The default is -1, which means "any" PCI subsystem vendor ID,
  but see below.

* `subdevice=n` -- This selects the PCI subsystem device ID of the device.
  The default is -1, which means "any" PCI subsystem device ID,
  but see below.

* `instance=n` -- This selects the nth matching PCI device matching the
  `vendor`, `device`, `subvendor` and `subdevice` parameters,
   counting from 0.  The default is 0.  This parameter is ignored
   when the `bus` or `slot` parameter has been set to a non-zero
   value.

* `eeprom=n` -- This specifies the size of serial EEPROM fitted.  The values
   `46`, `128` or `1024` specify a 1024-bit (128-byte) serial
   EEPROM.  The values `56`, `256` or `2048` specify a 2048-bit
   (256-byte) serial EEPROM.  The values `66`, `512` or `4096`
   specify a 4096-bit (512-byte) serial EEPROM.  The default
   depends on the PLX model of the target PCI device.  For the PLX
   PCI9050 and PCI9052, the default is 1024 bits (128 bytes) and
   this is the only size allowed.  For the PLX PCI9030, PCI9054,
   PCI9056 and PCI9656, the default is 2048 bits (256 bytes), but a
   size of 4096 bits (512 bytes) is also allowed.  For the PLX
   PCI9060 and PCI9080, the size must be specified as 1024 bits
   (128 bytes) or 2048 bits (256 bytes) and there is no default.

* `plx=n` -- This specifies the PLX chip type.  The default value is `0`,
	which causes the driver to attempt to guess the chip type.  For
	the PCI9030, `plx` may be set to `0x9030`, `9030` or `0`.  For
	the PCI9050 and PCI9052, `plx` may be set to `0x9050`, `9050`,
	`0x9052`, `9052` or `0`; the PCI9050 and PCI9052 are equivalent
	as far as this driver is concerned.  For the PCI9054, `plx` may
	be set to `0x9054`, `9054` or `0`.  For the PCI9056, `plx` may
	be set to `0x9056`, `9056` or `0`.  For the PCI9060, `plx` may
	be set to `0x9060`, `9060` or `0`.  For the PCI9080, `plx` may
	be set to `0x9080`, `9080` or `0`.  For the PCI9656, `plx` may
	be set to `0x9656`, `9656` or `0`.

If the `bus` or `slot` parameters are non-zero, the PCI device at the
specified location is matched against the `vendor`, `device`,
`subvendor` and `subdevice` parameters if they are set to their
non-default values.  In this case, the `vendor` or `device` parameter's
default value loses its special meaning of "any" if the other parameter
has a non-default parameter.  This is also true for the `subvendor` and
`subdevice` parameters.  The `instance` parameter is ignored in this
case.

If the `bus` and `slot` parameters are both set to the default value of
`0`, the entire list of installed PCI devices is searched for a match.
However, note that in this case, if `vendor` and `device` are both set
to the default value of `-1`, then `vendor` is changed to `0x10b5` and
`device` is changed to a value that depends on the `plx` parameter to
match the factory default PCI vendor and device ID for the specified PLX
chip type.  This defaults to `0x9050`, which is the factory default PCI
device ID for the PCI9050 or PCI9052.  The `subvendor` and `subdevice`
parameters remain unaltered.

The module will fail to load if a matching device cannot be found at the
specified localtion (if specified).  It will also fail to load if the
device does not appear to be supported or if there is a resource
conflict with another driver using the PCI device.  The check for a
supported device is more robust for the PCI9054, PCI9056, PCI9060,
PCI9080 and PCI9656 than for the PCI9030, PCI9050 and PCI9052, but is
not absolutely reliable.  Be careful with those parameters!

The module outputs a kernel message on loading that indicates which PCI
device is being used (if any), a reason why the module could not be
loaded (if any) and the major device number assigned to the driver (if
successfully loaded).  Recent kernel messages may be examined using the
`dmesg` command.


### Using

#### Creating the device file

A 'character special' file with the correct major device number is
required.  The driver currently ignores the minor device number but it
is suggested that the special file has this set to 0.

On modern systems with a dynamic `/dev` directory, the special file 
`/dev/plx905x` is created automatically and the remainder of this section
may be skipped.  On systems with a static `/dev`
directory, it needs to be created manually using the `mknod` command as
described below.

##### Creating the device file manually

If the major device number was set dynamically by loading the module
without setting a value for the `major` parameter or by setting it to `0`,
the major device number can be determined by examining the
`/proc/devices` file or by checking the kernel message output by the
module when it was loaded (for example by using the `dmesg` command).

Once the major device number is known, the character special file may be
created with the `mknod` command.  In this example a character special
file named `/dev/plx905x` is created with major device number 254 and
minor device number 0:

    mknod /dev/plx905x c 254 0

#### Using the device file

Byte offsets in the file map onto byte offsets in the serial EEPROM.
The serial EEPROM has 16-bit words.  The least significant byte of each
16-bit word has a byte offset evenly divisible by 2.  In other words,
the 16-bit words are mapped onto byte offsets in little-endian order.

The file supports `open`, `close`, `read`, `write` and `lseek` operations.
Attempts to seek outside the confines of the address space (128, 256 or
512 bytes, depending on the EEPROM size specified or defaulted to) are
errored with `EINVAL`.  Attempts to write starting at the end of the
address space are errored with `ENOSPC`.  An end-of-file condition is
returned when attempting to read from the end of the address space.
Problems physically reading or writing the EEPROM are errored with `EIO`.


### Examples

The entire address space of the serial EEPROM may be read and redirected
to a file using the `cat` command and shell redirection:

    cat /dev/plx905x > dump.bin

The `cat` command and shell redirection may be used to rewrite the
entire serial EEPROM using the contents of a regular file:

    cat dump.bin > /dev/plx905x

The `dd` command may be used to selectively read or write parts of the
serial EEPROM.  It's probably easiest to set the block size to 1
(`bs=1`), `count` to the number of bytes to be read and written, `seek`
to the byte offset within the output file to start writing, and `skip`
to the byte offset within the input file to start reading.  See the
__dd(1)__ manpage for details (`man 1 dd`).


### Unloading

After use, the module may be unloaded from the kernel using `rmmod`:

    rmmod plx905x

Changes to the EEPROM will not affect the PCI card until the system is
rebooted.
