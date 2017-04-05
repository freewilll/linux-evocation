#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/tty.h>
#include <linux/user.h>

// TODO WGJA tcp_get_info
int tcp_get_info(char* buffer)
{
	printk("TODO WGJA tcp_get_info\n");
	return 0;
}

// TODO WGJA udp_get_info
int udp_get_info(char* buffer)
{
	printk("TODO WGJA udp_get_info\n");
	return 0;
}

// TODO WGJA unix_get_info
int unix_get_info(char* buffer)
{
	printk("TODO WGJA unix_get_info\n");
	return 0;
}

// TODO WGJA arp_get_info
int arp_get_info(char* buffer)
{
	printk("TODO WGJA arp_get_info\n");
	return 0;
}

// TODO WGJA rt_get_info
int rt_get_info(char* buffer)
{
	printk("TODO WGJA rt_get_info\n");
	return 0;
}

// TODO WGJA dev_get_info
int dev_get_info(char* buffer)
{
	printk("TODO WGJA dev_get_info\n");
	return 0;
}

// TODO WGJA raw_get_info
int raw_get_info(char* buffer)
{
	printk("TODO WGJA raw_get_info\n");
	return 0;
}

// TODO WGJA eth_setup
void eth_setup(char *str, int *ints)
{
	printk("TODO WGJA eth_setup\n");
	return;
}

// TODO WGJA arp_ioctl
int arp_ioctl(unsigned int cmd, void* arg)
{
	printk("TODO WGJA arp_ioctl\n");
}

// TODO WGJA unix_proto_init
void unix_proto_init(struct ddi_proto *pro)
{
	printk("TODO WGJA unix_proto_init\n");
}

// TODO WGJA inet_proto_init
void inet_proto_init(struct ddi_proto *pro)
{
	printk("TODO WGJA inet_proto_init\n");
}
