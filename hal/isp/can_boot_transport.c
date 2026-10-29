/* Project/API reading notes: CAN Boot protocol adapter.
 * Validate frame length, command/state, metadata and update bounds before changing the session.
 * Raw data acceptance, physical Flash completion and complete-image validation are separate
 * stages.
 */
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "can_boot_transport.h"
#include "user_def.h"
#include "sw_crc32.h"
#include "device_board.h"

#define CAN_BOOT_DATA_BYTES (4U)
#define CAN_BOOT_FRAME_DLC (8U)
#define CAN_BOOT_CAPACITY_KIB (FLASH_UPDATE_MAX_SIZE / 1024UL)

#if (CAN_BOOT_INACTIVITY_TIMEOUT_MS > 65535U)
#error CAN_BOOT_INACTIVITY_TIMEOUT_MS must fit in the 16-bit timer counter.
#endif

extern unsigned char Device_FlashWrite(unsigned long address, unsigned char __near *data);
extern unsigned char Device_FlashErase(unsigned long address);

static volatile unsigned short g_can_boot_inactivity_ms;
static volatile unsigned char g_can_boot_timeout_configured;
static volatile unsigned char g_can_boot_timeout_enabled;
static volatile unsigned char g_can_boot_timeout_expired;
static unsigned char g_version_policy;

static void CAN_BootTimeoutStart(unsigned char enabled)
{
    g_can_boot_timeout_enabled = 0U;
    g_can_boot_timeout_configured = enabled;
    g_can_boot_inactivity_ms = 0U;
    g_can_boot_timeout_expired = 0U;
    if (enabled != 0U)
    {
        g_can_boot_timeout_enabled = 1U;
    }
}

static void CAN_BootTimeoutKick(void)
{
    g_can_boot_timeout_enabled = 0U;
    g_can_boot_inactivity_ms = 0U;
    g_can_boot_timeout_expired = 0U;
    if (g_can_boot_timeout_configured != 0U)
    {
        g_can_boot_timeout_enabled = 1U;
    }
}

static unsigned char CAN_BootTimeoutExpired(void)
{
    return g_can_boot_timeout_expired;
}

static signed long CAN_BootFinish(signed long result)
{
    g_can_boot_timeout_enabled = 0U;
    g_can_boot_timeout_configured = 0U;
    return result;
}

void CAN_BootTimerTick(void)
{
    if ((g_can_boot_timeout_enabled != 0U) && (g_can_boot_timeout_expired == 0U))
    {
        g_can_boot_inactivity_ms++;
        if (g_can_boot_inactivity_ms >= CAN_BOOT_INACTIVITY_TIMEOUT_MS)
        {
            g_can_boot_timeout_expired = 1U;
            g_can_boot_timeout_enabled = 0U;
        }
    }
}

static void CAN_BootSend(unsigned char command, unsigned char status, unsigned short sequence,
                         unsigned long value)
{
    can_frame_t frame;

    /* The Boot HAL supplies the fixed standard response ID/FD+BRS/DLC8. */
    frame.DB[0] = command;
    frame.DB[1] = status;
    frame.DB[2] = (unsigned char)sequence;
    frame.DB[3] = (unsigned char)(sequence >> 8);
    frame.DB[4] = (unsigned char)value;
    frame.DB[5] = (unsigned char)(value >> 8);
    frame.DB[6] = (unsigned char)(value >> 16);
    frame.DB[7] = (unsigned char)(value >> 24);

    (void)Device_CanSend(&frame);
}

static void CAN_BootSendHello(unsigned char status)
{
    CAN_BootSend(CAN_BOOT_CMD_HELLO, status,
                 (unsigned short)CAN_BOOT_PROTOCOL_VERSION |
                     ((unsigned short)IMAGE_FORMAT_VERSION << 8),
                 (unsigned long)IMAGE_TARGET_ID | ((unsigned long)CAN_BOOT_CAPACITY_KIB << 16));
}

#include "can_boot_metadata.inc"
/* Reuse the manifest contract instead of duplicating its ROM words for INFO. */
static const unsigned char g_info_word[] = {0U,  0U,  0U,  4U,  5U,  7U,  8U,  9U,  10U,
                                            11U, 0U,  12U, 13U, 14U, 15U, 16U, 17U, 18U,
                                            19U, 20U, 21U, 22U, 6U,  23U, 25U, 26U, 0U};

