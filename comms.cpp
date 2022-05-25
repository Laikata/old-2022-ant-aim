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
        if(echo(packet, 1) < 1) return false;
        if(packet[0] == 0x16) {
            const uint8_t data_size = 13;
            if(echo(packet + 1, 3) < 3) return false;
            if(packet[1] == data_size){ 
              if(packet[3] != 0x01 && packet[3] != 0x03) return false;

              if(echo(packet + 4, 12) < 12) return false;
              
              // We have to read the checksum before writing to pos
              if(echo(packet + 16, 4) < 4) return false;
              uint32_t checksum = 
                      ( ((uint32_t) packet[16]) << 24) +
                      ( ((uint32_t) packet[17]) << 16) +
                      (packet[18] << 8) +
                      packet[19];

              if(crc32(packet + 3, 13) == checksum) {
                if(packet[3] == 0x01) {
                  float longitude, latitude;
                  memcpy(&longitude, &packet[4], 4);
                  memcpy(&latitude, &packet[8], 4);
                  
                  float bige_lon = BigEndianFloat(longitude);
                  float bige_lat = BigEndianFloat(latitude);
                  bige_lon *= PI/180;
                  bige_lat *= PI/180;

                  pos->x = bige_lon;
                  pos->y = bige_lat;
                } else if(packet[3] == 0x03) {
                  float pressure;
                  memcpy(&pressure, &packet[12], 4);
                  float bige_pre = BigEndianFloat(pressure);
                  pos->z = 44330 * (1- pow(bige_pre/101325, 1/5.255));
                  //Serial.println(44330 * (1- pow(bige_pre/101325, 1/5.255)));
                }
                
                return true;
              }
            }
        }
    }
    return false;
}
