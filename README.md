# Linux-evocation

Linux-evocation is a conversion of the historic 0.99.12 linux kernel so that it compiles under gcc 5, cross compiles under MacOS and runs with qemu. Why this kernel? This was the one used by the first linux distribution, [Slackware 1.01](http://mirrors.slackware.com/slackware/slackware-1.01/) back in 1993. 

# Prerequisites
The kernel can be built using both these OSSes:
- Ubuntu 16.04 xenial xerus 32 bit
- MacOS Sierra

The following was used to compile the kernel:
- gcc 5.4
- binutils 2.26.1
- make 4.1
- as86 and ld86 0.16.17

Testing was done with qemu i386 2.8.0 on MacOS Sierra using the slackware 1.01 disk image from the [2014 QEMU Advent Calendar](http://www.qemu-advent-calendar.org/2014/). 

A Vagrantfile has been included that sets up Ubuntu using the `ubuntu/xenial32` base image. Just cd to the linux directory in the ubuntu home dir.

# Compilation
```sh
make config
make dep
make
```
The Makefile detects if you are using MacOS and if so, sets the `CROSS_COMPILE` variable to `i386-elf-`.

To cleanup
```sh
# Wipe build, but keep config
make clean

# Wipe everything, including config
make mrproper
```

# Running it
## Just booting the kernel
To see the kernel boot, run it like this:
```sh
qemu-system-i386 -m 8 -kernel zImage -curses
```
The `-curses` parameter makes qemu use the curses library to emulate a VGA screen. Press esc-2-q to exit.

## Running it with a slackware's 1.01 disk image
The kernel isn't particularly useful without a root filesystem, so to see something more realistic, use the QEMU slackware 1.01 disk image.

Download and extract the slackware 1.01 [disk image](http://www.qemu-advent-calendar.org/2014/download/qemu-xmas-slackware.tar.xz) from the [2014 QEMU Advent Calendar](http://www.qemu-advent-calendar.org/2014/) with 
```sh
wget http://www.qemu-advent-calendar.org/2014/download/qemu-xmas-slackware.tar.xz
tar xfv qemu-xmas-slackware.tar.xz
unxz qemu-xmas-slackware/slackware.qcow2.xz
mkdir images
mv qemu-xmas-slackware/slackware.qcow2 images/
rm -Rf qemu-xmas-slackware qemu-xmas-slackware.tar.xz
```

The kernel needs to have a major/minor device for the root device written into it. This is done by creating a floppy image and booting off of that.

First create an empty floppy
```sh
# Create an empty floppy
dd bs=512 count=2880 if=/dev/zero of=floppy
```
Then copy the kernel to the floppy and set the root to `/dev/hda1`.
```sh
dd bs=512 if=zImage of=floppy conv=notrunc
near the end of the boot sector.
echo -ne "\x01\x03" | dd ibs=1 obs=1 count=2 seek=508 of=floppy conv=notrunc
```

Run it with the slackware 1.01 using `-curses`
```sh
qemu-system-i386 -m 8 -curses -drive if=floppy,format=raw,file=floppy -drive if=ide,format=qcow2,file=images/slackware.qcow2
```

The results:
```
...
Mounting remote file systems...
INET: syslogd inetd
May  7 14:16:54 init[1]: Entering runlevel: 6

Welcome to Linux 0.99pl12.

slack login: root
May  7 14:16:56 login[32]: no shadow password for `root' on `tty1'

Linux 0.99pl12. (Posix).
No mail.
May  7 14:16:56 login[32]: ROOT LOGIN ON TTY `tty1'

slack:/# uname -a
Linux slack 0.99.12 #6 Sun Aug 8 16:02:35 CDT 1993 i586
```

## Running it with networking
Add the following to the qemu command line to enable [SLIRP networking](http://wiki.qemu-project.org/Documentation/Networking#User_Networking_.28SLIRP.29):
```sh
-device ne2k_isa,netdev=slirp -netdev user,id=slirp
```

Then run this inside slackware:
```sh
/etc/ifconfig eth0 10.0.2.15 netmask 255.255.255.0 broadcast 10.0.2.255 up
/etc/route add 10.0.2.0 eth0
/etc/route add default gw 10.0.2.2
```

That results in:
```sh
slack:/# ping 10.0.2.2 -c 1
PING 10.0.2.2 (10.0.2.2): 56 data bytes
64 bytes from 10.0.2.2: icmp_seq=0 ttl=185 time=3 ms

--- 10.0.2.2 ping statistics ---
1 packets transmitted, 1 packets received, 0% packet loss
round-trip min/avg/max = 3/3/3 ms
```

# Changes
- `sys_idle()` uses the `hlt` instruction for better performance under qemu.
- `panic()` stops on a `cld` & `hlt` instead of an infinite loop, also so that the cpu doesn't go to 100% on qemu if there is a problem.
- Search for a ramdisk filesystem on more blocks on the floppy, to allow for using the 1 MB image from [linux-0.11-lab](https://github.com/tinyclub/linux-0.11-lab).
- Adapted ramdisk code to also look for an ext2 filesystem. 
- Reduced `calibrate_delay()` loop time from 100 ticks to 10 ticks for faster booting.
- Added `/* wait for "start of" clock tick */` from a later kernel to `calibrate_delay()`.
- Fixed spurious keyboard "7"s when running under qemu and moving the mouse, or detaching from the UI and then pressing a key.

# Compiler Warnings
Although `-Wall` is switched on, a considerable amount of warnings have been ignored by using `-Wno-*` parameters to gcc. I preferred having clean output and didn't want to spend too much time fixing warnings. I'll leave it as an exercise to a motivated reader :) to fix them.

# Why
The main purpose was to learn about how the linux kernel works by reading, compiling and running the code. Inspired by [Mariuz's Blog](http://mapopa.blogspot.co.uk/2008/09/linux-0.html), I decided to make whatever changes were needed to compile the kernel with a modern toolchain.

# Known Issues
- It's got Y2k bugs
- The keyboard fix for the qemu "7"s (see commit f39c093) isn't perfect. There must be a better way of doing this.
- There's something not right about the networking code. qemu sometimes goes to 100% cpu usage after pinging 10.0.2.2 for a while
- Pings sometimes come back with negative times