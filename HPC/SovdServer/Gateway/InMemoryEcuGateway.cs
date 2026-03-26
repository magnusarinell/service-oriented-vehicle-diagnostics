using SovdServer.Models;

namespace SovdServer.Gateway;

/// <summary>
/// In-memory stub for local development without Docker/D-Bus.
/// Simulates a single ECU (ecu0) with hardcoded diagnostic data.
/// </summary>
internal sealed class InMemoryEcuGateway : IEcuGateway
{
    private static readonly List<SovdCapability> Capabilities =
    [
        new("data",         "Read live sensor data",           "read"),
        new("faults",       "Read and clear fault codes",      "read-write"),
        new("operations",   "Execute diagnostic routines",     "execute"),
    ];

    private static readonly List<SovdDataItem> DataItems =
    [
        new("coolant_temp",   "Coolant Temperature",  "82",   "°C"),
        new("battery_voltage","Battery Voltage",       "12.6", "V"),
        new("engine_rpm",     "Engine RPM",            "0",    "rpm"),
        new("vehicle_speed",  "Vehicle Speed",         "0",    "km/h"),
    ];

    private static readonly List<SovdOperation> Operations =
    [
        new("reset",      "ECU Reset",       "Perform a soft reset of the ECU"),
        new("self_test",  "Self-Test",        "Run internal ECU self-diagnostics"),
    ];

    // Mutable fault store so clear-faults is testable
    private readonly Dictionary<string, List<SovdFault>> _faults = new()
    {
        ["ecu0"] =
        [
            new("P0100", "Mass or Volume Air Flow Circuit Malfunction", "high",   "active"),
            new("P0200", "Injector Circuit Malfunction",                "medium", "pending"),
        ]
    };

    private bool IsKnownEcu(string ecuId) => ecuId == "ecu0";

    public Task<List<SovdCapability>> GetCapabilitiesAsync(string ecuId, CancellationToken ct = default) =>
        IsKnownEcu(ecuId)
            ? Task.FromResult(Capabilities)
            : Task.FromResult(new List<SovdCapability>());

    public Task<List<SovdDataItem>> ReadDataAsync(string ecuId, CancellationToken ct = default) =>
        IsKnownEcu(ecuId)
            ? Task.FromResult(DataItems)
            : Task.FromResult(new List<SovdDataItem>());

    public Task<SovdDataItem?> ReadDataItemAsync(string ecuId, string dataId, CancellationToken ct = default)
    {
        if (!IsKnownEcu(ecuId)) return Task.FromResult<SovdDataItem?>(null);
        var item = DataItems.FirstOrDefault(d => d.Id == dataId);
        return Task.FromResult(item);
    }

    public Task<List<SovdFault>> GetFaultsAsync(string ecuId, CancellationToken ct = default)
    {
        _faults.TryGetValue(ecuId, out var faults);
        return Task.FromResult(faults ?? []);
    }

    public Task ClearFaultsAsync(string ecuId, CancellationToken ct = default)
    {
        _faults[ecuId] = [];
        return Task.CompletedTask;
    }

    public Task<List<SovdOperation>> GetOperationsAsync(string ecuId, CancellationToken ct = default) =>
        IsKnownEcu(ecuId)
            ? Task.FromResult(Operations)
            : Task.FromResult(new List<SovdOperation>());

    public Task<SovdOperationResult> ExecuteOperationAsync(
        string ecuId, string operationId, SovdOperationRequest request, CancellationToken ct = default)
    {
        if (!IsKnownEcu(ecuId))
            return Task.FromResult(new SovdOperationResult("error", "Unknown ECU", null));

        var result = operationId switch
        {
            "reset"     => new SovdOperationResult("ok", "ECU reset acknowledged", null),
            "self_test" => new SovdOperationResult("ok", "Self-test passed", new Dictionary<string, string> { ["result"] = "pass" }),
            _           => new SovdOperationResult("error", $"Unknown operation: {operationId}", null),
        };
        return Task.FromResult(result);
    }
}
