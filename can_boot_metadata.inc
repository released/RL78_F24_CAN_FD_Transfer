/* Shared by the real boot transport and host-side fault tests. C90 only.
 * No erase/write operation is reachable through staging or validation. */
static unsigned long g_metadata_words[64];
static unsigned short g_metadata_staged;
static unsigned char g_metadata_sealed;
/* No armed permission: FORCE_START applies only to its own START request. */
#define VersionAllowed(policy, installed, command) \
    ((command) == CAN_BOOT_CMD_FORCE_START || (policy) != 3U || \
     g_metadata_words[4] >= (installed))

static void MetadataReset(void)
{
    g_metadata_staged = 0U;
    g_metadata_sealed = 0U;
}

/* Transport adapter: status mapping and RAM staging stay outside parser. */
#if IMAGE_METADATA_BAD_IMAGE != CAN_BOOT_STATUS_BAD_IMAGE
#error Map metadata validation results to this transport's status codes.
#endif
#define MetadataValid(words) (ImageMetadataValidate(words) == IMAGE_METADATA_OK)

static unsigned char MetadataStage(unsigned short offset, const unsigned char *data)
{
    unsigned char i;
    unsigned char *bytes;
    bytes = (unsigned char *)g_metadata_words;
    if ((offset & 3U) || offset >= 256U || offset > g_metadata_staged) return CAN_BOOT_STATUS_BAD_SEQUENCE;
    if (offset < g_metadata_staged)
    {
        for (i = 0U; i < 4U; i++) if (bytes[offset+i] != data[i]) return CAN_BOOT_STATUS_BAD_IMAGE;
        return CAN_BOOT_STATUS_OK;
    }
    for (i = 0U; i < 4U; i++) bytes[offset+i] = data[i];
    g_metadata_staged += 4U;
    g_metadata_sealed = 0U;
    return CAN_BOOT_STATUS_OK;
}

static unsigned char MetadataSeal(unsigned long expected_crc)
{
    g_metadata_sealed = 0U;
    if (g_metadata_staged != 256U ||
        !MetadataValid((const unsigned long __far *)g_metadata_words)) return CAN_BOOT_STATUS_BAD_IMAGE;
    if (F13_Crc32Bytes((const unsigned char __far *)g_metadata_words, 256UL) != expected_crc)
        return CAN_BOOT_STATUS_CRC_ERROR;
    g_metadata_sealed = 1U;
    return CAN_BOOT_STATUS_OK;
}
