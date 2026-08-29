using Bms.Protocol;
using Bms.Transport;

namespace Bms.Client;

public sealed class BmsClient : IAsyncDisposable
{
    private readonly IBmsTransport _transport;
    private readonly List<byte> _receiveBuffer = new();
    private byte _sequence;
    public BmsClient(IBmsTransport transport)
    {
        _transport = transport;
        _transport.DataReceived += OnDataReceived;
    }
    public event EventHandler<BmsFrame>? FrameReceived;
    public bool IsConnected => _transport.IsConnected;
    public Task ConnectAsync(CancellationToken cancellationToken = default) => _transport.ConnectAsync(cancellationToken);
    public Task DisconnectAsync(CancellationToken cancellationToken = default) => _transport.DisconnectAsync(cancellationToken);
    public async Task<byte> SendAsync(byte messageType, ushort command, ReadOnlyMemory<byte> payload, CancellationToken cancellationToken = default)
    {
        byte sequence = unchecked(++_sequence);
        byte[] bytes = FrameCodec.Encode(new BmsFrame(messageType, sequence, command, payload.ToArray()));
        await _transport.WriteAsync(bytes, cancellationToken).ConfigureAwait(false);
        return sequence;
    }
    private void OnDataReceived(object? sender, byte[] data)
    {
        lock (_receiveBuffer)
        {
            _receiveBuffer.AddRange(data);
            while (_receiveBuffer.Count >= FrameCodec.HeaderSize + FrameCodec.CrcSize)
            {
                if (_receiveBuffer[0] != (byte)(FrameCodec.Magic & 0xFF) || _receiveBuffer[1] != (byte)(FrameCodec.Magic >> 8)) { _receiveBuffer.RemoveAt(0); continue; }
                int payloadLength = _receiveBuffer[8] | (_receiveBuffer[9] << 8);
                int total = FrameCodec.HeaderSize + payloadLength + FrameCodec.CrcSize;
                if (payloadLength > FrameCodec.MaxPayload) { _receiveBuffer.RemoveAt(0); continue; }
                if (_receiveBuffer.Count < total) return;
                byte[] candidate = _receiveBuffer.GetRange(0, total).ToArray();
                _receiveBuffer.RemoveRange(0, total);
                if (FrameCodec.TryDecode(candidate, out BmsFrame? frame) == FrameDecodeResult.Ok && frame is not null) FrameReceived?.Invoke(this, frame);
            }
        }
    }
    public async ValueTask DisposeAsync()
    {
        _transport.DataReceived -= OnDataReceived;
        await _transport.DisposeAsync().ConfigureAwait(false);
    }
}
