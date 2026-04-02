# Multi-stage build for SomeIpGateway (C++ vsomeip + sdbus-c++)
FROM ubuntu:22.04 AS build
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake git \
      libboost-system-dev libboost-thread-dev \
      libdbus-1-dev libsystemd-dev \
      pkg-config \
    && rm -rf /var/lib/apt/lists/*

# Build vsomeip
RUN git clone --depth 1 --branch 3.4.10 \
      https://github.com/COVESA/vsomeip.git /opt/vsomeip-src
RUN cmake -S /opt/vsomeip-src -B /opt/vsomeip-build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DENABLE_SIGNAL_HANDLING=1 \
    && cmake --build /opt/vsomeip-build -j$(nproc) \
    && cmake --install /opt/vsomeip-build

# Build sdbus-c++ v2
RUN git clone --depth 1 --branch 2.1.0 \
      https://github.com/Kistler-Group/sdbus-cpp.git /opt/sdbus-src
RUN cmake -S /opt/sdbus-src -B /opt/sdbus-build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DSDBUSCPP_BUILD_CODEGEN=ON \
    && cmake --build /opt/sdbus-build -j$(nproc) \
    && cmake --install /opt/sdbus-build

WORKDIR /app
COPY HPC/SomeIpGateway/ .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc)

# ── Runtime ──────────────────────────────────────────────────────────────────
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      libboost-system1.74.0 libboost-thread1.74.0 \
      libdbus-1-3 libsystemd0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /usr/local/lib/libvsomeip3*.so*   /usr/local/lib/
COPY --from=build /usr/local/lib/libsdbus-c++*.so*  /usr/local/lib/
COPY --from=build /app/build/someip-gateway         /app/someip-gateway

RUN ldconfig

ENV VSOMEIP_CONFIGURATION=/etc/vsomeip/vsomeip.json
ENTRYPOINT ["/app/someip-gateway"]
