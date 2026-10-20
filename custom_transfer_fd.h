#ifndef CUSTOM_TRANSFER_FD_H
#define CUSTOM_TRANSFER_FD_H
/* Additive Custom Transfer v1 extension. Standard710/711, FD+BRS.
 * Eight-byte discovery/control remains v1. ECHO_FD64 uses wire DLC15.
 * Layout: magic,version,nonzero sequence,command,59 echoed bytes,CRC8-ATM.
 * CRC8 covers bytes0..62; response magic is5A, request magic isA5.
 */
#define CT_FD64_COMMAND (0x07U)
#define CT_FD64_SIZE (64U)
#define CT_FD64_PAYLOAD (59U)
#define CT_FD64_CAPABILITY (0x0040U)
#endif
