/*
 *  linux/kernel/blk_drv/ramdisk.c
 *
 *  Written by Theodore Ts'o, 12/2/91
 *
 * Modifications by Fred N. van Kempen to allow for bootable root
 * disks (which are used in LINUX/Pro).  Also some cleanups.  03/03/93
 */


#include <linux/config.h>
#include <linux/sched.h>
#include <linux/minix_fs.h>
#include <linux/ext2_fs.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <asm/system.h>
#include <asm/segment.h>

#define MAJOR_RAMDISK	1		/* should be in <linux/major.h>	*/
#define MAJOR_FLOPPY	2		/* should be in <linux/major.h>	*/
#define MINOR_RAMDISK	1

#define MAJOR_NR	MAJOR_RAMDISK	/* weird hack- FvK */
#include "blk.h"


char	*rd_start;
int	rd_length = 0;
static int rd_blocksizes[2] = {0, 0};

static void do_rd_request(void)
{
	int	len;
	char	*addr;

repeat:
	INIT_REQUEST;
	addr = rd_start + (CURRENT->sector << 9);
	len = CURRENT->nr_sectors << 9;
	if ((MINOR(CURRENT->dev) != MINOR_RAMDISK) ||
	    (addr+len > rd_start+rd_length)) {
		end_request(0);
		goto repeat;
	}
	if (CURRENT-> cmd == WRITE) {
		(void ) memcpy(addr,
			      CURRENT->buffer,
			      len);
	} else if (CURRENT->cmd == READ) {
		(void) memcpy(CURRENT->buffer, 
			      addr,
			      len);
	} else
		panic("RAMDISK: unknown RAM disk command !\n");
	end_request(1);
	goto repeat;
}

static struct file_operations rd_fops = {
	NULL,			/* lseek - default */
	block_read,		/* read - general block-dev read */
	block_write,		/* write - general block-dev write */
	NULL,			/* readdir - bad */
	NULL,			/* select */
	NULL,			/* ioctl */
	NULL,			/* mmap */
	NULL,			/* no special open code */
	NULL,			/* no special release code */
	block_fsync		/* fsync */
};

/*
 * Returns amount of memory which needs to be reserved.
 */
long rd_init(long mem_start, int length)
{
	int	i;
	char	*cp;

	if (register_blkdev(MAJOR_RAMDISK,"rd",&rd_fops)) {
		printk("RAMDISK: Unable to get major %d.\n", MAJOR_RAMDISK);
		return 0;
	}
	blk_dev[MAJOR_RAMDISK].request_fn = DEVICE_REQUEST;
	rd_start = (char *) mem_start;
	rd_length = length;
	cp = rd_start;
	for (i=0; i < length; i++)
		*cp++ = '\0';

	for(i=0;i<2;i++) rd_blocksizes[i] = 1024;
	blksize_size[MAJOR_NR] = rd_blocksizes;

	return(length);
}

/*
 * If the root device is the RAM disk, try to load it.
 * In order to do this, the root device is originally set to the
 * floppy, and we later change it to be RAM disk.
 */
void rd_load(void)
{
	struct buffer_head *bh;
	struct minix_super_block msb;
	struct ext2_super_block esb;
	int		block, tries;
	int		i = 1;
	int		nblocks;
	char		*cp;

	/* If no RAM disk specified, give up early. */
	if (!rd_length) return;
	printk("RAMDISK: %d bytes, starting at 0x%x\n",
					rd_length, (int) rd_start);

	/* If we are doing a diskette boot, we might have to pre-load it. */
	if (MAJOR(ROOT_DEV) != MAJOR_FLOPPY) return;

	/*
	 * Check for a super block on the diskette.
	 * The old-style boot/root diskettes had their RAM image
	 * starting at block 512 of the boot diskette.  LINUX/Pro
	 * uses the enire diskette as a file system, so in that
	 * case, we have to look at block 0.  Be intelligent about
	 * this, and check both... - FvK
	 *
	 * Also look in multiples of 256 to allow for 1 MB images.
	 */
	for (tries = 0; tries < 1536; tries += 256) {
		block = tries;
		bh = breada(ROOT_DEV,block+1,block,block+2,-1);
		if (!bh) {
			printk("RAMDISK: I/O error while looking for super block!\n");
			return;
		}

		*((struct minix_super_block *) &msb) =
			*((struct minix_super_block *) bh->b_data);

		*((struct ext2_super_block *) &esb) =
			*((struct ext2_super_block *) bh->b_data);

		brelse(bh);
		
		if (msb.s_magic == MINIX_SUPER_MAGIC) {
			nblocks = msb.s_nzones << msb.s_log_zone_size;
			printk("Found minix ramdisk at block %d\n", block);
		}
		else if (esb.s_magic == EXT2_SUPER_MAGIC || esb.s_magic == EXT2_OLD_SUPER_MAGIC) {
			nblocks = esb.s_blocks_count;
			printk("Found ext2 ramdisk at block %d\n", block);
		}
		else {
			printk("RAMDISK: trying old-style RAM image.\n");
			continue;
		}

		if (nblocks > (rd_length >> BLOCK_SIZE_BITS)) {
			printk("RAMDISK: image too big! (%d/%d blocks)\n",
					nblocks, rd_length >> BLOCK_SIZE_BITS);
			return;
		}
		printk("RAMDISK: Loading %d blocks into RAM disk", nblocks);

		/* We found an image file system.  Load it into core! */
		cp = rd_start;
		while (nblocks) {
			if (nblocks > 2) 
				bh = breada(ROOT_DEV, block, block+1, block+2, -1);
			else
				bh = bread(ROOT_DEV, block, BLOCK_SIZE);
			if (!bh) {
				printk("RAMDISK: I/O error on block %d, aborting!\n", 
				block);
				return;
			}
			(void) memcpy(cp, bh->b_data, BLOCK_SIZE);
			brelse(bh);
			if (!(nblocks-- & 15)) printk(".");
			cp += BLOCK_SIZE;
			block++;
			i++;
		}
		printk("\ndone\n");

		/* We loaded the file system image.  Prepare for mounting it. */
		ROOT_DEV = ((MAJOR_RAMDISK << 8) | MINOR_RAMDISK);
		return;
	}
}