static void CAN_BootSendInfo(unsigned char page, unsigned char status)
{
    unsigned long value;
    unsigned char word;
    if (page == CAN_BOOT_INFO_PAGE_VERSION_POLICY)
    {
        value = g_version_policy;
    }
    else if (page > CAN_BOOT_INFO_PAGE_BLOCK)
    {
        value = 0UL;
        status = CAN_BOOT_STATUS_BAD_SIZE;
    }
    else
    {
        word = g_info_word[page];
        value = g_image_metadata_contract[word];
        if (page == 0U)
        {
            value = IMAGE_TARGET_ID | ((unsigned long)CAN_BOOT_PROTOCOL_VERSION << 16) |
                    ((unsigned long)IMAGE_FORMAT_VERSION << 24);
        }
        else if (page == 1U)
        {
            value = IMAGE_BOOT_VERSION;
        }
        else if (page == 2U)
        {
            value = IMAGE_BOOT_BUILD;
        }
        else if (page == 10U)
        {
            value = CAN_BOOT_CAN_MODE_MASK | ((unsigned long)IMAGE_CHECKSUM_CRC32 << 16) |
                    ((unsigned long)CAN_BOOT_PROTOCOL_ID << 24);
        }
        else if (page == 26U)
        {
            value = CAN_BOOT_BLOCK_INFO;
        }
        else if (word >= 4U && word <= 6U)
        {
            value = (g_version_policy & 1U)
                        ? ((const unsigned long __far *)APP_MANIFEST_ADDRESS)[word]
                        : 0xFFFFFFFFUL;
        }
    }

    CAN_BootSend(CAN_BOOT_CMD_INFO, status,
                 (unsigned short)page | ((unsigned short)CAN_BOOT_STATE_BOOTLOADER << 8), value);
}

static unsigned char CAN_BootRead(can_frame_t *frame)
{
    Can_RtnType result;

    result = Device_CanRead(frame);
    if ((result != CAN_RTN_OK) && (result != CAN_RTN_OK_WITH_LOST))
    {
        return 0U;
    }
    if ((frame->IDE != 0U) || (frame->RTR != 0U) ||
        ((frame->IDL != CAN_BOOT_REQUEST_ID) && (frame->IDL != CAN_BOOT_BLOCK_DATA_ID)) ||
        (frame->DLC == 0U))
    {
        return 0U;
    }

    CAN_BootTimeoutKick();
    return 1U;
}

#include "can_boot_block.inc"

static signed long CAN_BootWrite(unsigned long address, unsigned char __near *data)
{
    unsigned char status;

    if ((address < FLASH_AREA) || (address > (FLASH_END_ADDRESS - (CAN_BOOT_DATA_BYTES - 1U))))
    {
        return CAN_BOOT_RESULT_SIZE_ERROR;
    }

    status = FSL_OK;
    DI();
    if ((address & (FLASH_BYTE_PER_BLOCK - 1UL)) == 0UL)
    {
        status = Device_FlashErase(address);
    }
    if (status == FSL_OK)
    {
        status = Device_FlashWrite(address, data);
    }
    EI();

    if (status != FSL_OK)
    {
        return CAN_BOOT_RESULT_FLASH_ERROR;
    }
    return 0L;
}

static unsigned long FrameValue(const can_frame_t *frame)
{
    return (unsigned long)frame->DB[4] | ((unsigned long)frame->DB[5] << 8) |
           ((unsigned long)frame->DB[6] << 16) | ((unsigned long)frame->DB[7] << 24);
}

unsigned char CAN_BootInit(void)
{
    return Device_CanInit(CAN_BOOT_REQUEST_ID, 1U);
}

