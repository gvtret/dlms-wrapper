# Plan for DLMS/COSEM WRAPPER codec implementation

## 1. Purpose

This document describes a phased implementation plan for a portable C++11
DLMS/COSEM WRAPPER layer library.

The plan follows the structure of the neighboring HDLC implementation plan, but
keeps WRAPPER-specific scope separate from HDLC and LLC.

The library is intended for the IP-based DLMS/COSEM profile:

```text
-----------------------------+
| APDU codec                  |
+-----------------------------+
| WRAPPER codec               |
+-----------------------------+
| Transport: TCP/UDP          |
+-----------------------------+
```

## 2. Source requirements

The WRAPPER protocol data unit (WPDU) consists of:

```text
Wrapper header
DATA field (xDLMS APDU)
```

The wrapper header has four 16-bit unsigned fields:

```text
Version
Source wPort
Destination wPort
Data length
```

Fixed v1 decisions:

| Question | Decision |
|---|---|
| Version | Support `0x0001`; reject other versions in strict decode |
| Byte order | Network byte order, big-endian |
| Header size | 8 bytes |
| Data field | Opaque xDLMS APDU bytes |
| Exceptions | Do not use exceptions in public API |
| Errors | Status codes only |
| TCP stream handling | Provide stream decoder by WRAPPER data length |
| UDP handling | Stateless single-WPDU encode/decode |
| TCP connection management | Out of scope for codec v1 |
| wPort binding registry | Out of scope for codec v1 |
| Invalid destination wPort policy | Expose validation helper/policy, do not own AE registry |
| C ABI | Separate stable layer |
| CMake | Minimum 3.16 |
| Tests | GoogleTest |

## 3. Goal for v1

Implement a library that can:

```text
encode a WPDU
decode a complete WPDU
validate wrapper version
validate data length against available bytes and configured limits
preserve APDU bytes exactly
decode TCP stream chunks into complete WPDUs
support client and server wPort values
provide C++11 API
provide stable C ABI
build through CMake 3.16
provide GoogleTest coverage
provide integration tests with APDU-shaped payloads
```

## 4. Scope

### 4.1. In scope

```text
WRAPPER header model
WPDU encoder
WPDU decoder
TCP stream decoder by Data length
UDP datagram decode helper
wPort constants and classification helpers
configurable limits
C++11 API
C ABI wrapper
GoogleTest coverage
CMake 3.16
integration tests in the root workspace
```

### 4.2. Out of scope

```text
TCP socket implementation
UDP socket implementation
TCP connect/disconnect/abort manager
retry or retransmission logic
remote delivery confirmation
APDU parsing
HDLC/LLC handling
security/ciphering
server AE registry
xDLMS block transfer
```

## 5. Considered approaches

### 5.1. Approach A - header encode/decode only

Pros: smallest implementation.

Cons: TCP users must duplicate stream framing.

Risk: every caller reimplements incomplete-WPDU buffering differently.

### 5.2. Approach B - WPDU codec plus stream decoder

Pros: covers both UDP datagrams and TCP stream boundaries.

Cons: slightly larger API and tests.

Risk: stream decoder can accidentally grow into TCP connection management.

### 5.3. Approach C - full TCP/UDP transport layer

Pros: closer to a complete profile.

Cons: sockets, connection state, confirms, and aborts make v1 too broad.

Risk: transport concerns leak into a codec library.

### 5.4. Choice

Use Approach B:

```text
WPDU codec
+ TCP stream decoder
- socket/connection manager
```

This is the smallest useful boundary for a reusable WRAPPER layer.

## 6. Project layout

```text
dlms-wrapper/
 ├── CMakeLists.txt
 ├── include/
 │   └── dlms/
 │       └── wrapper/
 │           ├── wrapper_types.hpp
 │           ├── wrapper_error.hpp
 │           ├── wrapper_ports.hpp
 │           ├── wrapper_frame.hpp
 │           ├── wrapper_codec.hpp
 │           ├── wrapper_stream_decoder.hpp
 │           └── wrapper_c_api.h
 ├── src/
 │   └── wrapper/
 │       ├── wrapper_ports.cpp
 │       ├── wrapper_codec.cpp
 │       ├── wrapper_stream_decoder.cpp
 │       └── wrapper_c_api.cpp
 ├── test/
 │   ├── CMakeLists.txt
 │   └── wrapper/
 │       ├── test_wrapper_error.cpp
 │       ├── test_wrapper_ports.cpp
 │       ├── test_wrapper_codec.cpp
 │       ├── test_wrapper_stream_decoder.cpp
 │       ├── test_wrapper_c_api.cpp
 │       └── test_wrapper_c_header.c
 └── docs/
     ├── 00_wrapper_requirements.md
     ├── 01_wrapper_codec_api.md
     ├── 02_wrapper_c_api.md
     ├── 03_wrapper_stream_decoder.md
     └── 04_wrapper_test_plan.md
```

