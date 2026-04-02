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

    private async Task<(int lockState, int windowPos, int mirrorAngle, int doorAjar)?> GetSensorsAsync(CancellationToken ct)
    {
        try
        {
            var json = await Http.GetStringAsync($"{BridgeBase}/ecu/door-ecu/sensors", ct);
            using var doc = JsonDocument.Parse(json);
            var r = doc.RootElement;
            return (
                r.GetProperty("lock_state").GetInt32(),
                r.GetProperty("window_pos").GetInt32(),
                r.GetProperty("mirror_angle").GetInt32(),
                r.GetProperty("door_ajar").GetInt32()
            );
        }
        catch { return null; }
    }

    private async Task WriteSensorFieldAsync(string field, int value, CancellationToken ct)
    {
        try
        {
            var body = new StringContent(
                $"{{\"value\":{value}}}",
                System.Text.Encoding.UTF8, "application/json");
            await Http.PostAsync($"{BridgeBase}/ecu/door-ecu/sensors/{field}", body, ct);
        }
        catch { }
    }

    public Task<List<SovdCapability>> GetCapabilitiesAsync(string ecuId, CancellationToken ct = default) =>
        Task.FromResult(IsKnownEcu(ecuId) ? Capabilities : []);

    public async Task<List<SovdDataItem>> ReadDataAsync(string ecuId, CancellationToken ct = default)
    {
        if (!IsKnownEcu(ecuId)) return [];
        var s = await GetSensorsAsync(ct);
        if (s is null) return DataItems;
        return
        [
            new("lock_state",   "Door Lock State",   s.Value.lockState   == 0 ? "unlocked" : "locked", ""),
            new("window_pos",   "Window Position",   s.Value.windowPos.ToString(), "%"),
            new("mirror_angle", "Mirror Angle",      s.Value.mirrorAngle.ToString(), "deg"),
            new("door_ajar",    "Door Ajar Sensor",  s.Value.doorAjar    == 0 ? "false" : "true", ""),
        ];
    }

    public async Task<SovdDataItem?> ReadDataItemAsync(string ecuId, string dataId, CancellationToken ct = default)
    {
        if (!IsKnownEcu(ecuId)) return null;
        var all = await ReadDataAsync(ecuId, ct);
        return all.FirstOrDefault(d => d.Id == dataId);
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

    public async Task<SovdOperationResult> ExecuteOperationAsync(
        string ecuId, string operationId, SovdOperationRequest request, CancellationToken ct = default)
    {
        if (!IsKnownEcu(ecuId))
            return new SovdOperationResult("error", "Unknown ECU", null);

        switch (operationId)
        {
            case "unlock_door":
                await WriteSensorFieldAsync("lock_state", 0, ct);
                return new SovdOperationResult("ok", "Door unlocked", null);

            case "reset_window":
                await WriteSensorFieldAsync("window_pos", 100, ct);
                return new SovdOperationResult("ok", "Window re-calibration started", null);

            default:
                return new SovdOperationResult("error", $"Unknown operation: {operationId}", null);
        }
    }
}


