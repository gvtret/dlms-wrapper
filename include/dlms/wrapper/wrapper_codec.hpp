#pragma once

#include "dlms/wrapper/wrapper_error.hpp"
#include "dlms/wrapper/wrapper_frame.hpp"

#include <cstddef>
#include <cstdint>

namespace dlms {
namespace wrapper {

bool WrapperCodecSkeletonAvailable();

struct WrapperCodecLimits
{
  std::size_t maximumDataSize;
  std::size_t maximumFrameSize;
};

WrapperCodecLimits DefaultWrapperCodecLimits();

WrapperStatus EncodeWrapperHeader(
  const WrapperHeader& header,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);

WrapperStatus DecodeWrapperHeader(
  const std::uint8_t* input,
  std::size_t inputSize,
  WrapperHeader& header);

WrapperStatus ValidateWrapperHeader(
  const WrapperHeader& header,
  const WrapperCodecLimits& limits,
  std::size_t availableDataSize);

} // namespace wrapper
} // namespace dlms
