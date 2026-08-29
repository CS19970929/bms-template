namespace Bms.Transport;

public interface IBmsTransport : IAsyncDisposable
{
    bool IsConnected { get; }
    event EventHandler<byte[]>? DataReceived;
    Task ConnectAsync(CancellationToken cancellationToken = default);
    Task DisconnectAsync(CancellationToken cancellationToken = default);
    Task WriteAsync(ReadOnlyMemory<byte> data, CancellationToken cancellationToken = default);
}
