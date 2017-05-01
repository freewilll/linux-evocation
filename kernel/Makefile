#
# Makefile for the linux kernel.
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

SUBDIRS	= chr_drv blk_drv FPU-emu

OBJS  = sched.o sys_call.o traps.o irq.o dma.o fork.o \
	panic.o printk.o vsprintf.o sys.o exit.o \
	signal.o mktime.o ptrace.o ioport.o itimer.o \
	info.o ldt.o

all: kernel.o kernelsubdirs

kernel.o: $(OBJS)
	$(LD) -r -o kernel.o $(OBJS)

kernelsubdirs: dummy
	@for i in $(SUBDIRS); do (cd $$i && echo $$i && $(MAKE)) || exit; done

sys_call.ss: sys_call.S

sys_call.o: sys_call.ss

sched.o: sched.c
	$(CC) $(CFLAGS) $(PROFILING) -fno-omit-frame-pointer -c $<

clean:
	rm -f core *.o *.a *.ss
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) clean); done

dep:
	$(CPP) -M *.c > .depend
	for i in $(SUBDIRS); do (cd $$i && $(MAKE) dep) || exit; done

dummy:

#
# include a dependency file if one exists
#
ifeq (.depend,$(wildcard .depend))
include .depend
endif

