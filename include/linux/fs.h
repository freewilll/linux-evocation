#ifndef _LINUX_FS_H
#define _LINUX_FS_H

/*
 * This file has definitions for some important file table
 * structures etc.
 */

#include <linux/limits.h>
#include <linux/wait.h>
#include <linux/types.h>
// TODO WGJA WIP: #include <linux/dirent.h>
// TODO WGJA WIP: #include <linux/vfs.h>
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * It's silly to have NR_OPEN bigger than NR_FILE, but I'll fix
// TODO WGJA WIP:  * that later. Anyway, now the file code is no longer dependent
// TODO WGJA WIP:  * on bitmaps in unsigned longs, but uses the new fd_set structure..
// TODO WGJA WIP:  *
// TODO WGJA WIP:  * Some programs (notably those using select()) may have to be 
// TODO WGJA WIP:  * recompiled to take full advantage of the new limits..
// TODO WGJA WIP:  */
// TODO WGJA WIP: #undef NR_OPEN
// TODO WGJA WIP: #define NR_OPEN 256
// TODO WGJA WIP: 
// TODO WGJA WIP: #define NR_INODE 2048	/* this should be bigger than NR_FILE */
#define NR_FILE 1024	/* this can well be larger on a larger system */
// TODO WGJA WIP: #define NR_SUPER 32
// TODO WGJA WIP: #define NR_HASH 997
// TODO WGJA WIP: #define NR_IHASH 131
// TODO WGJA WIP: #define NR_FILE_LOCKS 64
// TODO WGJA WIP: #define BLOCK_SIZE 1024
// TODO WGJA WIP: #define BLOCK_SIZE_BITS 10
#define MAX_CHRDEV 32
// TODO WGJA WIP: #define MAX_BLKDEV 32

/* devices are as follows: (same as minix, so we can use the minix
 * file system. These are major numbers.)
 *
 *  0 - unnamed (minor 0 = true nodev)
 *  1 - /dev/mem
 *  2 - /dev/fd
 *  3 - /dev/hd
 *  4 - /dev/ttyx
 *  5 - /dev/tty
 *  6 - /dev/lp
 *  7 -
 *  8 - /dev/sd
 *  9 - /dev/st
 * 10 - mice
 * 11 - scsi cdrom
 * 12 -
 * 13 -
 * 14 - sound card (?)
 * 15 -
 */

// TODO WGJA WIP: #define UNNAMED_MAJOR 0

#define MAY_EXEC 1
#define MAY_WRITE 2
#define MAY_READ 4

#define READ 0
#define WRITE 1
#define READA 2		/* read-ahead - don't pause */
#define WRITEA 3	/* "write-ahead" - silly, but somewhat useful */

// TODO WGJA WIP: extern void buffer_init(void);
// TODO WGJA WIP: extern unsigned long inode_init(unsigned long start, unsigned long end);
// TODO WGJA WIP: extern unsigned long file_table_init(unsigned long start, unsigned long end);

#define MAJOR(a) (((unsigned)(a))>>8)
#define MINOR(a) ((a)&0xff)

// TODO WGJA WIP: #ifndef NULL
// TODO WGJA WIP: #define NULL ((void *) 0)
// TODO WGJA WIP: #endif
// TODO WGJA WIP: 
// TODO WGJA WIP: #define NIL_FILP	((struct file *)0)
// TODO WGJA WIP: #define SEL_IN		1
// TODO WGJA WIP: #define SEL_OUT		2
// TODO WGJA WIP: #define SEL_EX		4

/*
 * These are the fs-independent mount-flags: up to 16 flags are supported
 */
#define MS_RDONLY    1 /* mount read-only */
#define MS_NOSUID    2 /* ignore suid and sgid bits */
#define MS_NODEV     4 /* disallow access to device special files */
#define MS_NOEXEC    8 /* disallow program execution */
#define MS_SYNC     16 /* writes are synced at once */
#define	MS_REMOUNT  32 /* alter flags of a mounted FS */

// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Flags that can be altered by MS_REMOUNT
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define MS_RMT_MASK (MS_RDONLY)
// TODO WGJA WIP: 
// TODO WGJA WIP: /*
// TODO WGJA WIP:  * Magic mount flag number. Has to be or-ed to the flag values.
// TODO WGJA WIP:  */
// TODO WGJA WIP: #define MS_MGC_VAL 0xC0ED0000 /* magic flag number to indicate "new" flags */
// TODO WGJA WIP: #define MS_MGC_MSK 0xffff0000 /* magic flag number mask */

/*
 * Note that read-only etc flags are inode-specific: setting some file-system
 * flags just means all the inodes inherit those flags by default. It might be
 * possible to overrride it sevelctively if you really wanted to with some
 * ioctl() that is not currently implemented.
 *
 * Exception: MS_RDONLY is always applied to the entire file system.
 */
#define IS_RDONLY(inode) (((inode)->i_sb) && ((inode)->i_sb->s_flags & MS_RDONLY))
#define IS_NOSUID(inode) ((inode)->i_flags & MS_NOSUID)
#define IS_NODEV(inode) ((inode)->i_flags & MS_NODEV)
#define IS_NOEXEC(inode) ((inode)->i_flags & MS_NOEXEC)
#define IS_SYNC(inode) ((inode)->i_flags & MS_SYNC)

// TODO WGJA WIP: /* the read-only stuff doesn't really belong here, but any other place is
// TODO WGJA WIP:    probably as bad and I don't want to create yet another include file. */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define BLKROSET 4701 /* set device read-only (0 = read-write) */
// TODO WGJA WIP: #define BLKROGET 4702 /* get read-only status (0 = read_write) */
// TODO WGJA WIP: #define BLKRRPART 4703 /* re-read partition table */
// TODO WGJA WIP: #define BLKGETSIZE 4704 /* return device size */
// TODO WGJA WIP: #define BLKFLSBUF 4705 /* flush buffer cache */
// TODO WGJA WIP: 
// TODO WGJA WIP: /* These are a few other constants  only used by scsi  devices */
// TODO WGJA WIP: 
// TODO WGJA WIP: #define SCSI_IOCTL_GET_IDLUN 0x5382
// TODO WGJA WIP: 
// TODO WGJA WIP: 
// TODO WGJA WIP: #define BMAP_IOCTL 1	/* obsolete - kept for compatibility */
// TODO WGJA WIP: #define FIBMAP	   1	/* bmap access */
// TODO WGJA WIP: #define FIGETBSZ   2	/* get the block size used for bmap */
// TODO WGJA WIP: 
// TODO WGJA WIP: /* these flags tell notify_change what is being changed */
// TODO WGJA WIP: 
#define NOTIFY_SIZE	1
// TODO WGJA WIP: #define NOTIFY_MODE	2
// TODO WGJA WIP: #define NOTIFY_TIME	4
// TODO WGJA WIP: #define NOTIFY_UIDGID	8
// TODO WGJA WIP: 
// TODO WGJA WIP: typedef char buffer_block[BLOCK_SIZE];
// TODO WGJA WIP: 
// TODO WGJA WIP: struct buffer_head {
// TODO WGJA WIP: 	char * b_data;			/* pointer to data block (1024 bytes) */
// TODO WGJA WIP: 	unsigned long b_size;		/* block size */
// TODO WGJA WIP: 	unsigned long b_blocknr;	/* block number */
// TODO WGJA WIP: 	dev_t b_dev;			/* device (0 = free) */
// TODO WGJA WIP: 	unsigned short b_count;		/* users using this block */
// TODO WGJA WIP: 	unsigned char b_uptodate;
// TODO WGJA WIP: 	unsigned char b_dirt;		/* 0-clean,1-dirty */
// TODO WGJA WIP: 	unsigned char b_lock;		/* 0 - ok, 1 -locked */
// TODO WGJA WIP: 	unsigned char b_req;		/* 0 if the buffer has been invalidated */
// TODO WGJA WIP: 	struct wait_queue * b_wait;
// TODO WGJA WIP: 	struct buffer_head * b_prev;		/* doubly linked list of hash-queue */
// TODO WGJA WIP: 	struct buffer_head * b_next;
// TODO WGJA WIP: 	struct buffer_head * b_prev_free;	/* doubly linked list of buffers */
// TODO WGJA WIP: 	struct buffer_head * b_next_free;
// TODO WGJA WIP: 	struct buffer_head * b_this_page;	/* circular list of buffers in one page */
// TODO WGJA WIP: 	struct buffer_head * b_reqnext;		/* request queue */
// TODO WGJA WIP: };

