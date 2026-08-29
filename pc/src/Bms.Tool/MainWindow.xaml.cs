using System.Windows;
using Bms.Client;
using Bms.Transport;

namespace Bms.Tool;

public partial class MainWindow : Window
{
    private BmsClient? _client;
    public MainWindow() { InitializeComponent(); RefreshPorts(); }
    private void RefreshPorts() { PortBox.ItemsSource = SerialTransport.GetPortNames(); if (PortBox.Items.Count > 0) PortBox.SelectedIndex = 0; }
    private void RefreshSerial_Click(object sender, RoutedEventArgs e) => RefreshPorts();
    private async void ConnectSerial_Click(object sender, RoutedEventArgs e)
    {
        if (PortBox.SelectedItem is not string port) { Append("Select a serial port."); return; }
        if (_client is not null) await _client.DisposeAsync();
        _client = new BmsClient(new SerialTransport(port));
        _client.FrameReceived += (_, frame) => Dispatcher.Invoke(() => Append($"RX cmd=0x{frame.Command:X4} seq={frame.Sequence} len={frame.Payload.Length}"));
        try { await _client.ConnectAsync(); Append($"Connected serial {port}"); }
        catch (Exception ex) { Append($"Serial error: {ex.Message}"); }
    }
    private async void ScanBle_Click(object sender, RoutedEventArgs e)
    {
        Append("Scanning BLE for 4 seconds...");
        try
        {
            IReadOnlyList<BleDeviceInfo> devices = await BleScanner.ScanAsync(TimeSpan.FromSeconds(4));
            foreach (BleDeviceInfo d in devices.Take(20)) Append($"BLE {d.AddressText} RSSI={d.Rssi} {d.Name}");
        }
        catch (Exception ex) { Append($"BLE scan error: {ex.Message}"); }
    }
    private void Append(string text) { LogBox.AppendText($"{DateTime.Now:HH:mm:ss} {text}{Environment.NewLine}"); LogBox.ScrollToEnd(); }
}
