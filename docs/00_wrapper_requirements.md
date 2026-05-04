# 00. WRAPPER Requirements

## Scope

The WRAPPER library implements the DLMS/COSEM wrapper protocol data unit
(WPDU) codec for IP-based profiles. It does not implement TCP or UDP sockets.

The WPDU layout is:

```text
Version, 2 bytes
Source wPort, 2 bytes
Destination wPort, 2 bytes
Data length, 2 bytes
DATA, Data length bytes
```

All header fields are encoded in network byte order.

## Fixed v1 Decisions

| Question | Decision |
|---|---|
| Wrapper version | `0x0001` |
| Header size | 8 bytes |
| DATA field | Opaque xDLMS APDU bytes |
| Errors | Status codes only |
| Exceptions | Not used by public/runtime API paths |
| TCP stream framing | Decode by WRAPPER Data length |
| UDP datagrams | Decode one complete WPDU from datagram data |
| Socket handling | Out of scope |
| TCP connect/disconnect/abort | Out of scope |
| Application entity registry | Out of scope |
| C ABI | Stable wrapper over C++ API |

## Required Behavior

The codec shall:

```text
encode WPDUs with version 0x0001
decode WPDUs with version 0x0001
reject unsupported versions in strict decode paths
validate DATA length against input size and configured limits
preserve DATA bytes exactly
handle empty DATA
handle DATA containing 0x7e as normal data
decode multiple WPDUs from TCP stream chunks
return NeedMoreData for incomplete header or DATA
```

## Out of Scope

```text
TCP sockets
UDP sockets
remote delivery confirmation
connection manager state machine
retry/retransmission
APDU parsing
HDLC/LLC parsing
security/ciphering
```

## Default Limits

`Data length` is a 16-bit unsigned field, therefore the representable maximum
DATA length is `65535` bytes. The default v1 codec limit is `65535` DATA bytes
and `65543` total WPDU bytes.

Callers may configure lower limits.
