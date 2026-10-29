/* Project/API reading notes: Boot/App RAM handoff contract.
 * Preserve the reserved address, signature/request validation and AA/55 owner meanings across
 * both images.
 * These RAM markers coordinate startup/IRQ ownership; they do not replace complete image
 * validation.
 */
#ifndef BOOT_APP_HANDOFF_H
#define BOOT_APP_HANDOFF_H

/* Boot/App no-init RAM contract; this is not serialized image metadata.
 * The owner byte selects Boot (AA) or App (55) IRQ dispatch.
 * HANDOFF is written just before App entry; UPDATE_COMPLETE is written after
 * a successful update. App consumes/clears both one-shot signatures.
 * Keep linker reservations/startup and generated dispatchers aligned.
 */
#define BOOT_APP_SHARED_RAM_ADDRESS (0x000FF710UL)
#define BOOT_APP_SHARED_RAM_SIZE (16U)
#define BOOT_APP_RAM_OWNER_OFFSET (0U)
#define BOOT_APP_RAM_HANDOFF_OFFSET (4U)
#define BOOT_APP_RAM_UPDATE_OFFSET (8U)
#define BOOT_APP_RAM_OWNER_BOOT (0xAAU)
#define BOOT_APP_RAM_OWNER_APPLICATION (0x55U)
#define BOOT_APP_HANDOFF_SIGNATURE (0x46444E48UL)         /* "HNDF" */
#define BOOT_APP_UPDATE_COMPLETE_SIGNATURE (0x44505543UL) /* "CUPD" */
#define BOOT_APP_RESET_TO_BOOT_SIGNATURE (0xAA55AA55UL)

#endif
