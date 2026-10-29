#ifndef DEVICE_APP_VECTORS_H
#define DEVICE_APP_VECTORS_H

/* Each aligned table word is BR !!address: EC, address low/mid/high.
 * Validate all 64 destinations after image metadata and CRC are accepted.
 * No physical vector ownership or ISR selection is changed here. */
static unsigned char Device_AppVectorsValid(const unsigned long __far *table, unsigned long first,
                                            unsigned long limit)
{
    unsigned long entry;
    unsigned char slot;
    for (slot = 0U; slot < 64U; ++slot)
    {
        entry = table[slot];
        if ((unsigned char)entry != 0xECU)
        {
            return 0U;
        }
        entry >>= 8;
        if (entry < first || entry >= limit)
        {
            return 0U;
        }
    }
    return 1U;
}
#endif
