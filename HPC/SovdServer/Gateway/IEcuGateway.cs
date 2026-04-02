using SovdServer.Models;

namespace SovdServer.Gateway;

public interface IEcuGateway
{
    Task<List<SovdCapability>> GetCapabilitiesAsync(string ecuId, CancellationToken ct = default);
    Task<List<SovdDataItem>> ReadDataAsync(string ecuId, CancellationToken ct = default);
    Task<SovdDataItem?> ReadDataItemAsync(string ecuId, string dataId, CancellationToken ct = default);
    Task<List<SovdFault>> GetFaultsAsync(string ecuId, CancellationToken ct = default);
    Task ClearFaultsAsync(string ecuId, CancellationToken ct = default);
    Task<List<SovdOperation>> GetOperationsAsync(string ecuId, CancellationToken ct = default);
    Task<SovdOperationResult> ExecuteOperationAsync(string ecuId, string operationId, SovdOperationRequest request, CancellationToken ct = default);
}
