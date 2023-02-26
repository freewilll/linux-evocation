
all:	Version zImage

.EXPORT_ALL_VARIABLES:

CONFIG_SHELL := $(shell if [ -x "$$BASH" ]; then echo $$BASH; \
	  else if [ -x /bin/bash ]; then echo /bin/bash; \
	  else echo sh; fi ; fi)

SHELL = /bin/bash

OS := $(shell uname)

#
# Make "config" the default target if there is no configuration file or
# "depend" the target if there is no top-level dependency information.
#
ifeq (.config,$(wildcard .config))
include .config
ifeq (.depend,$(wildcard .depend))
include .depend
else
CONFIGURATION = depend
endif
else
CONFIGURATION = config
endif

ifdef CONFIGURATION
CONFIGURE = dummy
endif

# Ported from later kernel, we need include paths
TOPDIR := $(shell if [ "$$PWD" != "" ]; then echo $$PWD; else pwd; fi)

HPATH = $(TOPDIR)/include

#
# ROOT_DEV specifies the default root-device when making the image.
# This can be either FLOPPY, CURRENT, /dev/xxxx or empty, in which case
# the default of FLOPPY is used by 'build'.
#

ROOT_DEV = CURRENT

#
# If you want to preset the SVGA mode, uncomment the next line and
# set SVGA_MODE to whatever number you want.
# Set it to -DSVGA_MODE=NORMAL_VGA if you just want the EGA/VGA mode.
# The number is the same as you would ordinarily press at bootup.
#

# Mode 3 doesn't exist in qemu, so use mode 1: 80x28
SVGA_MODE=	-DSVGA_MODE=1

# Special options.
#OPTS	= -pro

#
# standard CFLAGS
#

WARNING_CFLAGS=\
	-Wno-comment \
	-Wno-char-subscripts \
	-Wno-strict-aliasing \
	-Wno-endif-labels \
	-Wno-parentheses \
	-Wno-unused-variable \
	-Wno-sign-compare \
	-Wno-return-type \
	-Wno-unused-but-set-variable \
	-Wno-narrowing \
	-Wno-write-strings \
	-Wno-pointer-arith \
	-Wno-format \
	-Wno-endif-labels \
	-Wno-overflow \
	-Wno-maybe-uninitialized
CFLAGS= -Wall $(WARNING_CFLAGS) -O3 -fomit-frame-pointer -x c++ -fno-stack-protector -fno-tree-loop-distribute-patterns -fno-reorder-blocks-and-partition

ifdef CONFIG_M486
CFLAGS := $(CFLAGS) -march=i486
endif

#
# if you want the ram-disk device, define this to be the
# size in blocks.
#

RAMDISK ?= -DRAMDISK=1024

AS86	=as86 -0 -a
LD86	=ld86 -0

ifeq ($(OS), Darwin)
CROSS_COMPILE 	?=x86_64-elf-
endif

AS		=$(CROSS_COMPILE)as
LD		=$(CROSS_COMPILE)ld -m elf_i386
HOSTCC		=gcc -I$(HPATH)
CC		=$(CROSS_COMPILE)gcc -D__KERNEL__ -I$(HPATH) -m32
MAKE		=make
CPP		=$(CC) -E -Wno-extra-tokens
HOSTCPP		=$(HOSTCC) -E -Wno-extra-tokens
AR		=$(CROSS_COMPILE)ar
STRIP		=$(CROSS_COMPILE)strip
OBJCOPY		=$(CROSS_COMPILE)objcopy -O binary -R .note -R .comment -S

ARCHIVES	=kernel/kernel.o mm/mm.o fs/fs.o net/net.o ipc/ipc.o
FILESYSTEMS	=fs/filesystems.a
NETWORK		=net/network.a
DRIVERS		=kernel/blk_drv/blk_drv.a kernel/chr_drv/chr_drv.a \
		 kernel/blk_drv/scsi/scsi.a kernel/chr_drv/sound/sound.a \
		 ibcs/ibcs.o
MATH		=kernel/FPU-emu/math.a
LIBS		=lib/lib.a
SUBDIRS		=kernel mm fs net ipc ibcs lib

KERNELHDRS	=/usr/src/linux/include

.c.s:
	$(CC) $(CFLAGS) -S -o $*.s $<
.ss.o:
	$(AS) -c -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

Version: dummy
	rm -f tools/version.h

config:
	$(CONFIG_SHELL) Configure $(OPTS) < config.in
	mv .config~ .config
	$(MAKE) soundconf

soundconf:
	cd kernel/chr_drv/sound;$(MAKE) config

linuxsubdirs: dummy
	@for i in $(SUBDIRS); do (cd $$i && echo $$i && $(MAKE)) || exit; done

.PHONY: tools/./version.h
tools/./version.h: tools/version.h

.PHONY: tools/version.h
tools/version.h: $(CONFIGURE) Makefile
	@./makever.sh
	@echo \#define UTS_RELEASE \"0.99.12\" > tools/version.h
	@echo \#define UTS_VERSION \"\#`cat .version` `date`\" >> tools/version.h
	@echo \#define LINUX_COMPILE_TIME \"`date +%T`\" >> tools/version.h
	@echo \#define LINUX_COMPILE_BY \"`whoami`\" >> tools/version.h
	@echo \#define LINUX_COMPILE_HOST \"`hostname`\" >> tools/version.h
	@echo \#define LINUX_COMPILE_DOMAIN \"`domainname`\" >> tools/version.h

