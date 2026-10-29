/* Project/API reading notes: Image CRC32/ISO-HDLC backend.
 * The packaged image stores a little-endian CRC in its final four bytes; callers exclude that
 * trailer from the calculation.
 * RL78 Flash traversal uses 32-bit integer addresses before a far-pointer cast to cross 64 KiB
 * safely.
 */
#include "sw_crc32.h"

#ifdef F13_CRC_HOST_TEST
#include <stddef.h>
/* Host models retain native pointers; CC-RL uses a 32-bit byte address. */
typedef size_t f13_crc_address_t;
#else
typedef unsigned long f13_crc_address_t;
#endif

/* Table-free CRC32/ISO-HDLC: identical image contract, bounded RAM/ROM cost.
 * CC-RL far-pointer arithmetic wraps at 64 KiB. Increment an integer address
 * before casting for each read, for both Flash and staged RAM metadata. */
unsigned long F13_Crc32Bytes(const unsigned char __far *data, unsigned long size)
{
    unsigned long crc;
    unsigned char bit;
    crc = 0xFFFFFFFFUL;
    while (size != 0UL)
    {
        crc ^= *data;
        data = (const unsigned char __far *)((f13_crc_address_t)data + 1UL);
        bit = 8U;
        do
        {
            if ((crc & 1UL) != 0UL)
            {
                crc = (crc >> 1) ^ 0xEDB88320UL;
            }
            else
            {
                crc >>= 1;
            }
        } while (--bit != 0U);
        size--;
    }
    return crc ^ 0xFFFFFFFFUL;
}

#ifndef F13_CRC_HOST_TEST
/* Calculate the CRC over the requested Flash byte range.
 * Keep the address counter 32-bit so the RL78 traversal does not wrap at a 64-KiB pointer
 * boundary.
 */
unsigned long caculate_crc32_checksum(unsigned long start, unsigned long size)
{
    return F13_Crc32Bytes((const unsigned char __far *)start, size);
}
#endif
