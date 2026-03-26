# ── Stage 1: build ──────────────────────────────────────────────────────────
FROM mcr.microsoft.com/dotnet/sdk:9.0 AS build
WORKDIR /src

# Restore separately so layer is cached
COPY HPC/SovdServer/SovdServer.csproj ./SovdServer/
RUN dotnet restore ./SovdServer/SovdServer.csproj -r linux-x64

# Copy source and publish native AOT binary
COPY HPC/SovdServer/ ./SovdServer/
RUN dotnet publish ./SovdServer/SovdServer.csproj \
      -r linux-x64 -c Release \
      --no-restore \
      -o /publish

# ── Stage 2: runtime ────────────────────────────────────────────────────────
FROM ubuntu:22.04
RUN apt-get update && apt-get install -y --no-install-recommends \
      libssl3 ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /publish/SovdServer /app/SovdServer

ENV SOVD_GATEWAY_IMPL=dbus
ENV DBUS_SOCKET_PATH=/run/dbus/system_bus_socket
ENV ASPNETCORE_URLS=http://+:8080

EXPOSE 8080
ENTRYPOINT ["/app/SovdServer"]
