#include "syshead.h"
#include "dst.h"
#include "ip.h"
#include "arp.h"

int dst_neigh_output(struct sk_buff *skb)
{
    struct iphdr *iphdr = ip_hdr(skb);
    struct netdev *netdev = skb->dev;
    struct rtentry *rt = skb->rt;
    uint32_t daddr = ntohl(iphdr->daddr);
    uint32_t saddr = ntohl(iphdr->saddr);

    uint8_t *dmac;
    int rc;

    if (rt->flags & RT_GATEWAY) {
        daddr = rt->gateway;
    }
    
    dmac = arp_get_hwaddr(daddr);
    
    if (dmac) {
        return netdev_transmit(skb, dmac, ETH_P_IP);
    } else {
        rc = arp_request(saddr, daddr, netdev);

        /* TODO: Get rid of this abomination. A simple solution is to have
         * the retransmission mechanism just do its thing. */
        while ((dmac = arp_get_hwaddr(daddr)) == NULL) {
            sleep(1);
        }

        return netdev_transmit(skb, dmac, ETH_P_IP);
    }
}
