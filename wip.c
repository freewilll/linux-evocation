#include <linux/page.h>
#include <linux/segment.h>

int hard_math = 0;		/* set by boot/head.S */
int x86 = 0;			/* set by boot/head.S to 3 or 4 */

long user_stack [ PAGE_SIZE>>2 ] ;

struct stack_struct {
	long * a;
	short b;
};

stack_struct stack_start = { & user_stack [PAGE_SIZE>>2] , KERNEL_DS };

extern "C" void start_kernel(void) {
	for (;;);
}
