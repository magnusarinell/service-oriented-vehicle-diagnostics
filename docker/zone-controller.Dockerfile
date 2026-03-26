# Multi-stage build for ZoneController (C++ vsomeip SOME/IP service)
FROM ubuntu:22.04 AS build
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      build-essential cmake git \
      libboost-system-dev libboost-thread-dev \
      libdlt-dev \
    && rm -rf /var/lib/apt/lists/*

# Build vsomeip from source (COVESA)
RUN git clone --depth 1 --branch 3.4.10 \
      https://github.com/COVESA/vsomeip.git /opt/vsomeip-src
RUN cmake -S /opt/vsomeip-src -B /opt/vsomeip-build \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DENABLE_SIGNAL_HANDLING=1 \
    && cmake --build /opt/vsomeip-build -j$(nproc) \
    && cmake --install /opt/vsomeip-build

WORKDIR /app
COPY ZoneController/ .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j$(nproc)

# ── Runtime ──────────────────────────────────────────────────────────────────
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
      libboost-system1.74.0 libboost-thread1.74.0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /usr/local/lib/libvsomeip3*.so* /usr/local/lib/
COPY --from=build /app/build/zone-controller          /app/zone-controller
COPY --from=build /app/vsomeip/zone-controller.json   /etc/vsomeip/vsomeip.json

RUN ldconfig

ENV VSOMEIP_CONFIGURATION=/etc/vsomeip/vsomeip.json
ENV ZONE_CONTROLLER_UNICAST=172.20.0.10
ENTRYPOINT ["/app/zone-controller"]
