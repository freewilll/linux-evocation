#include <linux/page.h>
#include <linux/segment.h>
#include <asm/system.h>
#include <linux/head.h>

extern "C" void lcall7(void);

int hard_math = 0;		/* set by boot/head.S */
int x86 = 0;			/* set by boot/head.S to 3 or 4 */

long user_stack [ PAGE_SIZE>>2 ] ;

struct stack_struct {
	long * a;
	short b;
};

stack_struct stack_start = { & user_stack [PAGE_SIZE>>2] , KERNEL_DS };

struct desc_struct default_ldt;

extern "C" void start_kernel(void) {
	set_call_gate(&default_ldt,lcall7);
	for (;;);
}

extern "C" int printk(const char *fmt, ...)
{
	for (;;);
}
