#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

unsigned int hook_fn(void* priv, struct sk_buff* skb, struct nf_hook_state const* state)
{
    return NF_QUEUE;
}

static struct nf_hook_ops hook_in, hook_out;
static int modinit(void)
{
    hook_in.hook = &hook_fn;
    hook_in.pf = PF_INET;
    hook_in.hooknum = NF_INET_LOCAL_IN;
    hook_in.priority = NF_IP_PRI_FIRST;
    
    if (nf_register_net_hook(&init_net, &hook_in))
    {
        printk(KERN_WARNING "failed to register_net_hook hook_in\n");
        return -1;
    }

    hook_out.hook = &hook_fn;
    hook_out.pf = PF_INET;
    hook_out.hooknum = NF_INET_LOCAL_OUT;
    hook_out.priority = NF_IP_PRI_FIRST;

    if (nf_register_net_hook(&init_net, &hook_out))
    {
        printk(KERN_WARNING "failed to register_net_hook hook_out\n");
        return -1;
    }

    printk(KERN_ALERT "register_net_hook ok\n");
    return 0;
}

static void modexit(void)
{
    nf_unregister_net_hook(&init_net, &hook_in);
    nf_unregister_net_hook(&init_net, &hook_out);
    printk(KERN_ALERT "unregister_net_hook ok\n");
}

module_init(modinit);
module_exit(modexit);
MODULE_LICENSE("GPL");
