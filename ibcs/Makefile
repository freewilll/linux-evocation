#
# Makefile for the iBCS emulator files
#
# Note! Dependencies are done automagically by 'make dep', which also
# removes any old dependencies. DON'T put your own dependencies here
# unless it's something special (ie not a .c file).
#
# Note 2! The CFLAGS definitions are now in the main makefile...

SUBDIRS	= 

OBJS  = emulate.o

ibcs.o: $(OBJS)
	$(LD) -r -o ibcs.o $(OBJS)

clean:
	rm -f core *.o *.a *.s *.ss
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

