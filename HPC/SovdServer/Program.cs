using SovdServer.Models;
using SovdServer.Gateway;
using SovdServer.Logging;
using Microsoft.AspNetCore.Http.Json;
using System.Text;
using System.Text.Json;

var builder = WebApplication.CreateSlimBuilder(args);

// ── Broadcast logger (SSE on /api/v1/logs/stream) ─────────────────────
var broadcaster = new LogBroadcaster();
builder.Services.AddSingleton(broadcaster);
builder.Logging.ClearProviders();
builder.Logging.AddProvider(new BroadcastLoggerProvider(broadcaster));
// Suppress noisy framework logs on the console only – broadcaster sees everything at Info+
builder.Logging.AddConsole();
builder.Logging.AddFilter<Microsoft.Extensions.Logging.Console.ConsoleLoggerProvider>("Microsoft", LogLevel.Warning);
builder.Logging.AddFilter<Microsoft.Extensions.Logging.Console.ConsoleLoggerProvider>("Microsoft.AspNetCore.Routing", LogLevel.None);
builder.Logging.AddFilter<Microsoft.Extensions.Logging.Console.ConsoleLoggerProvider>("Microsoft.AspNetCore.Hosting", LogLevel.None);

// AOT-compatible JSON serialisation
builder.Services.Configure<JsonOptions>(opt =>
    opt.SerializerOptions.TypeInfoResolverChain.Add(SovdSerializationContext.Default));

builder.Services.AddSingleton<IEcuGateway, DbusEcuGateway>();

// ──────────────────────────────────────────────
//  Component registry  (SOVD discovery)
// ──────────────────────────────────────────────
var knownComponents = new List<SovdComponent>
{
    new("zone-controller", "Zone Controller", "Door ECU zone · SOME/IP diagnostic service"),
};
builder.Services.AddSingleton(knownComponents);

// ── CORS for local frontend dev
builder.Services.AddCors(opt =>
    opt.AddDefaultPolicy(p => p
        .WithOrigins("http://localhost:5174")
        .AllowAnyHeader()
        .AllowAnyMethod()));

var app = builder.Build();

app.UseCors();

// ── SSE log stream  GET /api/v1/logs/stream ───────────────────────────
app.MapGet("/api/v1/logs/stream", async (LogBroadcaster logs, HttpContext ctx, CancellationToken ct) =>
{
    ctx.Response.ContentType = "text/event-stream";
    ctx.Response.Headers.CacheControl = "no-cache";
    ctx.Response.Headers.Connection   = "keep-alive";

    var (history, channel) = logs.Subscribe();
    try
    {
        // Replay buffered history first
        foreach (var e in history)
        {
            await ctx.Response.WriteAsync(
                $"data: {JsonSerializer.Serialize(e, SovdSerializationContext.Default.LogEntry)}\n\n", ct);
        }
        await ctx.Response.Body.FlushAsync(ct);

        // Then stream live
        await foreach (var e in channel.Reader.ReadAllAsync(ct))
        {
            await ctx.Response.WriteAsync(
                $"data: {JsonSerializer.Serialize(e, SovdSerializationContext.Default.LogEntry)}\n\n", ct);
            await ctx.Response.Body.FlushAsync(ct);
        }
    }
    finally { logs.Release(channel); }
});

// ──────────────────────────────────────────────
//  SOVD discovery   /api/v1/components
// ──────────────────────────────────────────────
app.MapGet("/api/v1/components", (List<SovdComponent> components) =>
    Results.Ok(components));

// ──────────────────────────────────────────────
//  SOVD REST routes   /api/v1/ecu/{ecuId}/...
// ──────────────────────────────────────────────
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
