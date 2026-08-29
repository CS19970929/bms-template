using Windows.Devices.Bluetooth.Advertisement;
using Windows.Foundation;

namespace Bms.Transport;

public static class BleScanner
{
    public static async Task<IReadOnlyList<BleDeviceInfo>> ScanAsync(TimeSpan duration, CancellationToken cancellationToken = default)
    {
        var found = new Dictionary<ulong, BleDeviceInfo>();
        var watcher = new BluetoothLEAdvertisementWatcher { ScanningMode = BluetoothLEScanningMode.Active };
        TypedEventHandler<BluetoothLEAdvertisementWatcher, BluetoothLEAdvertisementReceivedEventArgs> handler = (_, args) =>
        {
            string name = args.Advertisement.LocalName ?? string.Empty;
            lock (found) found[args.BluetoothAddress] = new BleDeviceInfo(args.BluetoothAddress, name, args.RawSignalStrengthInDBm);
        };
        watcher.Received += handler;
        watcher.Start();
        try { await Task.Delay(duration, cancellationToken).ConfigureAwait(false); }
        finally
        {
            watcher.Stop();
            watcher.Received -= handler;
        }
        lock (found) return found.Values.OrderByDescending(x => x.Rssi).ToArray();
    }
}
