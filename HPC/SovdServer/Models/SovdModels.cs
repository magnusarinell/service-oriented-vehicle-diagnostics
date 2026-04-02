namespace SovdServer.Models;

public record SovdCapability(
    string Name,
    string Description,
    string Category
);

public record SovdDataItem(
    string Id,
    string Name,
    string Value,
    string Unit
);

public record SovdFault(
    string Code,
    string Description,
    string Severity,
    string Status
);

public record SovdOperation(
    string Id,
    string Name,
    string Description
);

public record SovdOperationRequest(
    Dictionary<string, string>? Parameters
);

public record SovdOperationResult(
    string Status,
    string? Message,
    Dictionary<string, string>? OutputParameters
);

public record SovdComponent(
    string Id,
    string Name,
    string Description
);
