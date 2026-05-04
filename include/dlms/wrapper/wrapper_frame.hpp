#pragma once

#include <cstdint>
#include <vector>

namespace dlms {
namespace wrapper {

struct WrapperHeader
{
  std::uint16_t version;
  std::uint16_t sourcePort;
  std::uint16_t destinationPort;
  std::uint16_t dataLength;
};

struct WrapperFrame
{
  std::uint16_t sourcePort;
  std::uint16_t destinationPort;
  const std::uint8_t* data;
  std::size_t dataSize;
};

struct WrapperFrameBuffer
{
  std::uint16_t sourcePort;
  std::uint16_t destinationPort;
  std::vector<std::uint8_t> data;
};

} // namespace wrapper
} // namespace dlms
