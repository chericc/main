# About

## About pcapng

https://pcapng.com/

## Packet processing

Pcapng --> RawData(Ethernet) --> EthernetPacket --> IPv4Packet(v6..) --> UDPPacket --> RTPPacket --> TsPacket --> ... (until no more parsing can be applied)

Packet:
[
    Type;
    SharedData; (raw data, offset, size) (the sub data)
    ParsedInfo:
     |--> [type, info]
]