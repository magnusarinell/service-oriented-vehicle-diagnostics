# Service-Oriented Vehicle Diagnostics (SOVD)

A demonistration project implementing the [ASAM SOVD](https://www.asam.net/standards/detail/sovd/) diagnostic standard using modern automotive software stack: C# .NET 9 AOT, C++ SOME/IP via vsomeip, D-Bus via sdbus-c++, and a Vue 3 frontend — all running on Linux/Docker.

## Architecture

```
┌─────────────────────────────┐
│  Vue 3 Frontend  :5173/8090 │  Vite + TypeScript + Tailwind
└──────────┬──────────────────┘
           │  HTTP/JSON  /api/v1/ecu/{ecuId}/...
┌──────────┴──────────────────┐
│  SOVD Server  (C# .NET 9)   │  ASP.NET Core Minimal API, PublishAot=true
│  :8080                      │  IEcuGateway abstraction (inmemory | dbus)
└──────────┬──────────────────┘
           │  D-Bus  (Tmds.DBus.Protocol / unix socket)
┌──────────┴──────────────────┐
│  GatewayECU  (C++)          │  sdbus-c++ adaptor + vsomeip client
└──────────┬──────────────────┘
           │  SOME/IP over TCP  (vsomeip 3.x, unicast)
┌──────────┴──────────────────┐
│  ECU Simulator  (C++)       │  vsomeip service, hardcoded diagnostic data
└─────────────────────────────┘
   [Phase 2: Renode + NXP FRDM-K64F firmware]
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
| `sovd-server` | — (internal) | .NET 9 AOT REST API |
| `gateway-ecu` | — (internal) | C++ D-Bus + SOME/IP bridge |
| `ecu-simulator` | — (internal) | C++ vsomeip SOME/IP service |
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
├── src/
│   ├── SovdServer/              C# .NET 9 AOT
│   │   ├── Program.cs           Minimal API routes + DI configuration
│   │   ├── Models/              SovdModels.cs, SovdSerializationContext.cs
│   │   └── Gateway/
│   │       ├── IEcuGateway.cs        Interface
│   │       ├── InMemoryEcuGateway.cs Local dev stub
│   │       └── DbusEcuGateway.cs     D-Bus client (Docker)
│   │
│   ├── GatewayECU/              C++ D-Bus service + SOME/IP client
│   │   ├── CMakeLists.txt
│   │   ├── interfaces/gateway-ecu.xml   D-Bus interface definition
│   │   └── src/
│   │       ├── GatewayDbusService.{h,cpp}   sdbus-c++ adaptor
│   │       ├── EcuSomeIpClient.{h,cpp}      vsomeip client
│   │       └── main.cpp
│   │
│   ├── EcuSimulator/            C++ SOME/IP service (simulated ECU)
│   │   ├── CMakeLists.txt
│   │   ├── vsomeip/ecu-sim.json  vsomeip configuration
│   │   └── src/
│   │       ├── DiagService.{h,cpp}   Hardcoded diagnostic data
│   │       └── main.cpp
│   │
│   └── frontend/                Vite + Vue 3 + TypeScript
│       └── src/
│           ├── types/sovd.ts         SOVD TypeScript types
│           ├── composables/useEcu.ts Reactive fetch wrapper
│           ├── components/           FaultList, DataPanel, OperationPanel,
│           │                         CapabilityBadges, EcuDashboard
│           └── App.vue
│
├── docker/
│   ├── sovd-server.Dockerfile    Multi-stage: .NET SDK → ubuntu (AOT binary)
│   ├── gateway-ecu.Dockerfile    Multi-stage: build vsomeip+sdbus-c++ from source
│   ├── ecu-simulator.Dockerfile  Multi-stage: build vsomeip from source
│   ├── frontend.Dockerfile       Multi-stage: npm build → nginx
│   ├── nginx.conf                /api proxy + SPA fallback
│   └── gateway-ecu-vsomeip.json  vsomeip unicast config for Docker network
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
| Live data | Coolant temp 82°C, Battery 12.6V, Engine RPM 0, Speed 0 km/h |
| Faults | P0100 (MAF circuit, high severity), P0200 (Injector circuit, medium) |
| Operations | `reset` (ECU soft reset), `self_test` (internal diagnostics) |

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

## Phase 2: Renode + NXP K64F

The EcuSimulator can be replaced with firmware running in [Renode](https://renode.io/) on an emulated NXP FRDM-K64F board:

- Baremetal C firmware: LwIP TCP/IP stack, minimal SOME/IP request/response
- Renode `.resc` platform script with emulated Ethernet bridged to the Docker network
- Same SOME/IP service IDs — GatewayECU requires no changes
