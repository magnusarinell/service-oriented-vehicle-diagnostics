using SovdServer.Models;
using System.Text.Json;

namespace SovdServer.Gateway;

/// <summary>
/// Development gateway that reads fault state from the renode-bridge HTTP API
/// so injected faults show up in the SOVD client panel.
/// </summary>
internal sealed class InMemoryEcuGateway : IEcuGateway
{
    private static readonly string BridgeBase =
        Environment.GetEnvironmentVariable("BRIDGE_URL") ?? "http://localhost:8787";

    private static readonly HttpClient Http = new() { Timeout = TimeSpan.FromSeconds(2) };

    private static readonly List<SovdCapability> Capabilities =
    [
        new("data",         "Read live door sensor data",      "read"),
        new("faults",       "Read and clear door fault codes", "read-write"),
        new("operations",   "Execute door control routines",   "execute"),
    ];

    private static readonly List<SovdDataItem> DataItems =
    [
        new("lock_state",    "Door Lock State",    "locked",  ""),
        new("window_pos",    "Window Position",    "100",     "%"),
        new("mirror_angle",  "Mirror Angle",       "0",       "deg"),
        new("door_ajar",     "Door Ajar Sensor",   "false",   ""),
    ];

    private static readonly List<SovdOperation> Operations =
    [
        new("unlock_door",   "Unlock Door",   "Send unlock command to door latch actuator"),
        new("reset_window",  "Reset Window",  "Run window motor re-calibration routine"),
    ];

    private static readonly Dictionary<int, (string code, string desc, string severity)> FaultDefs = new()
    {
        [0] = ("B1001", "Door Latch Sensor Fault",   "high"),
        [1] = ("B1002", "Window Motor Over-Current", "medium"),
        [2] = ("B1003", "CAN Bus Cable Fault",       "high"),
    };

    private bool IsKnownEcu(string ecuId) => ecuId == "door-ecu";

    private async Task<int> GetFaultFlagsAsync(CancellationToken ct)
    {
        try
        {
            var json = await Http.GetStringAsync(
                $"{BridgeBase}/ecu/door-ecu/fault-flags", ct);
            using var doc = JsonDocument.Parse(json);
            return doc.RootElement.GetProperty("faultFlags").GetInt32();
        }
        catch { return 0; }
    }

    public Task<List<SovdCapability>> GetCapabilitiesAsync(string ecuId, CancellationToken ct = default) =>
        Task.FromResult(IsKnownEcu(ecuId) ? Capabilities : []);

    public Task<List<SovdDataItem>> ReadDataAsync(string ecuId, CancellationToken ct = default) =>
        Task.FromResult(IsKnownEcu(ecuId) ? DataItems : []);

    public Task<SovdDataItem?> ReadDataItemAsync(string ecuId, string dataId, CancellationToken ct = default)
    {
        if (!IsKnownEcu(ecuId)) return Task.FromResult<SovdDataItem?>(null);
        return Task.FromResult(DataItems.FirstOrDefault(d => d.Id == dataId));
    }

    public async Task<List<SovdFault>> GetFaultsAsync(string ecuId, CancellationToken ct = default)
    {
        if (!IsKnownEcu(ecuId)) return [];
        var flags = await GetFaultFlagsAsync(ct);
        var result = new List<SovdFault>();
        foreach (var (bit, (code, desc, severity)) in FaultDefs)
            if ((flags & (1 << bit)) != 0)
                result.Add(new SovdFault(code, desc, severity, "active"));
        return result;
    }

    public async Task ClearFaultsAsync(string ecuId, CancellationToken ct = default)
    {
        if (!IsKnownEcu(ecuId)) return;
        try { await Http.PostAsync($"{BridgeBase}/restore", null, ct); } catch { }
    }

    public Task<List<SovdOperation>> GetOperationsAsync(string ecuId, CancellationToken ct = default) =>
        Task.FromResult(IsKnownEcu(ecuId) ? Operations : []);

    public Task<SovdOperationResult> ExecuteOperationAsync(
        string ecuId, string operationId, SovdOperationRequest request, CancellationToken ct = default)
    {
        if (!IsKnownEcu(ecuId))
            return Task.FromResult(new SovdOperationResult("error", "Unknown ECU", null));

        var result = operationId switch
        {
            "unlock_door"  => new SovdOperationResult("ok", "Door unlock command sent", null),
            "reset_window" => new SovdOperationResult("ok", "Window re-calibration started", null),
            _ => new SovdOperationResult("error", $"Unknown operation: {operationId}", null),
        };
        return Task.FromResult(result);
    }
}


