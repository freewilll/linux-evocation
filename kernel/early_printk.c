#include <stdarg.h>
#include <linux/page.h>
#include <linux/segment.h>
#include <asm/system.h>
#include <linux/head.h>
#include <linux/types.h>
#include <glob.h>

extern char empty_zero_page[PAGE_SIZE];
extern "C" int vsprintf(char *,const char *,va_list);

#define PARAM	empty_zero_page
#define SCREEN_INFO (*(struct screen_info *) (PARAM+0))

char *vidmem = (char *)0xb8000;
int lines, cols;
static char printbuf[1024];

struct screen_info {
	unsigned char  orig_x;
	unsigned char  orig_y;
	unsigned char  unused1[2];
	unsigned short orig_video_page;
	unsigned char  orig_video_mode;
	unsigned char  orig_video_cols;
	unsigned short orig_video_ega_ax;
	unsigned short orig_video_ega_bx;
	unsigned short orig_video_ega_cx;
	unsigned char  orig_video_lines;
};

__ptr_t memcpy(__ptr_t __dest, __const __ptr_t __src,
			    size_t __n)
{
	int i;
	char *d = (char *)__dest, *s = (char *)__src;

	for (i=0;i<__n;i++) d[i] = s[i];
}

static void scroll()
{
	int i;

	memcpy ( vidmem, vidmem + cols * 2, ( lines - 1 ) * cols * 2 );
	for ( i = ( lines - 1 ) * cols * 2; i < lines * cols * 2; i += 2 )
		vidmem[i] = ' ';
}

static void puts(char *s)
{
	int x,y;
	char c;

	x = SCREEN_INFO.orig_x;
	y = SCREEN_INFO.orig_y;

	while ( ( c = *s++ ) != '\0' ) {
		if ( c == '\n' ) {
			x = 0;
			if ( ++y >= lines ) {
				scroll();
				y--;
			}
		} else {
			vidmem [ ( x + cols * y ) * 2 ] = c;
			if ( ++x >= cols ) {
				x = 0;
				if ( ++y >= lines ) {
					scroll();
					y--;
				}
			}
		}
	}

	SCREEN_INFO.orig_x = x;
	SCREEN_INFO.orig_y = y;
}

extern "C" int eprintk(const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i=vsprintf(printbuf,fmt,args);
	printbuf[i] = 0;
	puts(printbuf);
	va_end(args);
	return i;
}

void init_early_printk() {
	// For when the kernel is stil in its infancy.
	lines = SCREEN_INFO.orig_video_lines;
	cols = SCREEN_INFO.orig_video_cols;
	eprintk("Early kernel printk initialized\n");
}