#include <linux/pipe_fs_i.h>
#include <linux/minix_fs_i.h>
#include <linux/ext_fs_i.h>
#include <linux/ext2_fs_i.h>
#include <linux/msdos_fs_i.h>
#include <linux/iso_fs_i.h>
#include <linux/nfs_fs_i.h>
#include <linux/xia_fs_i.h>

struct inode {
	dev_t		i_dev;
	unsigned long	i_ino;
	umode_t		i_mode;
	nlink_t		i_nlink;
	uid_t		i_uid;
	gid_t		i_gid;
	dev_t		i_rdev;
	off_t		i_size;
	time_t		i_atime;
	time_t		i_mtime;
	time_t		i_ctime;
	unsigned long	i_blksize;
	unsigned long	i_blocks;
	struct inode_operations * i_op;
	struct super_block * i_sb;
	struct wait_queue * i_wait;
	struct file_lock * i_flock;
	struct vm_area_struct * i_mmap;
	struct inode * i_next, * i_prev;
	struct inode * i_hash_next, * i_hash_prev;
	struct inode * i_bound_to, * i_bound_by;
	unsigned short i_count;
	unsigned short i_flags;
	unsigned char i_lock;
	unsigned char i_dirt;
	unsigned char i_pipe;
	unsigned char i_mount;
	unsigned char i_seek;
	unsigned char i_update;
	union {
		struct pipe_inode_info pipe_i;
		struct minix_inode_info minix_i;
		struct ext_inode_info ext_i;
		struct ext2_inode_info ext2_i;
		struct msdos_inode_info msdos_i;
		struct iso_inode_info isofs_i;
		struct nfs_inode_info nfs_i;
		struct xiafs_inode_info xiafs_i;
	} u;
};

struct file {
	mode_t f_mode;
	dev_t f_rdev;			/* needed for /dev/tty */
	off_t f_pos;
	unsigned short f_flags;
	unsigned short f_count;
	unsigned short f_reada;
	struct file *f_next, *f_prev;
	struct inode * f_inode;
	struct file_operations * f_op;
};

// TODO WGJA WIP: struct file_lock {
// TODO WGJA WIP: 	struct file_lock *fl_next;	/* singly linked list */
// TODO WGJA WIP: 	struct task_struct *fl_owner;	/* NULL if on free list, for sanity checks */
// TODO WGJA WIP: 	struct wait_queue *fl_wait;
// TODO WGJA WIP: 	char fl_type;
// TODO WGJA WIP: 	char fl_whence;
// TODO WGJA WIP: 	off_t fl_start;
// TODO WGJA WIP: 	off_t fl_end;
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: #include <linux/minix_fs_sb.h>
// TODO WGJA WIP: #include <linux/ext_fs_sb.h>
// TODO WGJA WIP: #include <linux/ext2_fs_sb.h>
// TODO WGJA WIP: #include <linux/msdos_fs_sb.h>
// TODO WGJA WIP: #include <linux/iso_fs_sb.h>
// TODO WGJA WIP: #include <linux/nfs_fs_sb.h>
// TODO WGJA WIP: #include <linux/xia_fs_sb.h>

struct super_block {
	dev_t s_dev;
	unsigned long s_blocksize;
	unsigned char s_blocksize_bits;
	unsigned char s_lock;
	unsigned char s_rd_only;
	unsigned char s_dirt;
	struct super_operations *s_op;
	unsigned long s_flags;
	unsigned long s_magic;
	unsigned long s_time;
	struct inode * s_covered;
	struct inode * s_mounted;
	struct wait_queue * s_wait;
	union {
// TODO WGJA WIP: 		struct minix_sb_info minix_sb;
// TODO WGJA WIP: 		struct ext_sb_info ext_sb;
// TODO WGJA WIP: 		struct ext2_sb_info ext2_sb;
// TODO WGJA WIP: 		struct msdos_sb_info msdos_sb;
// TODO WGJA WIP: 		struct isofs_sb_info isofs_sb;
// TODO WGJA WIP: 		struct nfs_sb_info nfs_sb;
// TODO WGJA WIP: 		struct xiafs_sb_info xiafs_sb;
	} u;
};