tools/build: $(CONFIGURE) tools/build.c
	$(HOSTCC) -o tools/build tools/build.c

boot/head.o: $(CONFIGURE) boot/head.ss

boot/head.ss: $(CONFIGURE) boot/head.S include/linux/tasks.h
	$(CPP) -traditional boot/head.S -o boot/head.ss

tools/version.o: tools/version.c tools/version.h

init/main.o: $(CONFIGURE) init/main.c
	$(CC) $(CFLAGS) $(PROFILING) -c -o $*.o $<

tools/system:	boot/head.o init/main.o tools/version.o linuxsubdirs
	$(LD) $(LDFLAGS) -M boot/head.o init/main.o tools/version.o \
		$(ARCHIVES) \
		$(FILESYSTEMS) \
		$(NETWORK) \
		$(DRIVERS) \
		$(MATH) \
		$(LIBS) \
		-o tools/system > System.map

boot/setup: boot/setup.ss
	$(AS86) -o boot/setup.o boot/setup.ss
	$(LD86) -s -o boot/setup boot/setup.o

boot/setup.ss: $(CONFIGURE) boot/setup.S include/linux/config.h Makefile
	$(CPP) -traditional $(SVGA_MODE) $(RAMDISK) boot/setup.S -o boot/setup.ss

boot/bootsect.ss: $(CONFIGURE) boot/bootsect.S include/linux/config.h Makefile
	$(CPP) -traditional $(SVGA_MODE) $(RAMDISK) boot/bootsect.S -o boot/bootsect.ss

boot/bootsect:	boot/bootsect.ss
	$(AS86) -o boot/bootsect.o boot/bootsect.ss
	$(LD86) -s -o boot/bootsect boot/bootsect.o

zBoot/zSystem: zBoot/*.c zBoot/*.S tools/zSystem
	cd zBoot;$(MAKE)

zImage: $(CONFIGURE) boot/bootsect boot/setup zBoot/zSystem tools/build
	$(OBJCOPY) zBoot/zSystem zBoot/zSystem.out
	tools/build boot/bootsect boot/setup zBoot/zSystem.out $(ROOT_DEV) > zImage

zdisk: zImage
	dd bs=8192 if=zImage of=/dev/fd0

zlilo: $(CONFIGURE) zImage
	if [ -f /vmlinuz ]; then mv /vmlinuz /vmlinuz.old; fi
	cat zImage > /vmlinuz
	/etc/lilo/install

tools/zSystem:	boot/head.o init/main.o tools/version.o linuxsubdirs
	$(LD) $(LDFLAGS) -Ttext 100000 -M boot/head.o init/main.o tools/version.o \
		$(ARCHIVES) \
		$(FILESYSTEMS) \
		$(NETWORK) \
		$(DRIVERS) \
		$(MATH) \
		$(LIBS) \
		-o tools/zSystem > zSystem.map

fs: dummy
	$(MAKE) linuxsubdirs SUBDIRS=fs

lib: dummy
	$(MAKE) linuxsubdirs SUBDIRS=lib

mm: dummy
	$(MAKE) linuxsubdirs SUBDIRS=mm

kernel: dummy
	$(MAKE) linuxsubdirs SUBDIRS=kernel

clean:
	rm -f zImage zSystem.map tools/zSystem
	rm -f Image System.map core boot/bootsect boot/setup \
		boot/bootsect.ss boot/setup.ss boot/head.ss init/main.ss
	rm -f init/*.o tools/system tools/build boot/*.o tools/*.o
	for i in zBoot $(SUBDIRS); do (cd $$i && $(MAKE) clean); done

mrproper: clean
	rm -f include/linux/autoconf.h tools/version.h
	rm -f .version .config* config.old
	rm -f .depend `find . -name .depend -print`

backup: mrproper
	cd .. && tar cf - linux | gzip -9 > backup.gz

depend dep:
	touch tools/version.h
	for i in init/*.c;do echo -n "init/";$(CPP) -M $$i;done > .depend~
	echo -n "tools/";$(CPP) -M tools/version.c >> .depend~
	echo -n "tools/";$(HOSTCPP) -M tools/build.c >> .depend~
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) dep) || exit; done
	rm -f tools/version.h
	mv .depend~ .depend

ifdef CONFIGURATION
..$(CONFIGURATION):
	@echo
	@echo "You have a bad or nonexistent" .$(CONFIGURATION) ": running 'make" $(CONFIGURATION)"'"
	@echo
	$(MAKE) $(CONFIGURATION)
	@echo
	@echo "Successful. Try re-making (ignore the error that follows)"
	@echo
	exit 1

dummy: ..$(CONFIGURATION)

else

dummy:

endif

#
# Leave these dummy entries for now to tell people that they are going away..
#
lilo:
	@echo
	@echo Uncompressed kernel images no longer supported. Use
	@echo \"make zlilo\" instead.
	@echo
	@exit 1

Image:
	@echo
	@echo Uncompressed kernel images no longer supported. Use
	@echo \"make zImage\" instead.
	@echo
	@exit 1

disk:
	@echo
	@echo Uncompressed kernel images no longer supported. Use
	@echo \"make zdisk\" instead.
	@echo
	@exit 1
