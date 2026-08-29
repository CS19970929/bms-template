using System.IO.Ports;

namespace Bms.Transport;

public sealed class SerialTransport : IBmsTransport
{
    private readonly SerialPort _port;
    public SerialTransport(string portName, int baudRate = 115200)
    {
        _port = new SerialPort(portName, baudRate, Parity.None, 8, StopBits.One) { ReadTimeout = 1000, WriteTimeout = 1000 };
        _port.DataReceived += OnDataReceived;
    }
    public bool IsConnected => _port.IsOpen;
    public event EventHandler<byte[]>? DataReceived;
    public Task ConnectAsync(CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (!_port.IsOpen) _port.Open();
        return Task.CompletedTask;
    }
    public Task DisconnectAsync(CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (_port.IsOpen) _port.Close();
        return Task.CompletedTask;
    }
    public async Task WriteAsync(ReadOnlyMemory<byte> data, CancellationToken cancellationToken = default)
    {
        if (!_port.IsOpen) throw new InvalidOperationException("Serial port is not connected.");
        await _port.BaseStream.WriteAsync(data, cancellationToken).ConfigureAwait(false);
        await _port.BaseStream.FlushAsync(cancellationToken).ConfigureAwait(false);
    }
    private void OnDataReceived(object sender, SerialDataReceivedEventArgs e)
    {
        int count = _port.BytesToRead;
        if (count <= 0) return;
        byte[] bytes = new byte[count];
        int read = _port.Read(bytes, 0, bytes.Length);
        if (read != bytes.Length) Array.Resize(ref bytes, read);
        DataReceived?.Invoke(this, bytes);
    }
    public async ValueTask DisposeAsync()
    {
        await DisconnectAsync().ConfigureAwait(false);
        _port.DataReceived -= OnDataReceived;
        _port.Dispose();
    }
    public static string[] GetPortNames() => SerialPort.GetPortNames().OrderBy(x => x, StringComparer.OrdinalIgnoreCase).ToArray();
}
