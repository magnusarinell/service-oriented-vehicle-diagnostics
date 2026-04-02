# Plan: Native full-stack utan Docker

**TL;DR:** Bygg vsomeip + sdbus-c++ från källkod, konfigurera D-Bus-policy, skapa lokala vsomeip-configs med `127.0.0.1` istället för Docker-IP:er, bygg ZoneController + CDA, sätt upp tap0 för Renode.

---

## Fas 1 — Systemberoenden (apt + externt)

1. `sudo apt install build-essential cmake git pkg-config ninja-build libboost-all-dev libdbus-1-dev libsystemd-dev dbus gcc-arm-none-eabi`
2. **.NET 10 SDK** via Microsoft's apt-feed (Ubuntu 24.04 har bara .NET 8 i default apt)
3. **Node.js 20+** via NodeSource script (eller apt om 20+ finns)
4. **Renode** — ladda ned `.deb` från GitHub Releases v1.16.1

## Fas 2 — Bygg vsomeip från källkod (installeras till `/usr/local`)

5. `git clone --depth 1 --branch 3.4.10 https://github.com/COVESA/vsomeip.git`
6. cmake — libdlt saknas i Ubuntu 24.04 apt → vsomeip:s cmake auto-detekterar och skippar DLT, inget explicit flag behövs
7. `cmake --build` + `sudo cmake --install`

## Fas 3 — Bygg sdbus-c++ från källkod

8. `git clone --depth 1 --branch 2.1.0 https://github.com/Kistler-Group/sdbus-cpp.git`
9. cmake med `-DBUILD_CODE_GEN=ON` (behövs för xml2cpp-verktyget som genererar D-Bus-adaptor-headern vid `cmake -S HPC/CDA`)
10. `cmake --build` + `sudo cmake --install`

## Fas 4 — D-Bus policy

11. Skapa `/etc/dbus-1/system.d/com.vehicle.gateway.conf` — tillåter aktuell användare att äga `com.vehicle.gateway` på system-bussen *(kräver sudo, men bara en gång)*

## Fas 5 — Lokala vsomeip-konfigfiler

12. Skapa `ZoneController/vsomeip/zone-controller-local.json` — kopia av befintlig men `"unicast": "127.0.0.1"` istället för `172.20.0.10`
13. Skapa `docker/cda-vsomeip-local.json` — kopia av `cda-vsomeip.json` men `"unicast": "127.0.0.1"` *(CDA:s routing master pekar redan på port 30511)*

## Fas 6 — Bygg C++-komponenter

14. Bygg ZoneController: `cmake -S ZoneController -B ZoneController/build -DCMAKE_BUILD_TYPE=Release && cmake --build ZoneController/build`
15. Bygg CDA: `cmake -S HPC/CDA -B HPC/CDA/build -DCMAKE_BUILD_TYPE=Release && cmake --build HPC/CDA/build`
16. Bygg DoorECU-firmware: `cmake -S DoorECU -B DoorECU/build -DCMAKE_TOOLCHAIN_FILE=DoorECU/toolchain-k64f.cmake && cmake --build DoorECU/build`

## Fas 7 — tap0 för Renode

17. `sudo ip tuntap add dev tap0 mode tap && sudo ip link set tap0 up && sudo ip addr add 172.20.0.1/16 dev tap0`

## Fas 8 — Node.js-beroenden + köra stacken

18. `npm run deps` (kör install för alla tre Node-projekt)
19. Köra stacken i rätt ordning: ZoneController → CDA (med `VSOMEIP_CONFIGURATION`) → SovdServer (med `SOVD_GATEWAY_IMPL=dbus`) → Renode → Frontend

---

## Relevanta filer

- `ZoneController/vsomeip/zone-controller.json` — mall för lokal config
- `docker/cda-vsomeip.json` — mall för lokal CDA-config
- `HPC/CDA/src/main.cpp` — `createSystemBusConnection` → kräver system-bus policy
- `SovdServer/Gateway/DbusEcuGateway.cs` — läser `DBUS_SOCKET_PATH`

## Verifiering

1. `ldconfig -p | grep vsomeip` — bekräftar vsomeip installerat
2. `ldconfig -p | grep sdbus` — bekräftar sdbus-c++ installerat
3. ZoneController startar utan "service not available"-fel
4. `curl http://localhost:8080/api/v1/ecu/ecu0/capabilities` — SovdServer svarar via riktig D-Bus→SOME/IP-kedja

## Beslut / Avgränsningar

- Använder **system-bussen** (ingen kodändring i CDA krävs) — löses med D-Bus policy-fil
- vsomeip byggs **utan DLT** (auto-detect, Ubuntu 24.04 saknar libdlt-dev)
- Bygg-artefakter hamnar i respektive `*/build/`-katalog, inget installeras till systemet utöver vsomeip/sdbus-c++
