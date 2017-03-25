
HEAD = head.o
SYSTEM = ../tools/zSystem

OBJCOPY = $(CROSS_COMPILE)objcopy -O binary -R .note -R .comment -R .eh_frame -S

zOBJECTS = $(HEAD) inflate.o unzip.o misc.o string.o

CFLAGS := -O6 -DSTDC_HEADERS $(TEST) -fno-stack-protector -fno-builtin -fno-tree-loop-distribute-patterns

ZIMAGE_OFFSET = 0x0
ZLDFLAGS = -e startup_32
ZLINKFLAGS = -Ttext $(ZIMAGE_OFFSET) $(ZLDFLAGS)

.c.s:
	$(CC) $(CFLAGS) -S -o $*.s $<
.ss.o:
	$(AS) -c -o $*.o $<
.c.o:
	$(CC) $(CFLAGS) -c -o $*.o $<

all:	zSystem

zSystem:	piggy.o $(zOBJECTS)
		$(LD) $(ZLINKFLAGS) -o zSystem $(zOBJECTS) piggy.o

head.o:	head.ss

head.ss: head.S ../include/linux/tasks.h
	$(CPP) -traditional head.S -o head.ss

piggy.o: $(SYSTEM)
	tmppiggy=_tmp_$$$$piggy; \
	rm -f $$tmppiggy $$tmppiggy.gz $$tmppiggy.lnk; \
	$(OBJCOPY) $(SYSTEM) $$tmppiggy; \
	gzip -f -9 < $$tmppiggy > $$tmppiggy.gz; \
	echo "SECTIONS { .data : { input_len = .; LONG(input_data_end - input_data) input_data = .; *(.data) input_data_end = .; }}" > $$tmppiggy.lnk; \
	$(LD) -r -o piggy.o -b binary $$tmppiggy.gz -b elf32-i386 -T $$tmppiggy.lnk; \
	rm -f $$tmppiggy $$tmppiggy.gz $$tmppiggy.lnk

$(SYSTEM):
	cd ..;make tools/zSystem

clean:
	-rm -f *.o *.ss zSystem core piggy.* _tmp_* zBoot/zSystem.out
