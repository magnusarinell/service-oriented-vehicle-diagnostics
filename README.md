# Service-Oriented Vehicle Diagnostics (SOVD)

A demonistration project implementing the [ASAM SOVD](https://www.asam.net/standards/detail/sovd/) diagnostic standard using modern automotive software stack: C# .NET 9 AOT, C++ SOME/IP via vsomeip, D-Bus via sdbus-c++, and a Vue 3 frontend — all running on Linux/Docker.

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│  Simulator/frontend  :5174/8090                             │  Vue 3 + TypeScript + Tailwind
│  DoorECU Visualizer ← WebSocket → Simulator/renode-bridge  │  WebSocket bridge → Renode XML-RPC
└──────────┬──────────────────────────────────────────────────┘
           │  HTTP/JSON  /api/v1/ecu/{ecuId}/...
┌──────────┴──────────────────┐
│  HPC / SovdServer           │  C# .NET 9 AOT, ASP.NET Core Minimal API
│  :8080                      │  IEcuGateway abstraction (inmemory | dbus)
└──────────┬──────────────────┘
           │  D-Bus  (Tmds.DBus.Protocol / unix socket)
┌──────────┴──────────────────┐
│  HPC / CDA  (C++)           │  Component Diagnostic Adapter
│                             │  sdbus-c++ adaptor + vsomeip client
└──────────┬──────────────────┘
           │  SOME/IP over TCP  (vsomeip 3.x, unicast)
┌──────────┴──────────────────┐
│  ZoneController  (C++)      │  SOME/IP service + UDSClient stub
│                             │  Routes requests to DoorECU via stubbed UDS
└──────────┬──────────────────┘
           │  UDS (stubbed — hardcoded responses)
┌──────────┴──────────────────┐
│  DoorECU  (NXP FRDM-K64F)  │  Baremetal C firmware
│  172.20.0.30:30509          │  LwIP SOME/IP server · Renode emulation
└─────────────────────────────┘
```

## Technologies

| Layer | Technology |
|---|---|
| REST API | C# .NET 9, ASP.NET Core Minimal API, Native AOT |
| D-Bus (C#) | [Tmds.DBus.Protocol](https://github.com/tmds/Tmds.DBus) |
| D-Bus (C++) | [sdbus-c++ v2](https://github.com/Kistler-Group/sdbus-cpp) |
| SOME/IP | [vsomeip 3.x (COVESA)](https://github.com/COVESA/vsomeip) |
| Frontend | Vue 3 + TypeScript + Vite + Tailwind CSS |
| Containers | Docker + Docker Compose |
| JSON (C++) | [nlohmann/json](https://github.com/nlohmann/json) |

## SOVD API Endpoints

All endpoints are prefixed with `/api/v1/ecu/{ecuId}`.

| Method | Path | Description |
|---|---|---|
| GET | `/capabilities` | List ECU capabilities |
| GET | `/data` | Read all live data items |
| GET | `/data/{dataId}` | Read a single data item |
| GET | `/faults` | Get active fault codes (DTCs) |
| DELETE | `/faults` | Clear all fault codes |
| GET | `/operations` | List available operations |
| POST | `/operations/{opId}/execute` | Execute a diagnostic operation |

Known ECU IDs: `ecu0`

## Getting Started

### Prerequisites

- [.NET 9 SDK](https://dotnet.microsoft.com/download/dotnet/9.0)
- [Node.js 20+](https://nodejs.org/)
- Docker Desktop (for the full stack — not required for local dev)

### Local Development (no Docker)

The SOVD Server uses an `InMemoryEcuGateway` by default, providing hardcoded diagnostic data without any C++ components or Docker.

```bash
# Install dependencies and start everything
npm install
npm run dev
```

This starts:
- **SOVD Server** on `http://localhost:8080` (C# dotnet run)
- **Frontend** on `http://localhost:5173` (Vite dev server with /api proxy)

Or start them individually:

```bash
# Terminal 1
npm run dev:server

# Terminal 2
npm run dev:frontend
```

The frontend proxies `/api` → `http://localhost:8080` automatically.

### Full Stack with Docker

```bash
docker compose up --build
```

Services:

| Service | Port | Description |
|---|---|---|
| `frontend` | `8090` | Vue 3 app (nginx) |
| `sovd-server` | — (internal) | .NET 9 AOT REST API (HPC/SovdServer) |
| `cda` | — (internal) | C++ Component Diagnostic Adapter (HPC/CDA, D-Bus + SOME/IP) |
| `zone-controller` | — (internal) | C++ ZoneController — SOME/IP service + UDSClient stub |
| `dbus-daemon` | — (internal) | System D-Bus daemon (shared socket volume) |

Open `http://localhost:8090` for the UI, or call the API directly:

```bash
curl http://localhost:8090/api/v1/ecu/ecu0/faults
curl -X DELETE http://localhost:8090/api/v1/ecu/ecu0/faults
curl -X POST http://localhost:8090/api/v1/ecu/ecu0/operations/reset/execute \
     -H 'Content-Type: application/json' -d '{}'
```

## Project Structure

```
├── HPC/
│   ├── SovdServer/              C# .NET 9 AOT REST API
│   │   ├── Program.cs           Minimal API routes + DI configuration
│   │   ├── Models/              SovdModels.cs, SovdSerializationContext.cs
│   │   └── Gateway/
│   │       ├── IEcuGateway.cs        Interface
│   │       ├── InMemoryEcuGateway.cs Local dev stub
│   │       └── DbusEcuGateway.cs     D-Bus client (Docker)
│   │
│   └── CDA/                     C++ Component Diagnostic Adapter
│       ├── CMakeLists.txt
│       ├── interfaces/cda-dbus.xml      D-Bus interface definition
│       └── src/
│           ├── GatewayDbusService.{h,cpp}   sdbus-c++ adaptor
│           ├── EcuSomeIpClient.{h,cpp}      vsomeip client
│           └── main.cpp
│
├── ZoneController/              C++ SOME/IP service (AUTOSAR ZoneController)
│   ├── CMakeLists.txt
│   ├── vsomeip/zone-controller.json  vsomeip configuration
│   └── src/
│       ├── DiagService.{h,cpp}   SOME/IP handlers + UDSClient stub
│       └── main.cpp
│
├── DoorECU/                     NXP FRDM-K64F baremetal C firmware
│   ├── CMakeLists.txt
│   ├── toolchain-k64f.cmake
│   └── src/
│       ├── diag_data.{h,c}      Door sensor data + fault flags (B-codes)
│       ├── someip.{h,c}         LwIP TCP SOME/IP server
│       └── main.c
│
├── Simulator/
│   ├── frontend/                Vite + Vue 3 + TypeScript
│   │   └── src/
│   │       ├── types/           sovd.ts, renode.ts (DoorEcuFaultBits, B-codes)
│   │       ├── composables/     useEcu.ts, useRenode.ts
│   │       ├── components/      EcuDashboard, FaultList, DataPanel,
│   │       │   └── door-ecu/    DoorEcuBoard.vue, DoorEcuVisualizer.vue
│   │       └── App.vue
│   │
│   └── renode-bridge/           Node.js Fastify WebSocket bridge
│       ├── index.mjs
│       ├── config.mjs           RENODE_MACHINE, FAULT_BITS, paths
│       └── lib/renode.mjs       Renode XML-RPC + .map file symbol lookup
│
├── renode/
│   ├── door-ecu.repl            K64F platform + EthernetBridge (tap0)
│   └── door-ecu.resc            Renode simulation launch script
│
├── docker/
│   ├── sovd-server.Dockerfile   Multi-stage: .NET SDK → ubuntu (AOT binary)
│   ├── cda.Dockerfile           Multi-stage: build vsomeip+sdbus-c++ from source
│   ├── zone-controller.Dockerfile  Multi-stage: build vsomeip from source
│   ├── frontend.Dockerfile      Multi-stage: npm build → nginx
│   ├── nginx.conf               /api proxy + SPA fallback
│   └── gateway-ecu-vsomeip.json vsomeip unicast config for Docker network
└── docker-compose.yml
```

## Gateway Abstraction

The `IEcuGateway` interface decouples the REST layer from the transport:

```
SOVD_GATEWAY_IMPL=inmemory   → InMemoryEcuGateway  (local dev, default)
SOVD_GATEWAY_IMPL=dbus       → DbusEcuGateway       (Docker, real stack)
```

Switch by setting the environment variable before starting the server.

## Simulated ECU Data (ecu0)

| Type | Data |
|---|---|
| Live data | lock_state (locked/unlocked), window_pos (0–100%), mirror_angle (°), door_ajar |
| Faults | B1001 (Door Latch), B1002 (Window Motor), B1003 (CAN Cable Fault) |
| Operations | `unlock_door`, `reset_window` |

## SOME/IP Service Identifiers

| Identifier | Value |
|---|---|
| Service ID | `0x1111` |
| Instance ID | `0x0001` |
| GetCapabilities | `0x0001` |
| ReadData | `0x0002` |
| ReadDataItem | `0x0003` |
| GetFaults | `0x0004` |
| ClearFaults | `0x0005` |
| GetOperations | `0x0006` |
| ExecuteOperation | `0x0007` |

## Phase 2: Renode + NXP K64F (DoorECU)

The DoorECU is emulated in [Renode](https://renode.io/) running a baremetal C firmware on an emulated NXP FRDM-K64F board:

- `DoorECU/` — Baremetal C firmware: LwIP NO_SYS=1 TCP/IP stack, SOME/IP request/response, B-code door fault flags
- `renode/door-ecu.repl` — K64F platform description with EthernetBridge on `tap0`
- `renode/door-ecu.resc` — Renode launch script; load with `npm run start:renode`
- `Simulator/renode-bridge/` — Node.js Fastify + WebSocket bridge to Renode XML-RPC (port 8787)
- **DoorECU Visualizer** panel in the frontend connects via WebSocket; click "Cut Cable" to set B1003, "Inject Fault" for B1001/B1002 — faults propagate through SOME/IP → ZoneController → CDA → SOVD Server → frontend

### Running Phase 2 locally (Linux host required for tap0)

```bash
# 1. Build DoorECU firmware
cd DoorECU && cmake -B build -DCMAKE_TOOLCHAIN_FILE=toolchain-k64f.cmake && cmake --build build

# 2. Set up tap interface
sudo ip tuntap add dev tap0 mode tap && sudo ip link set tap0 up
sudo ip addr add 172.20.0.1/16 dev tap0

# 3. Start Renode
npm run start:renode

# 4. Start the bridge (separate terminal)
npm run dev:bridge

# 5. Start the frontend
npm run dev:frontend
```
