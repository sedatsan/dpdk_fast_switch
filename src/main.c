#include <stdio.h>
#include <stdlib.h>
#include <rte_eal.h>
#include <rte_debug.h>
#include <rte_errno.h>
#include <rte_ethdev.h>
#include <rte_mbuf.h>
#include <rte_ether.h>
#include <inttypes.h>
#include "dpdk_fast_switch/mac_table.h"
#include "dpdk_fast_switch/ipc.h"

#define RX_RING_SIZE 1024
#define TX_RING_SIZE 1024
#define NUM_MBUFS 8191
#define MBUF_CACHE_SIZE 250
#define BURST_SIZE 32

static struct rte_hash *mac_table;
static uint64_t port_pps[RTE_MAX_ETHPORTS];
static uint64_t port_packets[RTE_MAX_ETHPORTS];

static const struct rte_eth_conf port_conf_default = {
    .rxmode = {
        .mq_mode = RTE_ETH_MQ_RX_NONE,
    },
};

static inline int port_init(uint16_t port, struct rte_mempool *mbuf_pool) {
    struct rte_eth_conf port_conf = port_conf_default;
    const uint16_t rx_rings = 1, tx_rings = 1;
    int retval;

    retval = rte_eth_dev_configure(port, rx_rings, tx_rings, &port_conf);
    if (retval != 0) return retval;

    retval = rte_eth_rx_queue_setup(port, 0, RX_RING_SIZE, rte_eth_dev_socket_id(port), NULL, mbuf_pool);
    if (retval < 0) return retval;

    retval = rte_eth_tx_queue_setup(port, 0, TX_RING_SIZE, rte_eth_dev_socket_id(port), NULL);
    if (retval < 0) return retval;

    retval = rte_eth_dev_start(port);
    if (retval < 0) return retval;

    rte_eth_promiscuous_enable(port);

    return 0;
}

int main(int argc, char **argv) {
    struct rte_mempool *mbuf_pool;
    uint16_t nb_ports;
    uint16_t portid;

    int ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error with EAL initialization: %s\n", rte_strerror(-ret));

    argc -= ret;
    argv += ret;

    nb_ports = rte_eth_dev_count_avail();
    if (nb_ports < 2)
        rte_exit(EXIT_FAILURE, "Error: number of ports must be at least 2\n");

    mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", NUM_MBUFS * nb_ports,
        MBUF_CACHE_SIZE, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());

    if (mbuf_pool == NULL)
        rte_exit(EXIT_FAILURE, "Cannot create mbuf pool\n");

    RTE_ETH_FOREACH_DEV(portid)
        if (port_init(portid, mbuf_pool) != 0)
            rte_exit(EXIT_FAILURE, "Cannot init port %"PRIu16 "\n", portid);

    mac_table = create_mac_table();
    if (mac_table == NULL)
        rte_exit(EXIT_FAILURE, "Cannot create MAC table\n");

    start_ipc_server();

    printf("DPDK Switch Initialized successfully with %u ports!\n", nb_ports);
    printf("Entering main loop. Press Ctrl+C to exit.\n");

    uint64_t prev_tsc = rte_rdtsc(), cur_tsc, timer_tsc = 0;
    uint64_t timer_period = rte_get_timer_hz(); // 1 second

    while (1) {
        cur_tsc = rte_rdtsc();
        timer_tsc += (cur_tsc - prev_tsc);
        prev_tsc = cur_tsc;

        if (unlikely(timer_tsc >= timer_period)) {
            char json[512];
            int offset = snprintf(json, sizeof(json), "{\"ports\": [");
            
            int first = 1;
            RTE_ETH_FOREACH_DEV(portid) {
                port_pps[portid] = port_packets[portid];
                port_packets[portid] = 0;
                
                int n = snprintf(json + offset, sizeof(json) - offset, 
                    "%s{\"id\": %u, \"pps\": %lu}", 
                    first ? "" : ",", portid, port_pps[portid]);
                
                if (n > 0) offset += n;
                first = 0;
            }
            snprintf(json + offset, sizeof(json) - offset, "], \"mac_table_entries\": %d}\n", 
                rte_hash_count(mac_table));
            push_stats(json);
            timer_tsc = 0;
        }

        RTE_ETH_FOREACH_DEV(portid) {
            struct rte_mbuf *bufs[BURST_SIZE];
            const uint16_t nb_rx = rte_eth_rx_burst(portid, 0, bufs, BURST_SIZE);

            if (unlikely(nb_rx == 0))
                continue;

            port_packets[portid] += nb_rx;

            for (uint16_t i = 0; i < nb_rx; i++) {
                struct rte_mbuf *m = bufs[i];
                struct rte_ether_hdr *eth_hdr = rte_pktmbuf_mtod(m, struct rte_ether_hdr *);

                update_mac_entry(mac_table, &eth_hdr->src_addr, portid);

                int dest_port = lookup_mac_entry(mac_table, &eth_hdr->dst_addr);

                if (dest_port >= 0 && (uint16_t)dest_port != portid) {
                    uint16_t nb_tx = rte_eth_tx_burst((uint16_t)dest_port, 0, &m, 1);
                    if (unlikely(nb_tx < 1)) {
                        rte_pktmbuf_free(m);
                    }
                } else {
                    uint16_t p;
                    RTE_ETH_FOREACH_DEV(p) {
                        if (p != portid) {
                            struct rte_mbuf *m_clone = rte_pktmbuf_clone(m, mbuf_pool);
                            if (m_clone) {
                                uint16_t nb_tx = rte_eth_tx_burst(p, 0, &m_clone, 1);
                                if (unlikely(nb_tx < 1)) {
                                    rte_pktmbuf_free(m_clone);
                                }
                            }
                        }
                    }
                    rte_pktmbuf_free(m);
                }
            }
        }
    }

    rte_eal_cleanup();
    return 0;
}
