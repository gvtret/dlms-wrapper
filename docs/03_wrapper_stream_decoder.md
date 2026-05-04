# 03. WRAPPER Stream Decoder

## Purpose

TCP is a byte stream and does not preserve WPDU boundaries. The WRAPPER stream
decoder hides this from callers by buffering bytes until a complete WPDU is
available.

## States

```text
READ_HEADER
READ_DATA_BY_LENGTH
DECODE_FRAME
```

## Rules

The stream decoder shall:

```text
accept arbitrary chunk sizes
return NeedMoreData while a WPDU is incomplete
emit one frame for one complete WPDU
emit multiple frames when one chunk contains multiple WPDUs
leave a partial final WPDU buffered
clear buffered state on Reset()
enforce configured frame and data limits
```

The stream decoder must not:

```text
open sockets
poll TCP connection status
send TCP-ABORT indication
retry or retransmit data
parse APDUs
```

## Critical Boundary

Only the WRAPPER `Data length` field determines WPDU completion. Bytes such as
`0x7e` inside DATA have no framing meaning.
