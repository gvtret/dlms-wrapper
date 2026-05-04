#include "dlms/wrapper/wrapper_stream_decoder.hpp"

#include "dlms/wrapper/wrapper_ports.hpp"

namespace dlms {
namespace wrapper {

WrapperStreamDecoder::WrapperStreamDecoder(
  const WrapperStreamDecoderOptions& options)
  : options_(options)
{
}

WrapperStatus WrapperStreamDecoder::Push(
  const std::uint8_t* data,
  std::size_t size,
  std::vector<WrapperFrameBuffer>& frames)
{
  frames.clear();

  if (data == 0 && size != 0) {
    return WrapperStatus::InvalidArgument;
  }

  try {
    if (size != 0) {
      buffer_.insert(buffer_.end(), data, data + size);
    }
  } catch (...) {
    Reset();
    return WrapperStatus::InternalError;
  }

  while (buffer_.size() >= kWrapperHeaderSize) {
    WrapperHeader header;
    WrapperStatus status =
      DecodeWrapperHeader(&buffer_[0], buffer_.size(), header);
    if (status != WrapperStatus::Ok) {
      Reset();
      return status;
    }

    status = ValidateWrapperHeader(
      header,
      options_.limits,
      buffer_.size() - kWrapperHeaderSize);
    if (status == WrapperStatus::NeedMoreData) {
      break;
    }
    if (status != WrapperStatus::Ok && status != WrapperStatus::InvalidLength) {
      Reset();
      return status;
    }

    const std::size_t frameSize = kWrapperHeaderSize + header.dataLength;
    if (buffer_.size() < frameSize) {
      break;
    }

    WrapperFrameBuffer frame;
    status = DecodeWpdu(
      &buffer_[0],
      frameSize,
      options_.limits,
      frame);
    if (status != WrapperStatus::Ok) {
      Reset();
      return status;
    }

    try {
      frames.push_back(frame);
      buffer_.erase(buffer_.begin(), buffer_.begin() + frameSize);
    } catch (...) {
      Reset();
      frames.clear();
      return WrapperStatus::InternalError;
    }
  }

  return frames.empty() ? WrapperStatus::NeedMoreData : WrapperStatus::Ok;
}

void WrapperStreamDecoder::Reset()
{
  buffer_.clear();
}

} // namespace wrapper
} // namespace dlms
