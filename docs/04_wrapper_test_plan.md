# 04. WRAPPER Test Plan

## Codec Tests

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

## wPort Tests

```text
WrapperPorts_publicClient
WrapperPorts_managementLogicalDevice
WrapperPorts_allStationBroadcast
WrapperPorts_reservedClientRange
WrapperPorts_reservedServerRange
```

## Stream Decoder Tests

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

## C ABI Tests

```text
CApi_encodeWpdu
CApi_decodeWpdu
CApi_outputBufferTooSmall
CApi_streamDecoderCreateDestroy
CApi_noCrashOnNullArguments
CHeader_compilesAsC
```

## Integration Tests

Root integration tests should verify that APDU-shaped payloads survive WRAPPER
roundtrip and TCP chunking. They should be compiled only when the wrapper
library exposes `DLMS_WRAPPER_HAS_CODEC_API`.
