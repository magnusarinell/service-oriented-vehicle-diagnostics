using Microsoft.Extensions.Logging;

namespace SovdServer.Logging;

[ProviderAlias("Broadcast")]
public sealed class BroadcastLoggerProvider(LogBroadcaster broadcaster) : ILoggerProvider
{
    public ILogger CreateLogger(string categoryName) =>
        new BroadcastLogger(categoryName, broadcaster);

    public void Dispose() { }
}

internal sealed class BroadcastLogger(string category, LogBroadcaster broadcaster) : ILogger
{
    // Strip the full namespace prefix for readability
    private static string ShortCategory(string cat)
    {
        var last = cat.LastIndexOf('.');
        return last >= 0 ? cat[(last + 1)..] : cat;
    }

    public IDisposable? BeginScope<TState>(TState state) where TState : notnull => null;
    public bool IsEnabled(LogLevel level) => level >= LogLevel.Information;

    public void Log<TState>(
        LogLevel level, EventId eventId, TState state,
        Exception? exception, Func<TState, Exception?, string> formatter)
    {
        if (!IsEnabled(level)) return;
        var msg = formatter(state, exception);
        if (string.IsNullOrWhiteSpace(msg)) return;

        broadcaster.Publish(new LogEntry(
            Timestamp: DateTime.Now.ToString("HH:mm:ss.fff"),
            Level:     level.ToString()[..4].ToUpperInvariant(),
            Category:  ShortCategory(category),
            Message:   msg
        ));
    }
}
