using Windows.Devices.Bluetooth;
using Windows.Devices.Bluetooth.GenericAttributeProfile;
using Windows.Storage.Streams;

namespace Bms.Transport;

public sealed class BleTransport : IBmsTransport
{
    private readonly ulong _address;
    private readonly Guid _serviceUuid;
    private readonly Guid _writeUuid;
    private readonly Guid _notifyUuid;
    private BluetoothLEDevice? _device;
    private GattDeviceService? _service;
    private GattCharacteristic? _write;
    private GattCharacteristic? _notify;

    public BleTransport(ulong address, Guid serviceUuid, Guid writeUuid, Guid notifyUuid)
    {
        _address = address; _serviceUuid = serviceUuid; _writeUuid = writeUuid; _notifyUuid = notifyUuid;
    }
    public bool IsConnected => _device is not null && _write is not null && _notify is not null;
    public event EventHandler<byte[]>? DataReceived;

    public async Task ConnectAsync(CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        _device = await BluetoothLEDevice.FromBluetoothAddressAsync(_address).AsTask(cancellationToken);
        if (_device is null) throw new InvalidOperationException("Unable to open BLE device.");
        var services = await _device.GetGattServicesForUuidAsync(_serviceUuid, BluetoothCacheMode.Uncached).AsTask(cancellationToken);
        if (services.Status != GattCommunicationStatus.Success || services.Services.Count == 0) throw new InvalidOperationException("BLE service not found.");
        _service = services.Services[0];
        var writes = await _service.GetCharacteristicsForUuidAsync(_writeUuid, BluetoothCacheMode.Uncached).AsTask(cancellationToken);
        var notifies = await _service.GetCharacteristicsForUuidAsync(_notifyUuid, BluetoothCacheMode.Uncached).AsTask(cancellationToken);
        if (writes.Status != GattCommunicationStatus.Success || writes.Characteristics.Count == 0) throw new InvalidOperationException("BLE write characteristic not found.");
        if (notifies.Status != GattCommunicationStatus.Success || notifies.Characteristics.Count == 0) throw new InvalidOperationException("BLE notify characteristic not found.");
        _write = writes.Characteristics[0]; _notify = notifies.Characteristics[0];
        _notify.ValueChanged += OnValueChanged;
        var status = await _notify.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.Notify).AsTask(cancellationToken);
        if (status != GattCommunicationStatus.Success) throw new InvalidOperationException($"BLE notify subscription failed: {status}");
    }

    public async Task DisconnectAsync(CancellationToken cancellationToken = default)
    {
        cancellationToken.ThrowIfCancellationRequested();
        if (_notify is not null)
        {
            _notify.ValueChanged -= OnValueChanged;
            try { await _notify.WriteClientCharacteristicConfigurationDescriptorAsync(GattClientCharacteristicConfigurationDescriptorValue.None).AsTask(cancellationToken); } catch { }
        }
        _notify = null; _write = null; _service?.Dispose(); _service = null; _device?.Dispose(); _device = null;
    }

    public async Task WriteAsync(ReadOnlyMemory<byte> data, CancellationToken cancellationToken = default)
    {
        if (_write is null) throw new InvalidOperationException("BLE is not connected.");
        const int conservativeChunk = 20;
        for (int offset = 0; offset < data.Length; offset += conservativeChunk)
        {
            int count = Math.Min(conservativeChunk, data.Length - offset);
            using var writer = new DataWriter();
            writer.WriteBytes(data.Slice(offset, count).ToArray());
            GattCommunicationStatus status = await _write.WriteValueAsync(writer.DetachBuffer(), GattWriteOption.WriteWithoutResponse).AsTask(cancellationToken);
            if (status != GattCommunicationStatus.Success) throw new IOException($"BLE write failed: {status}");
        }
    }

    private void OnValueChanged(GattCharacteristic sender, GattValueChangedEventArgs args)
    {
        using var reader = DataReader.FromBuffer(args.CharacteristicValue);
        byte[] bytes = new byte[reader.UnconsumedBufferLength];
        reader.ReadBytes(bytes);
        DataReceived?.Invoke(this, bytes);
    }

    public async ValueTask DisposeAsync() => await DisconnectAsync().ConfigureAwait(false);
}
