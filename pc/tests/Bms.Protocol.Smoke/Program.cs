using Bms.Protocol;

byte[] payload = [1, 2, 3];
byte[] encoded = FrameCodec.Encode(new BmsFrame(1, 7, 0x1234, payload));
if (FrameCodec.TryDecode(encoded, out BmsFrame? decoded) != FrameDecodeResult.Ok || decoded is null) return 1;
if (decoded.Command != 0x1234 || decoded.Sequence != 7 || !decoded.Payload.SequenceEqual(payload)) return 2;
encoded[10] ^= 1;
if (FrameCodec.TryDecode(encoded, out _) != FrameDecodeResult.Crc) return 3;
Console.WriteLine("BMS_PC_PROTOCOL_SMOKE_PASS");
return 0;
