/*
 *  linux/tools/build.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

/*
 * This file builds a disk-image from three different files:
 *
 * - bootsect: max 510 bytes of 8086 machine code, loads the rest
 * - setup: max 4 sectors of 8086 machine code, sets up system parm
 * - system: 80386 code for actual system
 *
 * It does some checking that all files are of the correct type, and
 * just writes the result to stdout, removing headers and padding to
 * the right amount. It also writes some system data to stderr.
 */

/*
 * Changes by tytso to allow root device specification
 */

#include <stdio.h>	/* fprintf */
#include <string.h>
#include <stdlib.h>	/* contains exit */
#include <sys/types.h>	/* unistd.h needs this */
#ifndef __APPLE__
#include <sys/sysmacros.h> /* major and minor */
#endif
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "linux/config.h"
#include "linux/a.out.h"

#define MINIX_HEADER 32
#define GCC_HEADER 1024

#define SYS_SIZE DEF_SYSSIZE

#define DEFAULT_MAJOR_ROOT 0
#define DEFAULT_MINOR_ROOT 0

/* max nr of sectors of setup: don't change unless you also change
 * bootsect etc */
#define SETUP_SECTS 4

#define STRINGIFY(x) #x

typedef union {
	int l;
	short s[2];
	char b[4];
} conv;

int fd;

int intel_int(int l)
{
	conv t;

	t.b[0] = l & 0xff; l >>= 8;
	t.b[1] = l & 0xff; l >>= 8;
	t.b[2] = l & 0xff; l >>= 8;
	t.b[3] = l & 0xff; l >>= 8;
	return t.l;
}

short intel_short(short l)
{
	conv t;

	t.b[0] = l & 0xff; l >>= 8;
	t.b[1] = l & 0xff; l >>= 8;
	return t.s[0];
}

void die(char const *str)
{
	fprintf(stderr,"%s\n",str);
	exit(1);
}

void usage(void)
{
	die("Usage: build bootsect setup system [rootdev] [> image]");
}

int main(int argc, char ** argv)
{
	int i,c,id, sz;
	unsigned long sys_size;
	char buf[1024];
	int *intbuf = (int *)buf;
	char major_root, minor_root;
	struct stat sb;

	if ((argc < 4) || (argc > 5))
		usage();
	if (argc > 4) {
		if (!strcmp(argv[4], "CURRENT")) {
			if (stat("/", &sb)) {
				perror("/");
				die("Couldn't stat /");
			}
			major_root = major(sb.st_dev);
			minor_root = minor(sb.st_dev);
		} else if (strcmp(argv[4], "FLOPPY")) {
			if (stat(argv[4], &sb)) {
				perror(argv[4]);
				die("Couldn't stat root device.");
			}
			major_root = major(sb.st_rdev);
			minor_root = minor(sb.st_rdev);
		} else {
			major_root = 0;
			minor_root = 0;
		}
	} else {
		major_root = DEFAULT_MAJOR_ROOT;
		minor_root = DEFAULT_MINOR_ROOT;
	}
	fprintf(stderr, "Root device is (%d, %d)\n", major_root, minor_root);
	for (i=0;i<(int)sizeof buf; i++) buf[i]=0;
	if ((id=open(argv[1],O_RDONLY,0))<0)
		die("Unable to open 'boot'");
	if (read(id,buf,MINIX_HEADER) != MINIX_HEADER)
		die("Unable to read header of 'boot'");
	if (intbuf[0]!=intel_int(0x04100301))
		die("Non-Minix header of 'boot'");
	if (((int *) buf)[1]!=intel_int(MINIX_HEADER))
		die("Non-Minix header of 'boot'");
	if (((int *) buf)[3] != 0)
		die("Illegal data segment in 'boot'");
	if (((int *) buf)[4] != 0)
		die("Illegal bss in 'boot'");
	if (((int *) buf)[5] != 0)
		die("Non-Minix header of 'boot'");
	if (((int *) buf)[7] != 0)
		die("Illegal symbol table in 'boot'");
	i=read(id,buf,sizeof buf);
	fprintf(stderr,"Boot sector %d bytes.\n",i);
	if (i != 512)
		die("Boot block must be exactly 512 bytes");
	if ((*(unsigned short *)(buf+510)) != (unsigned short)intel_short(0xAA55))
		die("Boot block hasn't got boot flag (0xAA55)");
	buf[508] = (char) minor_root;
	buf[509] = (char) major_root;	
	i=write(1,buf,512);
	if (i!=512)
		die("Write call failed");
	close (id);
	
	if ((id=open(argv[2],O_RDONLY,0))<0)
		die("Unable to open 'setup'");
	if (read(id,buf,MINIX_HEADER) != MINIX_HEADER)
		die("Unable to read header of 'setup'");
	if (intbuf[0]!=intel_int(0x04100301))
		die("Non-Minix header of 'setup'");
	if (((int *) buf)[1]!=intel_int(MINIX_HEADER))
		die("Non-Minix header of 'setup'");
	if (((int *) buf)[3] != 0)
		die("Illegal data segment in 'setup'");
	if (((int *) buf)[4] != 0)
		die("Illegal bss in 'setup'");
	if (((int *) buf)[5] != 0)
		die("Non-Minix header of 'setup'");
	if (((int *) buf)[7] != 0)
		die("Illegal symbol table in 'setup'");
	for (i=0 ; (c=read(id,buf,sizeof buf))>0 ; i+=c )
		if (write(1,buf,c)!=c)
			die("Write call failed");
	if (c != 0)
		die("read-error on 'setup'");
	close (id);
	if (i > SETUP_SECTS*512)
		die("Setup exceeds " STRINGIFY(SETUP_SECTS)
			" sectors - rewrite build/boot/setup");
	fprintf(stderr,"Setup is %d bytes.\n",i);
	for (c=0 ; c<(int)sizeof(buf) ; c++)
		buf[c] = '\0';
	while (i<SETUP_SECTS*512) {
		c = SETUP_SECTS*512-i;
		if (c > (int)sizeof(buf))
			c = sizeof(buf);
		if (write(1,buf,c) != c)
			die("Write call failed");
		i += c;
	}
	

	if ((id=open(argv[3],O_RDONLY,0))<0)
		die("Unable to open 'system'");
	if (fstat (id, &sb))
		die("Unable to stat 'system'");

	sz = sb.st_size;
	fprintf (stderr, "System is %d kB\n", sz/1024);
	sys_size = (sz + 15) / 16;
	/* 0x28000*16 = 2.5 MB, conservative estimate for the current maximum */
	if (sys_size > 0xffff)
		fprintf(stderr,"warning: kernel is too big for standalone boot "
		    "from floppy\n");
	while (sz > 0) {
		int l, n;

		l = (sz > (int)sizeof(buf)) ? sizeof(buf) : sz;
		if ((n=read(id, buf, l)) != l) {
			if (n < 0)
				die("Error reading 'system'");
			else
				die("Unexpected EOF");
		}
		if (write(1, buf, l) != l)
			die("Write failed");
		sz -= l;
	}
	close(id);

	if (lseek(1,500,0) == 500) {
		buf[0] = (sys_size & 0xff);
		buf[1] = ((sys_size >> 8) & 0xff);
		if (write(1, buf, 2) != 2)
			die("Write call failed");
	}
	return(0);
}
