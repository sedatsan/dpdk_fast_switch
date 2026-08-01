# Stage 1: Build Environment
FROM ubuntu:22.04 AS builder

# Install DPDK dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libdpdk-dev \
    pkg-config \
    libnuma-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

# Build the project
RUN mkdir build && cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    cmake --build . -j$(nproc)

# Stage 2: Runtime Environment
FROM ubuntu:22.04

# Install DPDK runtime libraries
RUN apt-get update && apt-get install -y \
    libdpdk-dev \
    libnuma1 \
    pciutils \
    iproute2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy binary
COPY --from=builder /app/build/dpdk_fast_switch /usr/local/bin/dpdk_fast_switch

# DPDK requires hugepages and physical hardware access
# Must be run with --privileged and hardware volume mounts
ENTRYPOINT ["/usr/local/bin/dpdk_fast_switch"]
