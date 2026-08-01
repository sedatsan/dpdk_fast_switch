#ifndef MAC_TABLE_H
#define MAC_TABLE_H

#include <rte_ether.h>
#include <rte_hash.h>
#include <rte_jhash.h>

struct mac_entry {
    struct rte_ether_addr addr;
    uint16_t port;
    uint64_t last_seen;
};

struct rte_hash* create_mac_table(void);
int update_mac_entry(struct rte_hash *h, struct rte_ether_addr *addr, uint16_t port);
int lookup_mac_entry(struct rte_hash *h, struct rte_ether_addr *addr);

#endif