## 7. Status/error model

```cpp
enum class WrapperStatus
{
  Ok = 0,

  NeedMoreData,
  OutputBufferTooSmall,

  InvalidArgument,
  InvalidVersion,
  InvalidLength,
  InvalidSourcePort,
  InvalidDestinationPort,

  DataTooLarge,
  FrameTooLarge,
  UnsupportedFeature,

  InternalError
};
```

No public runtime path may throw exceptions or call `abort`/`assert`.

## 8. Buffer policy

High-level API may use `std::vector` for convenience:

```cpp
WrapperStatus EncodeWpdu(
  const WrapperFrame& frame,
  std::vector<std::uint8_t>& output);
```

Strict no-allocation API uses caller-provided buffers:

```cpp
WrapperStatus EncodeWpduToBuffer(
  const WrapperFrame& frame,
  std::uint8_t* output,
  std::size_t outputSize,
  std::size_t& writtenSize);
```

The C ABI must use only caller-provided buffers.

## 9. Frame model

```cpp
struct WrapperHeader
{
  std::uint16_t version;
  std::uint16_t sourcePort;
  std::uint16_t destinationPort;
  std::uint16_t dataLength;
};
```

```cpp
struct WrapperFrame
{
  std::uint16_t sourcePort;
  std::uint16_t destinationPort;
  const std::uint8_t* data;
  std::size_t dataSize;
};
```

```cpp
struct WrapperFrameBuffer
{
  std::uint16_t sourcePort;
  std::uint16_t destinationPort;
  std::vector<std::uint8_t> data;
};
```

`WrapperFrame` is a lightweight view. `WrapperFrameBuffer` owns decoded bytes.

## 10. wPort helpers

The library should expose constants and classification helpers without owning an
application entity registry:

```cpp
const std::uint16_t kWrapperVersion = 0x0001;
const std::uint16_t kNoStation = 0x0000;
const std::uint16_t kClientManagementProcess = 0x0001;
const std::uint16_t kPublicClient = 0x0010;
const std::uint16_t kManagementLogicalDevice = 0x0001;
const std::uint16_t kAllStationBroadcast = 0x007f;
```

Validation helpers:

```cpp
bool IsClientWrapperPort(std::uint16_t port);
bool IsServerWrapperPort(std::uint16_t port);
bool IsReservedClientWrapperPort(std::uint16_t port);
bool IsReservedServerWrapperPort(std::uint16_t port);
```

Codec functions validate numeric structure and configured policy. They do not
decide whether a destination AE actually exists.

## 11. Encoder

Encoder writes:

```text
Version
Source wPort
Destination wPort
Data length
DATA
```

Required checks:

```text
source/destination ports fit uint16
data size fits uint16
data size does not exceed configured maximum
output buffer has at least 8 + data size bytes
```

## 12. Decoder

Decoder must:

1. Require at least 8 bytes for the header.
2. Decode all fields in big-endian order.
3. Validate `Version == 0x0001` in strict mode.
4. Validate `Data length` against the remaining input.
5. Validate `Data length` against configured limits.
6. Copy or view exactly the DATA bytes.
7. Ignore trailing bytes only in APIs that explicitly allow datagram remainder.

## 13. TCP stream decoder

The stream decoder accepts arbitrary byte chunks and emits complete WPDUs.

```cpp
class WrapperStreamDecoder
{
public:
  explicit WrapperStreamDecoder(const WrapperStreamDecoderOptions& options);

  WrapperStatus Push(
    const std::uint8_t* data,
    std::size_t size,
    std::vector<WrapperFrameBuffer>& frames);

  void Reset();
};
```

States:

```text
READ_HEADER
READ_DATA_BY_LENGTH
DECODE_FRAME
```

Critical rule:

