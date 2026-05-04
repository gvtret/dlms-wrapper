#ifndef DLMS_WRAPPER_WRAPPER_C_API_H
#define DLMS_WRAPPER_WRAPPER_C_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum dlms_wrapper_status_t
{
  DLMS_WRAPPER_STATUS_OK = 0,
  DLMS_WRAPPER_STATUS_NEED_MORE_DATA = 1,
  DLMS_WRAPPER_STATUS_OUTPUT_BUFFER_TOO_SMALL = 2,
  DLMS_WRAPPER_STATUS_INVALID_ARGUMENT = 3,
  DLMS_WRAPPER_STATUS_INVALID_VERSION = 4,
  DLMS_WRAPPER_STATUS_INVALID_LENGTH = 5,
  DLMS_WRAPPER_STATUS_INVALID_SOURCE_PORT = 6,
  DLMS_WRAPPER_STATUS_INVALID_DESTINATION_PORT = 7,
  DLMS_WRAPPER_STATUS_DATA_TOO_LARGE = 8,
  DLMS_WRAPPER_STATUS_FRAME_TOO_LARGE = 9,
  DLMS_WRAPPER_STATUS_UNSUPPORTED_FEATURE = 10,
  DLMS_WRAPPER_STATUS_INTERNAL_ERROR = 11
} dlms_wrapper_status_t;

typedef struct dlms_wrapper_stream_decoder_t dlms_wrapper_stream_decoder_t;

dlms_wrapper_status_t dlms_wrapper_encode_wpdu(
  uint16_t source_port,
  uint16_t destination_port,
  const uint8_t* data,
  size_t data_size,
  uint8_t* output,
  size_t output_size,
  size_t* written_size);

dlms_wrapper_status_t dlms_wrapper_decode_wpdu(
  const uint8_t* input,
  size_t input_size,
  uint16_t* source_port,
  uint16_t* destination_port,
  uint8_t* data_output,
  size_t data_output_size,
  size_t* data_size);

dlms_wrapper_status_t dlms_wrapper_stream_decoder_create(
  dlms_wrapper_stream_decoder_t** decoder);

void dlms_wrapper_stream_decoder_destroy(
  dlms_wrapper_stream_decoder_t* decoder);

void dlms_wrapper_stream_decoder_reset(
  dlms_wrapper_stream_decoder_t* decoder);

#ifdef __cplusplus
}
#endif

#endif /* DLMS_WRAPPER_WRAPPER_C_API_H */
