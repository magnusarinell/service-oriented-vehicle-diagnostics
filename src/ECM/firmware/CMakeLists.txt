cmake_minimum_required(VERSION 3.16)
project(ecm-firmware C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Cross-compile toolchain must be set via -DCMAKE_TOOLCHAIN_FILE=toolchain-k64f.cmake
# ── or toolchain-native.cmake for host-build tests.

set(LWIP_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lwip)

# LwIP sources (minimal: core + IPv4 + ethernet)
set(LWIP_SOURCES
    ${LWIP_DIR}/src/core/init.c
    ${LWIP_DIR}/src/core/mem.c
    ${LWIP_DIR}/src/core/memp.c
    ${LWIP_DIR}/src/core/netif.c
    ${LWIP_DIR}/src/core/pbuf.c
    ${LWIP_DIR}/src/core/raw.c
    ${LWIP_DIR}/src/core/stats.c
    ${LWIP_DIR}/src/core/sys.c
    ${LWIP_DIR}/src/core/tcp.c
    ${LWIP_DIR}/src/core/tcp_in.c
    ${LWIP_DIR}/src/core/tcp_out.c
    ${LWIP_DIR}/src/core/timeouts.c
    ${LWIP_DIR}/src/core/udp.c
    ${LWIP_DIR}/src/core/ipv4/autoip.c
    ${LWIP_DIR}/src/core/ipv4/dhcp.c
    ${LWIP_DIR}/src/core/ipv4/etharp.c
    ${LWIP_DIR}/src/core/ipv4/icmp.c
    ${LWIP_DIR}/src/core/ipv4/ip4.c
    ${LWIP_DIR}/src/core/ipv4/ip4_addr.c
    ${LWIP_DIR}/src/core/ipv4/ip4_frag.c
    ${LWIP_DIR}/src/netif/ethernet.c
)

add_executable(ecm-firmware
    src/main.c
    src/diag_data.c
    src/someip.c
    src/port/sys_arch.c
    src/port/ethernetif.c
    ${LWIP_SOURCES}
)

target_include_directories(ecm-firmware PRIVATE
    src
    src/port
    ${LWIP_DIR}/src/include
)

# Pull in lwipopts
target_include_directories(ecm-firmware PRIVATE src/port)

target_compile_definitions(ecm-firmware PRIVATE
    LWIP_DEBUG=0
)