```text
TCP packet boundaries are not WPDU boundaries.
Only the WRAPPER Data length field determines when a WPDU is complete.
```

## 14. Limits

```cpp
struct WrapperCodecLimits
{
  std::size_t maximumDataSize;
  std::size_t maximumFrameSize;
};
```

Defaults should be conservative and documented. The maximum DATA size cannot
exceed `0xffff` because the header field is 16-bit.

## 15. C ABI

Principles:

```text
extern "C"
no C++ types
no exceptions
opaque handles for stateful decoders
fixed-width integer types
caller-provided buffers
stable enum values
```

Example:

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
  DLMS_WRAPPER_STATUS_FRAME_TOO_LARGE = 9
} dlms_wrapper_status_t;
```

## 16. CMake

The wrapper project should expose a `dlms_wrapper` target and set a feature flag
when the usable codec API is available:

```cmake
set(DLMS_WRAPPER_HAS_CODEC_API ON CACHE INTERNAL "dlms-wrapper codec API is available")
```

The root integration workspace can then include WRAPPER integration tests without
breaking earlier phases.

## 17. Test strategy

### 17.1. Header and codec tests

```text
EncodeWpdu_emptyData
EncodeWpdu_knownAarqPayload
EncodeWpdu_knownGetRequestPayload
EncodeWpdu_outputBufferTooSmall
EncodeWpdu_dataTooLarge
DecodeWpdu_emptyData
DecodeWpdu_knownAarqPayload
DecodeWpdu_invalidVersion
DecodeWpdu_headerTooShort
DecodeWpdu_declaredLengthTooShort
DecodeWpdu_declaredLengthTooLong
DecodeWpdu_maximumLength
DecodeWpdu_payloadContaining7e
```

### 17.2. wPort tests

```text
WrapperPorts_publicClient
WrapperPorts_managementLogicalDevice
WrapperPorts_allStationBroadcast
WrapperPorts_reservedClientRange
WrapperPorts_reservedServerRange
```

### 17.3. Stream decoder tests

```text
Push_fullWpdu
Push_headerThenData
Push_byteByByte
Push_multipleWpdus
Push_partialSecondWpdu
Push_invalidVersion
Push_frameTooLarge
Push_resetAfterError
```

### 17.4. C ABI tests

```text
CApi_encodeWpdu
CApi_decodeWpdu
CApi_outputBufferTooSmall
CApi_streamDecoderCreateDestroy
CApi_noCrashOnNullArguments
CHeader_compilesAsC
```

### 17.5. Root integration tests

```text
WrapperIntegration_AarqApduSurvivesWrapperRoundtrip
WrapperIntegration_GetRequestApduSurvivesWrapperRoundtrip
WrapperIntegration_TcpChunksReassembleWpdu
WrapperIntegration_MultipleWpdusPreserveBoundaries
WrapperIntegration_PayloadByte7eIsData
WrapperIntegration_InvalidDestinationPortIsRejectedByPolicy
```

## 18. Implementation phases

### Phase 0. Requirements documents

Result:

```text
docs/00_wrapper_requirements.md
docs/01_wrapper_codec_api.md
docs/02_wrapper_c_api.md
docs/03_wrapper_stream_decoder.md
docs/04_wrapper_test_plan.md
```

Ready when:

```text
all strict/lenient decode policies are explicit
all default limits are documented
wPort validation boundary is explicit
```

Commit message:

```text
docs(wrapper): define wrapper layer requirements
```

### Phase 1. Project structure

Result:

```text
CMakeLists.txt
include/dlms/wrapper/*.hpp
src/wrapper/*.cpp
test/CMakeLists.txt
```

Ready when:

```text
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

Commit message:

```text
build(wrapper): add wrapper project structure
```

### Phase 2. Status/error model

Result:

```text
wrapper_error.hpp
test_wrapper_error.cpp
```

Ready when:

```text
WrapperStatus values are stable
public API has no exception-based errors
```

Commit message:

```text
feat(wrapper): add status model
```

### Phase 3. wPort constants and helpers

Result:

```text
wrapper_ports.hpp
wrapper_ports.cpp
test_wrapper_ports.cpp
```

Ready when:

```text
reserved client/server ranges are tested
public client and management logical device constants are tested
```

Commit message:

```text
feat(wrapper): add wrapper port helpers
```

### Phase 4. Header encode/decode primitives

Result:

```text
wrapper_frame.hpp
wrapper_codec.hpp
wrapper_codec.cpp
test_wrapper_codec.cpp
```

Ready when:

```text
8-byte header roundtrips
big-endian field encoding is verified
invalid version and invalid length fail
```

Commit message:

```text
feat(wrapper): encode and decode wrapper headers
```

### Phase 5. WPDU encoder

Ready when:

```text
empty and non-empty APDUs encode
output buffer limits are enforced
data length is encoded exactly
```

Commit message:

```text
feat(wrapper): encode wrapper PDUs
```

### Phase 6. WPDU decoder

Ready when:

```text
valid WPDUs decode
truncated header/data returns NeedMoreData
length mismatch and unsupported version are rejected
payload bytes are preserved exactly
```

Commit message:

```text
feat(wrapper): decode wrapper PDUs
```

### Phase 7. TCP stream decoder

Result:

```text
wrapper_stream_decoder.hpp
wrapper_stream_decoder.cpp
test_wrapper_stream_decoder.cpp
```

Ready when:

```text
chunked input reassembles complete WPDUs
multiple WPDUs in one chunk are emitted separately
partial final WPDU remains buffered
reset clears buffered state
```

Commit message:

```text
feat(wrapper): add TCP stream decoder
```

### Phase 8. C ABI

Result:

```text
include/dlms/wrapper/wrapper_c_api.h
src/wrapper/wrapper_c_api.cpp
test_wrapper_c_api.cpp
test_wrapper_c_header.c
```

Ready when:

```text
C header compiles as C
encode/decode APIs use caller-provided buffers
stream decoder handle lifecycle is tested
null arguments return status codes
```

Commit message:

```text
feat(wrapper): add C ABI
```

### Phase 9. Realistic DLMS vectors

Ready when:

```text
AARQ/AARE/GetRequest/GetResponse APDU-shaped payloads roundtrip
known hex WPDUs decode to expected wPorts and APDU bytes
payload containing 0x7e is preserved
```

Commit message:

```text
test(wrapper): add realistic DLMS wrapper vectors
```

### Phase 10. Root integration tests

Result:

```text
E:/work/dlms/test/integration/test_wrapper_integration.cpp
```

Ready when:

```text
root CMake configures with dlms-wrapper present
integration tests run only when DLMS_WRAPPER_HAS_CODEC_API is set
APDU-shaped payloads survive wrapper roundtrip and TCP chunking
```

Commit message:

```text
test(integration): add wrapper integration coverage
```

### Phase 11. Public API documentation

Ready when:

```text
all public C++ and C symbols document ownership, limits, byte order, and statuses
C ABI header is self-contained
```

Commit message:

```text
docs(wrapper): document public wrapper API
```

## 19. Main risks

### 19.1. TCP stream boundaries treated as message boundaries

Control:

```text
stream decoder reads exactly 8 + Data length bytes
chunked and byte-by-byte tests are mandatory
```

### 19.2. Wrong byte order

Control:

```text
known hex header tests
no host-endian memcpy for protocol fields
```

### 19.3. Layer leakage

Control:

```text
wrapper data stays opaque APDU bytes
codec does not parse APDU
codec does not open sockets
codec does not own AE registry
```

### 19.4. Incorrect wPort validation

Control:

```text
constants and range helpers are separately tested
actual bound-destination checks stay outside codec
```

## 20. v1 milestone

```text
M1: Portable C++11 DLMS/COSEM WRAPPER Codec
```

Included:

```text
CMake 3.16
C++11
no exceptions
status-code API
WRAPPER version 0x0001
8-byte big-endian header
WPDU encode/decode
TCP stream decoder by Data length
UDP datagram decode helper
wPort constants/helpers
configurable limits
stable C ABI
GoogleTest coverage
root integration tests
public API documentation
```

Not included:

```text
sockets
TCP connection manager
UDP networking
remote confirm semantics
APDU parser
HDLC/LLC codecs
security/ciphering
AE registry
```

## 21. Next practical step

Start with requirements and project structure:

```text
1. docs/00_wrapper_requirements.md
2. docs/01_wrapper_codec_api.md
3. docs/02_wrapper_c_api.md
4. docs/03_wrapper_stream_decoder.md
5. docs/04_wrapper_test_plan.md
6. CMake project structure
7. empty library target
8. GoogleTest harness
9. WrapperStatus
10. wPort helpers
```
