# dlms-wrapper

`dlms-wrapper` implements the DLMS/COSEM Wrapper protocol data unit codec for
TCP/UDP/IP communication profiles.

The library encodes and decodes Wrapper WPDUs and provides a stream decoder for
TCP byte streams. It does not own sockets and does not parse APDU contents.

## Scope

Included:

- Wrapper version `0x0001`;
- 8-byte Wrapper header encode/decode;
- source and destination wrapper ports;
- DATA length validation;
- opaque DATA preservation;
- TCP stream decoder using Wrapper data length;
- stable C ABI wrapper;
- GoogleTest unit tests.

Not included:

- TCP or UDP sockets;
- APDU parsing;
- HDLC or LLC parsing;
- Application Association state;
- security and ciphering.

## Documentation

- [requirements](docs/00_wrapper_requirements.md)
- [codec API](docs/01_wrapper_codec_api.md)
- [C API](docs/02_wrapper_c_api.md)
- [stream decoder](docs/03_wrapper_stream_decoder.md)
- [architecture](docs/architecture.md)
- [test plan](docs/04_wrapper_test_plan.md)
