#include "comms.h"

static uint32_t crc32(const uint8_t data[], size_t data_length) {
	uint32_t crc32 = 0xFFFFFFFFu;
	
	for (size_t i = 0; i < data_length; i++) {
		const uint32_t lookupIndex = (crc32 ^ data[i]) & 0xff;
		crc32 = (crc32 >> 8) ^ crc_table[lookupIndex];  // CRCTable is an array of 256 32-bit constants
	}
	
	// Finalize the CRC-32 value by inverting all the bits
	crc32 ^= 0xFFFFFFFFu;
	return crc32;
}

void rev_float(float *value) {
    union {
        float f;
        uint32_t i;
    } f32_u32 = {.f = *value};

    f32_u32.i = __builtin_bswap32(f32_u32.i);
}

//Returns true if there's a new packet
bool commms_recv(vec3_t *pos) {
    uint8_t packet[20];
    if(Serial.available()){
        Serial.readBytes(packet, 1);
        if(packet[0] == 0x16) {
            const uint8_t data_size = 13;
            Serial.readBytes(packet + 1, 3);

            if(packet[1] != data_size || packet[3] != 0x01) 
                return false;

            Serial.readBytes(packet + 4, 12);
            memcpy(pos, packet + 4, 4);
            memcpy(pos, packet + 8, 4);
            memcpy(pos, packet + 12, 4);

            #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
                rev_float(&pos->x);
                rev_float(&pos->y);
                rev_float(&pos->z);
            #elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

            #else
            #error Unsupported endianness
            #endif
            
            Serial.readBytes(packet + 16, 4);
            uint32_t checksum = 
                    ( ((uint32_t) packet[16]) << 24) +
                    ( ((uint32_t) packet[17]) << 16) +
                    (packet[18] << 8) +
                    packet[19];

            if(crc32(packet + 3, 13) == checksum) 
                return true;
        }
    }
    return false;
}