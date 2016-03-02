#ifndef PROTOCOL_H
#define PROTOCOL_H

#pragma pack(1)

const char C_MAGIC_NUMBER[3] = {0x12, 0x34, 0x56};


typedef enum {
    E_PacketType_Empty = 0,
    E_PacketType_VBat,
    E_PacketType_InternalTemperature,
    E_PacketType_VBatAndInternalTemperature,
    E_PacketType_MAX,
} E_PacketType;


static unsigned short packetTypeDataLength(E_PacketType pType) {
    switch(pType) {
		case E_PacketType_Empty:
			return 0;
		case E_PacketType_VBat:
			return 2;
		case E_PacketType_InternalTemperature:
			return 2;
		case E_PacketType_VBatAndInternalTemperature:
			return 4;

		default: return 0;
    }
}


typedef struct {
    unsigned char magicNumber[3];
    unsigned short dataLength;
    unsigned char packetType;
    unsigned char radioID;
    short rssi;
} S_PacketHeader;

typedef struct {
	S_PacketHeader header;
    unsigned char *data;
} S_Packet;

#pragma pack()


#endif // PROTOCOL_H
