# 02. WRAPPER C ABI

## Principles

```text
extern "C"
no C++ types in public C headers
no exceptions
stable enum values
fixed-width integer types
caller-provided buffers
opaque handles for stateful stream decoders
```

## Status Values

The C ABI mirrors the C++ status model with stable integer values:

```c
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
```

## Buffer Ownership

The C ABI never allocates output payload buffers for the caller. Decode APIs
copy DATA into caller-provided memory and report the required size when the
buffer is too small.

## Stream Decoder Handles

Stateful TCP stream decoding uses opaque handles:

```c
typedef struct dlms_wrapper_stream_decoder_t dlms_wrapper_stream_decoder_t;
```

Handle lifecycle functions must tolerate null handles where practical and must
return status codes for invalid arguments.
