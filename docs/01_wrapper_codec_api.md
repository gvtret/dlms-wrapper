# 01. WRAPPER Codec API

## Public Model

```cpp
struct WrapperFrame
{
  std::uint16_t sourcePort;
  std::uint16_t destinationPort;
  const std::uint8_t* data;
  std::size_t dataSize;
};
```

`WrapperFrame` is a non-owning view used for encoding.

```cpp
struct WrapperFrameBuffer
{
  std::uint16_t sourcePort;
  std::uint16_t destinationPort;
  std::vector<std::uint8_t> data;
};
```

`WrapperFrameBuffer` owns decoded DATA bytes.

## Limits

```cpp
struct WrapperCodecLimits
{
  std::size_t maximumDataSize;
  std::size_t maximumFrameSize;
};
```

`DefaultWrapperCodecLimits()` returns limits compatible with the 16-bit
WRAPPER Data length field.

## Encode API

Convenience API:

```cpp
WrapperStatus EncodeWpdu(
  const WrapperFrame& frame,
  const WrapperCodecLimits& limits,
  std::vector<std::uint8_t>& output);
```

Strict caller-buffer API:

```cpp
WrapperStatus EncodeWpduToBuffer(
  const WrapperFrame& frame,
  const WrapperCodecLimits& limits,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);
```

## Decode API

Owning decode:

```cpp
WrapperStatus DecodeWpdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  const WrapperCodecLimits& limits,
  WrapperFrameBuffer& output);
```

View decode:

```cpp
WrapperStatus DecodeWpduView(
  const std::uint8_t* input,
  std::size_t inputSize,
  const WrapperCodecLimits& limits,
  WrapperFrame& output);
```

The decoder requires the input to contain exactly one complete WPDU unless a
future API explicitly states that trailing datagram bytes are allowed.
