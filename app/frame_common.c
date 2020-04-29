#include "frame_common.h"

uint8_t byte_sum_checksum(uint8_t *data, uint32_t length)
{
   uint32_t i;
   uint8_t checksum = data[0];

   for (i = 1; i < length; i++) {
      checksum = checksum ^ data[i];
   }

   return checksum;
}