struct file_operations {
	int (*lseek) (struct inode *, struct file *, off_t, int);
	int (*read) (struct inode *, struct file *, char *, int);
	int (*write) (struct inode *, struct file *, char *, int);
	int (*readdir) (struct inode *, struct file *, struct dirent *, int);
	int (*select) (struct inode *, struct file *, int, select_table *);
	int (*ioctl) (struct inode *, struct file *, unsigned int, unsigned long);
	int (*mmap) (struct inode *, struct file *, unsigned long, size_t, int, unsigned long);
	int (*open) (struct inode *, struct file *);
	void (*release) (struct inode *, struct file *);
	int (*fsync) (struct inode *, struct file *);
};

struct inode_operations {
	struct file_operations * default_file_ops;
	int (*create) (struct inode *,const char *,int,int,struct inode **);
	int (*lookup) (struct inode *,const char *,int,struct inode **);
	int (*link) (struct inode *,struct inode *,const char *,int);
	int (*unlink) (struct inode *,const char *,int);
	int (*symlink) (struct inode *,const char *,int,const char *);
	int (*mkdir) (struct inode *,const char *,int,int);
	int (*rmdir) (struct inode *,const char *,int);
	int (*mknod) (struct inode *,const char *,int,int,int);
	int (*rename) (struct inode *,const char *,int,struct inode *,const char *,int);
	int (*readlink) (struct inode *,char *,int);
	int (*follow_link) (struct inode *,struct inode *,int,int,struct inode **);
	int (*bmap) (struct inode *,int);
	void (*truncate) (struct inode *);
	int (*permission) (struct inode *, int);
};

struct super_operations {
	void (*read_inode) (struct inode *);
	int (*notify_change) (int flags, struct inode *);
	void (*write_inode) (struct inode *);
	void (*put_inode) (struct inode *);
	void (*put_super) (struct super_block *);
	void (*write_super) (struct super_block *);
	void (*statfs) (struct super_block *, struct statfs *);
	int (*remount_fs) (struct super_block *, int *);
};

// TODO WGJA WIP: struct file_system_type {
// TODO WGJA WIP: 	struct super_block *(*read_super) (struct super_block *, void *, int);
// TODO WGJA WIP: 	char *name;
// TODO WGJA WIP: 	int requires_dev;
// TODO WGJA WIP: };
// TODO WGJA WIP: 
// TODO WGJA WIP: #ifdef __KERNEL__
// TODO WGJA WIP: 
extern "C" int sys_open(const char *, int, int);
// TODO WGJA WIP: extern "C" int sys_close(unsigned int);		/* yes, it's really unsigned */

extern int getname(const char * filename, char **result);
extern void putname(char * name);

// TODO WGJA WIP: extern int register_blkdev(unsigned int, const char *, struct file_operations *);
// TODO WGJA WIP: extern int blkdev_open(struct inode * inode, struct file * filp);
// TODO WGJA WIP: extern struct file_operations def_blk_fops;
// TODO WGJA WIP: extern struct inode_operations blkdev_inode_operations;

