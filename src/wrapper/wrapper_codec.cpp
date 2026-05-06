#include "dlms/wrapper/wrapper_codec.hpp"

#include "dlms/wrapper/wrapper_ports.hpp"

#include <vector>

namespace dlms {
namespace wrapper {

WrapperCodecLimits DefaultWrapperCodecLimits()
{
  WrapperCodecLimits limits;
  limits.maximumDataSize = kMaximumWrapperDataLength;
  limits.maximumFrameSize = kMaximumWrapperFrameLength;
  return limits;
}

namespace {

void WriteUint16(std::uint16_t value, std::uint8_t* output)
{
  output[0] = static_cast<std::uint8_t>((value >> 8) & 0xffu);
  output[1] = static_cast<std::uint8_t>(value & 0xffu);
}

std::uint16_t ReadUint16(const std::uint8_t* input)
{
  return static_cast<std::uint16_t>(
    (static_cast<std::uint16_t>(input[0]) << 8) |
    static_cast<std::uint16_t>(input[1]));
}

} // namespace

WrapperStatus EncodeWrapperHeader(
  const WrapperHeader& header,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize)
{
  writtenSize = 0;

  if (output == 0) {
    return WrapperStatus::InvalidArgument;
  }

  if (header.version != kWrapperVersion) {
    return WrapperStatus::InvalidVersion;
  }

  if (outputSize < kWrapperHeaderSize) {
    return WrapperStatus::OutputBufferTooSmall;
  }

  WriteUint16(header.version, output);
  WriteUint16(header.sourcePort, output + 2);
  WriteUint16(header.destinationPort, output + 4);
  WriteUint16(header.dataLength, output + 6);

  writtenSize = kWrapperHeaderSize;
  return WrapperStatus::Ok;
}

WrapperStatus DecodeWrapperHeader(
  const std::uint8_t* input,
  std::size_t inputSize,
  WrapperHeader& header)
{
  if (input == 0 && inputSize != 0) {
    return WrapperStatus::InvalidArgument;
  }

  if (inputSize < kWrapperHeaderSize) {
    return WrapperStatus::NeedMoreData;
  }

  header.version = ReadUint16(input);
  header.sourcePort = ReadUint16(input + 2);
  header.destinationPort = ReadUint16(input + 4);
  header.dataLength = ReadUint16(input + 6);

  if (header.version != kWrapperVersion) {
    return WrapperStatus::InvalidVersion;
  }

  return WrapperStatus::Ok;
}

WrapperStatus ValidateWrapperHeader(
  const WrapperHeader& header,
  const WrapperCodecLimits& limits,
  std::size_t availableDataSize)
{
  if (header.version != kWrapperVersion) {
    return WrapperStatus::InvalidVersion;
  }

  if (header.dataLength > limits.maximumDataSize) {
    return WrapperStatus::DataTooLarge;
  }

  if (kWrapperHeaderSize + header.dataLength > limits.maximumFrameSize) {
    return WrapperStatus::FrameTooLarge;
  }

  if (availableDataSize < header.dataLength) {
    return WrapperStatus::NeedMoreData;
  }

  if (availableDataSize > header.dataLength) {
    return WrapperStatus::InvalidLength;
  }

  return WrapperStatus::Ok;
}

WrapperStatus EncodeWpduToBuffer(
  const WrapperFrame& frame,
  const WrapperCodecLimits& limits,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize)
{
  writtenSize = 0;

  if (output == 0) {
    return WrapperStatus::InvalidArgument;
  }

  if (frame.data == 0 && frame.dataSize != 0) {
    return WrapperStatus::InvalidArgument;
  }

  if (IsNoStationWrapperPort(frame.sourcePort)) {
    return WrapperStatus::InvalidSourcePort;
  }

  if (IsNoStationWrapperPort(frame.destinationPort)) {
    return WrapperStatus::InvalidDestinationPort;
  }

  if (frame.dataSize > kMaximumWrapperDataLength ||
      frame.dataSize > limits.maximumDataSize) {
    return WrapperStatus::DataTooLarge;
  }

  const std::size_t frameSize = kWrapperHeaderSize + frame.dataSize;
  if (frameSize > limits.maximumFrameSize) {
    return WrapperStatus::FrameTooLarge;
  }

  if (outputSize < frameSize) {
    return WrapperStatus::OutputBufferTooSmall;
  }

  WrapperHeader header;
  header.version = kWrapperVersion;
  header.sourcePort = frame.sourcePort;
  header.destinationPort = frame.destinationPort;
  header.dataLength = static_cast<std::uint16_t>(frame.dataSize);

  std::size_t headerSize = 0;
  const WrapperStatus headerStatus =
    EncodeWrapperHeader(header, output, outputSize, headerSize);
  if (headerStatus != WrapperStatus::Ok) {
    return headerStatus;
  }

  for (std::size_t i = 0; i < frame.dataSize; ++i) {
    output[kWrapperHeaderSize + i] = frame.data[i];
  }

  writtenSize = frameSize;
  return WrapperStatus::Ok;
}

WrapperStatus EncodeWpdu(
  const WrapperFrame& frame,
  const WrapperCodecLimits& limits,
  std::vector<std::uint8_t>& output)
{
  output.clear();

  if (frame.dataSize > kMaximumWrapperDataLength ||
      frame.dataSize > limits.maximumDataSize) {
    return WrapperStatus::DataTooLarge;
  }

  const std::size_t frameSize = kWrapperHeaderSize + frame.dataSize;
  if (frameSize > limits.maximumFrameSize) {
    return WrapperStatus::FrameTooLarge;
  }

  try {
    output.resize(frameSize);
  } catch (...) {
    output.clear();
    return WrapperStatus::InternalError;
  }

  std::size_t writtenSize = 0;
  const WrapperStatus status =
    EncodeWpduToBuffer(frame, limits, &output[0], output.size(), writtenSize);
  if (status != WrapperStatus::Ok) {
    output.clear();
    return status;
  }

  output.resize(writtenSize);
  return WrapperStatus::Ok;
}

WrapperStatus DecodeWpduView(
  const std::uint8_t* input,
  std::size_t inputSize,
  const WrapperCodecLimits& limits,
  WrapperFrame& output)
{
  output.sourcePort = 0;
  output.destinationPort = 0;
  output.data = 0;
  output.dataSize = 0;

  WrapperHeader header;
  const WrapperStatus headerStatus =
    DecodeWrapperHeader(input, inputSize, header);
  if (headerStatus != WrapperStatus::Ok) {
    return headerStatus;
  }

  const std::size_t availableDataSize = inputSize - kWrapperHeaderSize;
  const WrapperStatus validationStatus =
    ValidateWrapperHeader(header, limits, availableDataSize);
  if (validationStatus != WrapperStatus::Ok) {
    return validationStatus;
  }

  if (IsNoStationWrapperPort(header.sourcePort)) {
    return WrapperStatus::InvalidSourcePort;
  }

  if (IsNoStationWrapperPort(header.destinationPort)) {
    return WrapperStatus::InvalidDestinationPort;
  }

  output.sourcePort = header.sourcePort;
  output.destinationPort = header.destinationPort;
  output.dataSize = header.dataLength;
  output.data = header.dataLength == 0 ? 0 : input + kWrapperHeaderSize;
  return WrapperStatus::Ok;
}

WrapperStatus DecodeWpdu(
  const std::uint8_t* input,
  std::size_t inputSize,
  const WrapperCodecLimits& limits,
  WrapperFrameBuffer& output)
{
  output.sourcePort = 0;
  output.destinationPort = 0;
  output.data.clear();

  WrapperFrame view;
  const WrapperStatus status = DecodeWpduView(input, inputSize, limits, view);
  if (status != WrapperStatus::Ok) {
    return status;
  }

  try {
    if (view.dataSize == 0) {
      output.data.clear();
    } else {
      output.data.assign(view.data, view.data + view.dataSize);
    }
  } catch (...) {
    output.sourcePort = 0;
    output.destinationPort = 0;
    output.data.clear();
    return WrapperStatus::InternalError;
  }

  output.sourcePort = view.sourcePort;
  output.destinationPort = view.destinationPort;
  return WrapperStatus::Ok;
}

} // namespace wrapper
} // namespace dlms
