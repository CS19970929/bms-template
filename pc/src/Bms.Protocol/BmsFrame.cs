namespace Bms.Protocol;

public sealed record BmsFrame(byte MessageType, byte Sequence, ushort Command, byte[] Payload);

public enum FrameDecodeResult
{
    Ok = 0,
    TooShort,
    Magic,
    Version,
    Length,
    Crc
}
