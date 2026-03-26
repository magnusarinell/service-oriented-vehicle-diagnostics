using SovdServer.Models;
using Tmds.DBus.Protocol;

namespace SovdServer.Gateway;

/// <summary>
/// D-Bus gateway that communicates with the CDA C++ service.
/// Requires the dbus-daemon container and CDA container to be running.
/// Activated when SOVD_GATEWAY_IMPL=dbus.
/// </summary>
internal sealed class DbusEcuGateway : IEcuGateway, IAsyncDisposable
{
    private const string ServiceName  = "com.vehicle.gateway";
    private const string ObjectPath   = "/com/vehicle/gateway";
    private const string InterfaceName = "com.vehicle.gateway.IDiagnosticManager";

    private readonly Connection _connection;

    public DbusEcuGateway()
    {
        var socketPath = Environment.GetEnvironmentVariable("DBUS_SOCKET_PATH")
            ?? "/run/dbus/system_bus_socket";
        _connection = new Connection($"unix:path={socketPath}");
    }

    public ValueTask DisposeAsync()
    {
        _connection.Dispose();
        return ValueTask.CompletedTask;
    }

    // All methods delegate to D-Bus calls. The CDA service serialises
    // its responses as JSON strings so we can deserialise here with source-gen context.

    public async Task<List<SovdCapability>> GetCapabilitiesAsync(string ecuId, CancellationToken ct = default)
    {
        var json = await CallMethodAsync("GetCapabilities", ecuId, ct);
        return System.Text.Json.JsonSerializer.Deserialize(json, SovdSerializationContext.Default.ListSovdCapability) ?? [];
    }

    public async Task<List<SovdDataItem>> ReadDataAsync(string ecuId, CancellationToken ct = default)
    {
        var json = await CallMethodAsync("ReadData", ecuId, ct);
        return System.Text.Json.JsonSerializer.Deserialize(json, SovdSerializationContext.Default.ListSovdDataItem) ?? [];
    }

    public async Task<SovdDataItem?> ReadDataItemAsync(string ecuId, string dataId, CancellationToken ct = default)
    {
        var json = await CallMethodAsync("ReadDataItem", $"{ecuId}:{dataId}", ct);
        return System.Text.Json.JsonSerializer.Deserialize(json, SovdSerializationContext.Default.SovdDataItem);
    }

    public async Task<List<SovdFault>> GetFaultsAsync(string ecuId, CancellationToken ct = default)
    {
        var json = await CallMethodAsync("GetFaults", ecuId, ct);
        return System.Text.Json.JsonSerializer.Deserialize(json, SovdSerializationContext.Default.ListSovdFault) ?? [];
    }

    public async Task ClearFaultsAsync(string ecuId, CancellationToken ct = default) =>
        await CallMethodAsync("ClearFaults", ecuId, ct);

    public async Task<List<SovdOperation>> GetOperationsAsync(string ecuId, CancellationToken ct = default)
    {
        var json = await CallMethodAsync("GetOperations", ecuId, ct);
        return System.Text.Json.JsonSerializer.Deserialize(json, SovdSerializationContext.Default.ListSovdOperation) ?? [];
    }

    public async Task<SovdOperationResult> ExecuteOperationAsync(
        string ecuId, string operationId, SovdOperationRequest request, CancellationToken ct = default)
    {
        var json = await CallMethodAsync("ExecuteOperation", $"{ecuId}:{operationId}", ct);
        return System.Text.Json.JsonSerializer.Deserialize(json, SovdSerializationContext.Default.SovdOperationResult)
            ?? new SovdOperationResult("error", "Empty response from gateway", null);
    }

    private async Task<string> CallMethodAsync(string methodName, string argument, CancellationToken ct)
    {
        await _connection.ConnectAsync();
        var message = CreateMethodCall(methodName, argument);
        var result = await _connection.CallMethodAsync(message,
            static (Message msg, object? _) => msg.GetBodyReader().ReadString());
        return result ?? "[]";
    }

    private MessageBuffer CreateMethodCall(string methodName, string argument)
    {
        var writer = _connection.GetMessageWriter();
        writer.WriteMethodCallHeader(
            destination: ServiceName,
            path:        ObjectPath,
            @interface:  InterfaceName,
            member:      methodName,
            signature:   "s");
        writer.WriteString(argument);
        return writer.CreateMessage();
    }
}
