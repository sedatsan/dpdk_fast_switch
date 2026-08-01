# DPDK Fast Switch
[![Agentic AI Ready](https://img.shields.io/badge/Agentic_AI_Ready-AGENTS.md-blue)](AGENTS.md)

> **Project Summary:**  
> A Layer 2 fast-path switch built in C11 using the Data Plane Development Kit (DPDK). It demonstrates deep understanding of `rte_mbuf` zero-copy lifecycle management and fast MAC address learning using `rte_hash`. It streams live port statistics via IPC and conforms to strict DPDK C standards with a Pitchfork layout and CMake toolchain.

## Table of Contents
1. [System Architecture](#system-architecture)
2. [Getting Started](#getting-started)
3. [Usage](#usage)

## System Architecture

```mermaid
graph LR
    A((Network Interface)) -->|rte_eth_rx_burst| B[RX Queue]
    B -->|Zero-copy rte_mbuf| C{MAC Table Lookup}
    C -->|Hit| D[TX Queue - Targeted]
    C -->|Miss| E[TX Queue - Flooded]
    D -->|rte_eth_tx_burst| F((Destination Interface))
    E -->|rte_eth_tx_burst| F
```

## Getting Started

### Prerequisites
- C11 compatible compiler
- DPDK Development Libraries (`libdpdk-dev`)
- CMake >= 3.14

### Build Instructions
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

### Docker (Cloud Native Network Function - CNF)
To run this switch as a containerized network function, build the provided Dockerfile:

```bash
docker build -t dpdk_fast_switch .
docker run --rm --privileged \
  -v /sys/bus/pci:/sys/bus/pci \
  -v /sys/kernel/mm/hugepages:/sys/kernel/mm/hugepages \
  dpdk_fast_switch -c 0x3 -n 4
```

## Usage
Requires `root` privileges to initialize DPDK EAL and allocate hugepages.
```bash
sudo ./dpdk_fast_switch -c 0x3 -n 4
```

## Agentic AI Development
This repository is fully compliant with the [AGENTS.md](https://agents.md) open standard. It includes strict, drop-in operating instructions designed to correctly guide autonomous AI coding agents (such as Cursor, Devin, Copilot, or Antigravity) across the C11 and DPDK constraints of this codebase. By providing explicit boundaries, the AI is prevented from hallucinating architectural decisions or making non-deterministic fast-path memory allocations.
