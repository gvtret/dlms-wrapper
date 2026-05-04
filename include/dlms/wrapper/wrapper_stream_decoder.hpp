#pragma once

#include "dlms/wrapper/wrapper_codec.hpp"
#include "dlms/wrapper/wrapper_error.hpp"
#include "dlms/wrapper/wrapper_frame.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace dlms {
namespace wrapper {

struct WrapperStreamDecoderOptions
{
  WrapperCodecLimits limits;
};

class WrapperStreamDecoder
{
public:
  explicit WrapperStreamDecoder(const WrapperStreamDecoderOptions& options);

  WrapperStatus Push(
    const std::uint8_t* data,
    std::size_t size,
    std::vector<WrapperFrameBuffer>& frames);

  void Reset();

private:
  WrapperStreamDecoderOptions options_;
  std::vector<std::uint8_t> buffer_;
};

} // namespace wrapper
} // namespace dlms
