#include "dpdk_fast_switch/mac_table.h"
#include <rte_cycles.h>
#include <rte_malloc.h>

struct rte_hash* create_mac_table(void) {
    struct rte_hash_parameters params = {
        .name = "mac_table",
        .entries = 1024,
        .key_len = sizeof(struct rte_ether_addr),
        .hash_func = rte_jhash,
        .hash_func_init_val = 0,
        .socket_id = rte_socket_id(),
    };
    struct rte_hash *h = rte_hash_create(&params);
    if (h == NULL) {
        fprintf(stderr, "Unable to create hash table\n");
    }
    return h;
}

int update_mac_entry(struct rte_hash *h, struct rte_ether_addr *addr, uint16_t port) {
    if (unlikely(h == NULL || addr == NULL)) return -EINVAL;
    
    int ret = rte_hash_add_key_data(h, addr, (void*)(uintptr_t)port);
    if (unlikely(ret < 0)) {
        // Handle full table or other errors
        return ret;
    }
    return 0;
}

int lookup_mac_entry(struct rte_hash *h, struct rte_ether_addr *addr) {
    void *data;
    int ret = rte_hash_lookup_data(h, addr, &data);
    if (ret < 0) return -1;
    return (int)(uintptr_t)data;
}
