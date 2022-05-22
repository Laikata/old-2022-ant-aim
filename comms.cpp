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

float ReverseFloat(float value) {
    union {
    float f;
    uint32_t i;
  } f32_u8 = {.f = value};

    f32_u8.i = __builtin_bswap32(f32_u8.i);

  return f32_u8.f;
}

inline float BigEndianFloat(float value) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  return ReverseFloat(value);
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
  return value;
#else
# error Unsupported endianness
#endif
}

void rev_float(float *value) {
    union {
        float f;
        uint32_t i;
    } f32_u32 = {.f = *value};

    f32_u32.i = __builtin_bswap32(f32_u32.i);
}

extern SoftwareSerial ss;

// Reads from Serial while echoing
size_t echo(uint8_t *buffer, int length) {
  size_t result = ss.readBytes(buffer, length);
  Serial.write(buffer, result);
  return result;
}

// Returns true if there's a new packet
bool comms_recv(vec3 *pos) {
    uint8_t packet[20];
    if(ss.available()){
        echo(packet, 1);
        if(packet[0] == 0x16) {
            const uint8_t data_size = 13;
            echo(packet + 1, 3);
            if(packet[1] != data_size || packet[3] != 0x01) 
                return false;

            //Serial.println("New GPS Packet");
            echo(packet + 4, 12);
            
            // We have to read the checksum before writing to pos
            echo(packet + 16, 4);
            uint32_t checksum = 
                    ( ((uint32_t) packet[16]) << 24) +
                    ( ((uint32_t) packet[17]) << 16) +
                    (packet[18] << 8) +
                    packet[19];

            if(crc32(packet + 3, 13) == checksum) {
              float longitude, latitude, altitude;

              memcpy(&longitude, &packet[4], 4);
              memcpy(&latitude, &packet[8], 4);
              memcpy(&altitude, &packet[12], 4);
              
              float bige_lon = BigEndianFloat(longitude);
              float bige_lat = BigEndianFloat(latitude);
              float bige_alt = BigEndianFloat(altitude);
              
              memcpy(&pos->x, &bige_lon, 4);
              memcpy(&pos->y, &bige_lat, 4);
              memcpy(&pos->z, &bige_alt, 4);
              
              return true;
            }
        }
    }
    return false;
}
