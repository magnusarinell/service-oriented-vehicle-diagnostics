using SovdServer.Models;
using SovdServer.Gateway;
using Microsoft.AspNetCore.Http.Json;

var builder = WebApplication.CreateSlimBuilder(args);

// AOT-compatible JSON serialisation
builder.Services.Configure<JsonOptions>(opt =>
    opt.SerializerOptions.TypeInfoResolverChain.Add(SovdSerializationContext.Default));

// Gateway: choose via env var SOVD_GATEWAY_IMPL (inmemory | dbus)
var gatewayImpl = Environment.GetEnvironmentVariable("SOVD_GATEWAY_IMPL") ?? "inmemory";
if (gatewayImpl == "dbus")
    builder.Services.AddSingleton<IEcuGateway, DbusEcuGateway>();
else
    builder.Services.AddSingleton<IEcuGateway, InMemoryEcuGateway>();

// CORS for local frontend dev
builder.Services.AddCors(opt =>
    opt.AddDefaultPolicy(p => p
        .WithOrigins("http://localhost:5174")
        .AllowAnyHeader()
        .AllowAnyMethod()));

var app = builder.Build();

app.UseCors();

// ──────────────────────────────────────────────
//  SOVD REST routes   /api/v1/ecu/{ecuId}/...
// ──────────────────────────────────────────────
var ecu = app.MapGroup("/api/v1/ecu/{ecuId}");

// GET  /capabilities
ecu.MapGet("/capabilities", async (string ecuId, IEcuGateway gw, CancellationToken ct) =>
    Results.Ok(await gw.GetCapabilitiesAsync(ecuId, ct)));

// GET  /data
ecu.MapGet("/data", async (string ecuId, IEcuGateway gw, CancellationToken ct) =>
    Results.Ok(await gw.ReadDataAsync(ecuId, ct)));

// GET  /data/{dataId}
ecu.MapGet("/data/{dataId}", async (string ecuId, string dataId, IEcuGateway gw, CancellationToken ct) =>
{
    var item = await gw.ReadDataItemAsync(ecuId, dataId, ct);
    return item is null ? Results.NotFound() : Results.Ok(item);
});

// GET  /faults
ecu.MapGet("/faults", async (string ecuId, IEcuGateway gw, CancellationToken ct) =>
    Results.Ok(await gw.GetFaultsAsync(ecuId, ct)));

// DELETE  /faults
ecu.MapDelete("/faults", async (string ecuId, IEcuGateway gw, CancellationToken ct) =>
{
    await gw.ClearFaultsAsync(ecuId, ct);
    return Results.NoContent();
});

// GET  /operations
ecu.MapGet("/operations", async (string ecuId, IEcuGateway gw, CancellationToken ct) =>
    Results.Ok(await gw.GetOperationsAsync(ecuId, ct)));

// POST /operations/{opId}/execute
ecu.MapPost("/operations/{opId}/execute",
    async (string ecuId, string opId, SovdOperationRequest? body, IEcuGateway gw, CancellationToken ct) =>
{
    var result = await gw.ExecuteOperationAsync(ecuId, opId, body ?? new SovdOperationRequest(null), ct);
    return result.Status == "ok" ? Results.Ok(result) : Results.UnprocessableEntity(result);
});

app.Run();