signed long CAN_BootReceive(unsigned long destination, unsigned char flash_init_status,
                            unsigned char timeout_enabled)
{
    can_frame_t frame;
    unsigned long address;
    unsigned long value;
    unsigned long image_size;
    unsigned long manifest_image_size;
    unsigned long crc_address;
    unsigned long expected_offset;
    unsigned long request_offset;
    unsigned short target_id;
    unsigned short detail;
    unsigned long reply;
    unsigned char started;
    unsigned char hello_status;
    unsigned char word;
    signed long result;

    address = destination;
    expected_offset = 0UL;
    image_size = 0UL;
    started = 0U;
    g_block_bad = 1U;
    MetadataReset();
    g_version_policy = CAN_BootInstalledValid();
    if (g_version_policy)
    {
        g_version_policy |= (unsigned char)(*(__far unsigned char *)(APP_MANIFEST_ADDRESS +
                                                                     IMAGE_VERSION_CONTROL_OFFSET)
                                            << 1);
    }
    CAN_BootTimeoutStart(timeout_enabled);

    for (;;)
    {
        if (CAN_BootTimeoutExpired() != 0U)
        {
            if (started != 0U)
            {
                return CAN_BootFinish(CAN_BOOT_RESULT_SESSION_TIMEOUT);
            }
            return CAN_BootFinish(CAN_BOOT_RESULT_TIMEOUT);
        }
        if (CAN_BootRead(&frame) == 0U)
        {
            continue;
        }

        if (frame.IDL != CAN_BOOT_REQUEST_ID)
        {
            if (started != 0U)
            {
                BlockSegment((unsigned char)(frame.IDL & 7U), frame.DLC, frame.DB);
            }
            continue;
        }
        hello_status = CAN_BOOT_STATUS_OK;
        detail = (unsigned short)(expected_offset / 4UL);
        reply = address;
        result = 0L;
        switch (frame.DB[0])
        {
        case CAN_BOOT_CMD_HELLO:
            hello_status = CAN_BOOT_STATUS_OK;
            if (flash_init_status != FSL_OK)
            {
                hello_status = CAN_BOOT_STATUS_FLASH_ERROR;
            }
            CAN_BootSendHello(hello_status);
            continue;

        case CAN_BOOT_CMD_INFO:
            if (frame.DLC != 2U)
            {
                CAN_BootSendInfo(0xFFU, CAN_BOOT_STATUS_BAD_SIZE);
            }
            else
            {
                CAN_BootSendInfo(frame.DB[1], CAN_BOOT_STATUS_OK);
            }
            continue;

        case CAN_BOOT_CMD_METADATA_RESET:
            hello_status = CAN_BOOT_STATUS_BAD_STATE;
            if (started == 0U && frame.DLC == 1U)
            {
                MetadataReset();
                hello_status = CAN_BOOT_STATUS_OK;
            }
            detail = 0U;
            reply = 256UL;
            break;

        case CAN_BOOT_CMD_METADATA:
            hello_status = CAN_BOOT_STATUS_BAD_STATE;
            if (started == 0U && frame.DLC == 8U && frame.DB[3] == 0U)
            {
                hello_status = MetadataStage(
                    (unsigned short)frame.DB[1] | ((unsigned short)frame.DB[2] << 8), &frame.DB[4]);
            }
            detail = g_metadata_staged;
            reply = 256UL;
            break;

        case CAN_BOOT_CMD_SEAL:
            hello_status = CAN_BOOT_STATUS_BAD_IMAGE;
            if (started == 0U && frame.DLC == 8U && frame.DB[1] == 0U && frame.DB[2] == 0U &&
                frame.DB[3] == 0U)
            {
                hello_status = MetadataSeal(FrameValue(&frame));
            }
            detail = g_metadata_staged;
            reply = 256UL;
            break;

        case CAN_BOOT_CMD_START:
        case CAN_BOOT_CMD_FORCE_START:
            if (frame.DLC != CAN_BOOT_FRAME_DLC)
            {
                hello_status = CAN_BOOT_STATUS_BAD_SIZE;
                reply = FLASH_UPDATE_MAX_SIZE;
                break;
            }
            target_id = (unsigned short)frame.DB[2] | ((unsigned short)frame.DB[3] << 8);
            if (target_id != IMAGE_TARGET_ID || frame.DB[1] != CAN_BOOT_PROTOCOL_VERSION)
            {
                hello_status = CAN_BOOT_STATUS_BAD_TARGET;
                reply = IMAGE_TARGET_ID;
                break;
            }
            value = FrameValue(&frame);
            if (g_metadata_sealed == 0U || value != g_metadata_words[6])
            {
                hello_status = CAN_BOOT_STATUS_BAD_IMAGE;
                detail = 0U;
                reply = 0UL;
                break;
            }
            if (started != 0U)
            {
                detail = 0U;
                reply = destination;
                break;
            }
            reply = *(__far unsigned long *)(APP_MANIFEST_ADDRESS + IMAGE_MANIFEST_VERSION_OFFSET);
            if (!VersionAllowed(g_version_policy, reply, frame.DB[0]))
            {
                hello_status = CAN_BOOT_STATUS_VERSION;
                break;
            }
            image_size = value;
            if (flash_init_status != FSL_OK)
            {
                hello_status = CAN_BOOT_STATUS_FLASH_ERROR;
                reply = flash_init_status;
                break;
            }
            address = destination;
            expected_offset = 0UL;
            started = 1U;
            detail = 0U;
            reply = address;
            break;

        case CAN_BOOT_CMD_BLOCK_BEGIN:
            g_block_bad = 1U;
            if (started != 0U && frame.DLC == 8U)
            {
                request_offset = (unsigned long)frame.DB[1] | ((unsigned long)frame.DB[2] << 8) |
                                 ((unsigned long)frame.DB[3] << 16);
                BlockBegin(request_offset, FrameValue(&frame), expected_offset, image_size);
            }
            continue;

        case CAN_BOOT_CMD_BLOCK_COMMIT:
            if ((started == 0U) || (frame.DLC != CAN_BOOT_FRAME_DLC))
            {
                hello_status = CAN_BOOT_STATUS_BAD_STATE;
                break;
            }
            request_offset = (unsigned long)frame.DB[1] | ((unsigned long)frame.DB[2] << 8) |
                             ((unsigned long)frame.DB[3] << 16);
            hello_status = BlockCheck(request_offset, FrameValue(&frame));
            if (hello_status == CAN_BOOT_STATUS_OK && request_offset != expected_offset)
            {
                hello_status =
                    (F13_Crc32Bytes((const unsigned char __far *)(destination + request_offset),
                                    g_block_length) == g_block_crc)
                        ? CAN_BOOT_STATUS_OK
                        : CAN_BOOT_STATUS_BAD_SEQUENCE;
            }
            else if (hello_status == CAN_BOOT_STATUS_OK)
            {
                if (expected_offset >= 0x100UL && expected_offset < 0x200UL)
                {
                    for (word = 0U; word < g_block_length / 4U; word++)
                    {
                        if (g_block_words[word] !=
                            g_metadata_words[(unsigned short)((expected_offset - 0x100UL) / 4UL) +
                                             word])
                        {
                            hello_status = CAN_BOOT_STATUS_BAD_IMAGE;
                            result = CAN_BOOT_RESULT_IMAGE_ERROR;
                            goto block_done;
                        }
                    }
                }
                for (word = 0U; word < g_block_length / 4U; word++)
                {
                    result = CAN_BootWrite(address, (unsigned char __near *)&g_block_words[word]);
                    if (result != 0L)
                    {
                        hello_status = CAN_BOOT_STATUS_FLASH_ERROR;
                        goto block_done;
                    }
                    address += 4UL;
                }
                expected_offset += g_block_length;
            }
        block_done:
            detail = (unsigned short)(request_offset / 64UL);
            reply = address;
            break;

        case CAN_BOOT_CMD_END:
            if ((started == 0U) || (expected_offset != image_size) ||
                (address != (destination + image_size)))
            {
                hello_status = CAN_BOOT_STATUS_BAD_SIZE;
                break;
            }
            manifest_image_size =
                *(__far unsigned long *)(APP_MANIFEST_ADDRESS + IMAGE_MANIFEST_SIZE_OFFSET);
            if ((ImageMetadataValidate((const unsigned long __far *)APP_MANIFEST_ADDRESS) !=
                 IMAGE_METADATA_OK) ||
                (manifest_image_size != image_size))
            {
                hello_status = CAN_BOOT_STATUS_BAD_IMAGE;
                reply = manifest_image_size;
                result = CAN_BOOT_RESULT_IMAGE_ERROR;
                break;
            }
            crc_address = destination + image_size - APP_CRC_SIZE;
            reply = caculate_crc32_checksum(destination, image_size - APP_CRC_SIZE);
            if (reply != *(__far unsigned long *)crc_address)
            {
                hello_status = CAN_BOOT_STATUS_CRC_ERROR;
                result = CAN_BOOT_RESULT_CRC_ERROR;
            }
            else
            {
                result = (signed long)image_size;
            }
            break;

        case CAN_BOOT_CMD_ABORT:
            result = CAN_BOOT_RESULT_ABORT;
            break;

        default:
            hello_status = CAN_BOOT_STATUS_BAD_COMMAND;
            break;
        }
        CAN_BootSend(frame.DB[0], hello_status, detail, reply);
        if (result != 0L)
        {
            return CAN_BootFinish(result);
        }
    }
}
