#ifdef _SYS_DIR_H
#error "Can't include both sys/dir.h and sys/dirent.h"
#define _SYS_DIRENT_H
#endif

#ifndef _SYS_DIRENT_H
#define _SYS_DIRENT_H

#include <limits.h>
#include <sys/types.h>	/* for type off_t in dirent.h */
#include <linux/dirent.h>

#ifndef DIRSIZ
#define DIRSIZ NAME_MAX
#endif

#endif
