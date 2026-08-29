using System.Buffers.Binary;

namespace Bms.Protocol;

public static class FrameCodec
{
    public const ushort Magic = 0xB54D;
    public const byte Version = 1;
    public const int HeaderSize = 10;
    public const int CrcSize = 4;
    public const int MaxPayload = 512;

    public static byte[] Encode(BmsFrame frame)
    {
        ArgumentNullException.ThrowIfNull(frame);
        if (frame.Payload.Length > MaxPayload) throw new ArgumentOutOfRangeException(nameof(frame), "Payload is too large.");
        byte[] buffer = new byte[HeaderSize + frame.Payload.Length + CrcSize];
        BinaryPrimitives.WriteUInt16LittleEndian(buffer.AsSpan(0, 2), Magic);
        buffer[2] = Version;
        buffer[3] = frame.MessageType;
        buffer[4] = frame.Sequence;
        buffer[5] = 0;
        BinaryPrimitives.WriteUInt16LittleEndian(buffer.AsSpan(6, 2), frame.Command);
        BinaryPrimitives.WriteUInt16LittleEndian(buffer.AsSpan(8, 2), checked((ushort)frame.Payload.Length));
        frame.Payload.CopyTo(buffer, HeaderSize);
        uint crc = Crc32.Compute(buffer.AsSpan(0, HeaderSize + frame.Payload.Length));
        BinaryPrimitives.WriteUInt32LittleEndian(buffer.AsSpan(HeaderSize + frame.Payload.Length, CrcSize), crc);
        return buffer;
    }

    public static FrameDecodeResult TryDecode(ReadOnlySpan<byte> data, out BmsFrame? frame)
    {
        frame = null;
        if (data.Length < HeaderSize + CrcSize) return FrameDecodeResult.TooShort;
        if (BinaryPrimitives.ReadUInt16LittleEndian(data[..2]) != Magic) return FrameDecodeResult.Magic;
        if (data[2] != Version) return FrameDecodeResult.Version;
        ushort payloadLength = BinaryPrimitives.ReadUInt16LittleEndian(data.Slice(8, 2));
        if (payloadLength > MaxPayload) return FrameDecodeResult.Length;
        int expected = HeaderSize + payloadLength + CrcSize;
        if (data.Length != expected) return FrameDecodeResult.Length;
        uint actual = Crc32.Compute(data[..(HeaderSize + payloadLength)]);
        uint expectedCrc = BinaryPrimitives.ReadUInt32LittleEndian(data.Slice(HeaderSize + payloadLength, CrcSize));
        if (actual != expectedCrc) return FrameDecodeResult.Crc;
        frame = new BmsFrame(data[3], data[4], BinaryPrimitives.ReadUInt16LittleEndian(data.Slice(6, 2)), data.Slice(HeaderSize, payloadLength).ToArray());
        return FrameDecodeResult.Ok;
    }
}
