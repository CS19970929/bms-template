using Windows.Devices.Bluetooth.Advertisement;

namespace Bms.Transport;

public static class BleScanner
{
    public static async Task<IReadOnlyList<BleDeviceInfo>> ScanAsync(TimeSpan duration, CancellationToken cancellationToken = default)
    {
        var found = new Dictionary<ulong, BleDeviceInfo>();
        using var watcher = new BluetoothLEAdvertisementWatcher { ScanningMode = BluetoothLEScanningMode.Active };
        watcher.Received += (_, args) =>
        {
            string name = args.Advertisement.LocalName ?? string.Empty;
            lock (found) found[args.BluetoothAddress] = new BleDeviceInfo(args.BluetoothAddress, name, args.RawSignalStrengthInDBm);
        };
        watcher.Start();
        try { await Task.Delay(duration, cancellationToken).ConfigureAwait(false); }
        finally { watcher.Stop(); }
        lock (found) return found.Values.OrderByDescending(x => x.Rssi).ToArray();
    }
}
