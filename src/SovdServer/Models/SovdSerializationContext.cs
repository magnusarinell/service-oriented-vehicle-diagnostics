using System.Text.Json.Serialization;
using SovdServer.Models;

namespace SovdServer.Models;

[JsonSourceGenerationOptions(
    WriteIndented = false,
    PropertyNamingPolicy = JsonKnownNamingPolicy.CamelCase,
    DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull)]
[JsonSerializable(typeof(SovdCapability))]
[JsonSerializable(typeof(List<SovdCapability>))]
[JsonSerializable(typeof(SovdDataItem))]
[JsonSerializable(typeof(List<SovdDataItem>))]
[JsonSerializable(typeof(SovdFault))]
[JsonSerializable(typeof(List<SovdFault>))]
[JsonSerializable(typeof(SovdOperation))]
[JsonSerializable(typeof(List<SovdOperation>))]
[JsonSerializable(typeof(SovdOperationRequest))]
[JsonSerializable(typeof(SovdOperationResult))]
[JsonSerializable(typeof(Dictionary<string, string>))]
internal partial class SovdSerializationContext : JsonSerializerContext { }