extern int register_chrdev(unsigned int, const char *, struct file_operations *);
// TODO WGJA WIP: extern int chrdev_open(struct inode * inode, struct file * filp);
// TODO WGJA WIP: extern struct file_operations def_chr_fops;
// TODO WGJA WIP: extern struct inode_operations chrdev_inode_operations;
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void init_fifo(struct inode * inode);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern struct file_operations connecting_pipe_fops;
// TODO WGJA WIP: extern struct file_operations read_pipe_fops;
// TODO WGJA WIP: extern struct file_operations write_pipe_fops;
// TODO WGJA WIP: extern struct file_operations rdwr_pipe_fops;
// TODO WGJA WIP: 
// TODO WGJA WIP: extern struct file_system_type *get_fs_type(char *name);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int fs_may_mount(dev_t dev);
// TODO WGJA WIP: extern int fs_may_umount(dev_t dev, struct inode * mount_root);
// TODO WGJA WIP: extern int fs_may_remount_ro(dev_t dev);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern struct file *first_file;
// TODO WGJA WIP: extern int nr_files;
// TODO WGJA WIP: extern struct super_block super_blocks[NR_SUPER];
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void grow_buffers(int size);
extern int shrink_buffers(unsigned int priority);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int nr_buffers;
// TODO WGJA WIP: extern int buffermem;
// TODO WGJA WIP: extern int nr_buffer_heads;
// TODO WGJA WIP: 
// TODO WGJA WIP: extern void check_disk_change(dev_t dev);
// TODO WGJA WIP: extern void invalidate_inodes(dev_t dev);
// TODO WGJA WIP: extern void invalidate_buffers(dev_t dev);
// TODO WGJA WIP: extern int floppy_change(struct buffer_head * first_block);
// TODO WGJA WIP: extern void sync_inodes(dev_t dev);
// TODO WGJA WIP: extern void sync_dev(dev_t dev);
// TODO WGJA WIP: extern int fsync_dev(dev_t dev);
// TODO WGJA WIP: extern void sync_supers(dev_t dev);
// TODO WGJA WIP: extern int bmap(struct inode * inode,int block);
extern int notify_change(int flags, struct inode * inode);
// TODO WGJA WIP: extern int namei(const char * pathname, struct inode ** res_inode);
// TODO WGJA WIP: extern int lnamei(const char * pathname, struct inode ** res_inode);
// TODO WGJA WIP: extern int permission(struct inode * inode,int mask);
extern int open_namei(const char * pathname, int flag, int mode,
	struct inode ** res_inode, struct inode * base);
// TODO WGJA WIP: extern int do_mknod(const char * filename, int mode, dev_t dev);
extern void iput(struct inode * inode);
// TODO WGJA WIP: extern struct inode * iget(struct super_block * sb,int nr);
// TODO WGJA WIP: extern struct inode * get_empty_inode(void);
// TODO WGJA WIP: extern void insert_inode_hash(struct inode *);
// TODO WGJA WIP: extern void clear_inode(struct inode *);
// TODO WGJA WIP: extern struct inode * get_pipe_inode(void);
extern struct file * get_empty_filp(void);
// TODO WGJA WIP: extern struct buffer_head * get_hash_table(dev_t dev, int block, int size);
// TODO WGJA WIP: extern struct buffer_head * getblk(dev_t dev, int block, int size);
// TODO WGJA WIP: extern void ll_rw_block(int rw, int nr, struct buffer_head * bh[]);
// TODO WGJA WIP: extern void ll_rw_page(int rw, int dev, int nr, char * buffer);
// TODO WGJA WIP: extern void ll_rw_swap_file(int rw, int dev, unsigned int *b, int nb, char *buffer);
// TODO WGJA WIP: extern void brelse(struct buffer_head * buf);
// TODO WGJA WIP: extern void set_blocksize(dev_t dev, int size);
// TODO WGJA WIP: extern struct buffer_head * bread(dev_t dev, int block, int size);
// TODO WGJA WIP: extern unsigned long bread_page(unsigned long addr,dev_t dev,int b[],int size,int prot);
// TODO WGJA WIP: extern struct buffer_head * breada(dev_t dev,int block,...);
// TODO WGJA WIP: extern void put_super(dev_t dev);
extern dev_t ROOT_DEV;

// TODO WGJA WIP: extern void mount_root(void);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int char_read(struct inode *, struct file *, char *, int);
// TODO WGJA WIP: extern int block_read(struct inode *, struct file *, char *, int);
// TODO WGJA WIP: extern int read_ahead[];
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int char_write(struct inode *, struct file *, char *, int);
// TODO WGJA WIP: extern int block_write(struct inode *, struct file *, char *, int);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int generic_mmap(struct inode *, struct file *, unsigned long, size_t, int, unsigned long);
// TODO WGJA WIP: 
// TODO WGJA WIP: extern int block_fsync(struct inode *, struct file *);
// TODO WGJA WIP: extern int file_fsync(struct inode *, struct file *);
// TODO WGJA WIP: 
// TODO WGJA WIP: #endif /* __KERNEL__ */

#endif
