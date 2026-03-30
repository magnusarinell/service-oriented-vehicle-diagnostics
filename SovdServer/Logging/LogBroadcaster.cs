using System.Collections.Concurrent;
using System.Text.Json.Serialization;
using System.Threading.Channels;

namespace SovdServer.Logging;

/// <summary>
/// Captures structured log entries and fans them out to SSE subscribers.
/// Also keeps a ring buffer of the last 200 lines for new connections.
/// </summary>
public sealed class LogBroadcaster
{
    private const int BufferSize = 200;

    private readonly Queue<LogEntry>  _buffer  = new();
    private readonly List<Channel<LogEntry>> _channels = [];
    private readonly Lock                    _lock     = new();

    public void Publish(LogEntry entry)
    {
        lock (_lock)
        {
            _buffer.Enqueue(entry);
            while (_buffer.Count > BufferSize) _buffer.Dequeue();
            foreach (var ch in _channels)
                ch.Writer.TryWrite(entry);
        }
    }

    /// <summary>
    /// Returns buffered history + live channel. Caller must call Release() when done.
    /// </summary>
    public (IReadOnlyList<LogEntry> history, Channel<LogEntry> live) Subscribe()
    {
        var ch = Channel.CreateBounded<LogEntry>(new BoundedChannelOptions(512)
        {
            FullMode = BoundedChannelFullMode.DropOldest,
            SingleReader = true,
        });
        lock (_lock)
        {
            _channels.Add(ch);
            return ([.. _buffer], ch);
        }
    }

    public void Release(Channel<LogEntry> ch)
    {
        lock (_lock) { _channels.Remove(ch); ch.Writer.TryComplete(); }
    }
}

public record LogEntry(
    [property: JsonPropertyName("ts")]    string Timestamp,
    [property: JsonPropertyName("level")] string Level,
    [property: JsonPropertyName("cat")]   string Category,
    [property: JsonPropertyName("msg")]   string Message
);
