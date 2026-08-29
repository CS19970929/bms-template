namespace Bms.Transport;

public sealed record BleDeviceInfo(ulong Address, string Name, short Rssi)
{
    public string AddressText => Address.ToString("X12");
}
