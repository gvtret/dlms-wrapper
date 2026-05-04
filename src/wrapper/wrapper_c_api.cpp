#include "dlms/wrapper/wrapper_c_api.h"

#include "dlms/wrapper/wrapper_codec.hpp"
#include "dlms/wrapper/wrapper_stream_decoder.hpp"

struct dlms_wrapper_stream_decoder_t
{
  explicit dlms_wrapper_stream_decoder_t(
    const dlms::wrapper::WrapperStreamDecoderOptions& options)
    : decoder(options)
  {
  }

  dlms::wrapper::WrapperStreamDecoder decoder;
};

namespace {

dlms_wrapper_status_t ToCApiStatus(dlms::wrapper::WrapperStatus status)
{
  switch (status) {
  case dlms::wrapper::WrapperStatus::Ok:
    return DLMS_WRAPPER_STATUS_OK;
  case dlms::wrapper::WrapperStatus::NeedMoreData:
    return DLMS_WRAPPER_STATUS_NEED_MORE_DATA;
  case dlms::wrapper::WrapperStatus::OutputBufferTooSmall:
    return DLMS_WRAPPER_STATUS_OUTPUT_BUFFER_TOO_SMALL;
  case dlms::wrapper::WrapperStatus::InvalidArgument:
    return DLMS_WRAPPER_STATUS_INVALID_ARGUMENT;
  case dlms::wrapper::WrapperStatus::InvalidVersion:
    return DLMS_WRAPPER_STATUS_INVALID_VERSION;
  case dlms::wrapper::WrapperStatus::InvalidLength:
    return DLMS_WRAPPER_STATUS_INVALID_LENGTH;
  case dlms::wrapper::WrapperStatus::InvalidSourcePort:
    return DLMS_WRAPPER_STATUS_INVALID_SOURCE_PORT;
  case dlms::wrapper::WrapperStatus::InvalidDestinationPort:
    return DLMS_WRAPPER_STATUS_INVALID_DESTINATION_PORT;
  case dlms::wrapper::WrapperStatus::DataTooLarge:
    return DLMS_WRAPPER_STATUS_DATA_TOO_LARGE;
  case dlms::wrapper::WrapperStatus::FrameTooLarge:
    return DLMS_WRAPPER_STATUS_FRAME_TOO_LARGE;
  case dlms::wrapper::WrapperStatus::UnsupportedFeature:
    return DLMS_WRAPPER_STATUS_UNSUPPORTED_FEATURE;
  case dlms::wrapper::WrapperStatus::InternalError:
    return DLMS_WRAPPER_STATUS_INTERNAL_ERROR;
  }

  return DLMS_WRAPPER_STATUS_INTERNAL_ERROR;
}

} // namespace

extern "C" dlms_wrapper_status_t dlms_wrapper_encode_wpdu(
  uint16_t source_port,
  uint16_t destination_port,
  const uint8_t* data,
  size_t data_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size)
{
  if (written_size == 0) {
    return DLMS_WRAPPER_STATUS_INVALID_ARGUMENT;
  }

  *written_size = 0;

  dlms::wrapper::WrapperFrame frame;
  frame.sourcePort = source_port;
  frame.destinationPort = destination_port;
  frame.data = data;
  frame.dataSize = data_size;

  try {
    return ToCApiStatus(dlms::wrapper::EncodeWpduToBuffer(
      frame,
      dlms::wrapper::DefaultWrapperCodecLimits(),
      output,
      output_size,
      *written_size));
  } catch (...) {
    *written_size = 0;
    return DLMS_WRAPPER_STATUS_INTERNAL_ERROR;
  }
}

extern "C" dlms_wrapper_status_t dlms_wrapper_decode_wpdu(
  const uint8_t* input,
  size_t input_size,
  uint16_t* source_port,
  uint16_t* destination_port,
  uint8_t* data_output,
  size_t data_output_size,
  size_t* data_size)
{
  if (source_port == 0 || destination_port == 0 || data_size == 0) {
    return DLMS_WRAPPER_STATUS_INVALID_ARGUMENT;
  }

  *source_port = 0;
  *destination_port = 0;
  *data_size = 0;

  dlms::wrapper::WrapperFrame frame;
  const dlms::wrapper::WrapperStatus status = dlms::wrapper::DecodeWpduView(
    input,
    input_size,
    dlms::wrapper::DefaultWrapperCodecLimits(),
    frame);
  if (status != dlms::wrapper::WrapperStatus::Ok) {
    return ToCApiStatus(status);
  }

  *data_size = frame.dataSize;
  if (data_output_size < frame.dataSize) {
    return DLMS_WRAPPER_STATUS_OUTPUT_BUFFER_TOO_SMALL;
  }

  if (frame.dataSize != 0 && data_output == 0) {
    return DLMS_WRAPPER_STATUS_INVALID_ARGUMENT;
  }

  for (size_t i = 0; i < frame.dataSize; ++i) {
    data_output[i] = frame.data[i];
  }

  *source_port = frame.sourcePort;
  *destination_port = frame.destinationPort;
  return DLMS_WRAPPER_STATUS_OK;
}

extern "C" dlms_wrapper_status_t dlms_wrapper_stream_decoder_create(
  dlms_wrapper_stream_decoder_t** decoder)
{
  if (decoder == 0) {
    return DLMS_WRAPPER_STATUS_INVALID_ARGUMENT;
  }

  *decoder = 0;

  try {
    dlms::wrapper::WrapperStreamDecoderOptions options;
    options.limits = dlms::wrapper::DefaultWrapperCodecLimits();
    *decoder = new dlms_wrapper_stream_decoder_t(options);
  } catch (...) {
    *decoder = 0;
    return DLMS_WRAPPER_STATUS_INTERNAL_ERROR;
  }

  return DLMS_WRAPPER_STATUS_OK;
}

extern "C" void dlms_wrapper_stream_decoder_destroy(
  dlms_wrapper_stream_decoder_t* decoder)
{
  delete decoder;
}

extern "C" void dlms_wrapper_stream_decoder_reset(
  dlms_wrapper_stream_decoder_t* decoder)
{
  if (decoder != 0) {
    decoder->decoder.Reset();
  }
}
