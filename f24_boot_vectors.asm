; Generated fixed normal-vector ABI. Do not edit.
.EXTERN _Device_TimerIsr
.EXTERN _Device_CanRxIsr
.PUBLIC _Device_Jump
.EXTERN _Device_CanStop
.EXTERN _R_Config_TAU0_1_Stop
.SECTION .text, TEXT
device_irq_04 .VECTOR 0x04
device_irq_04:
    CALL !device_dispatch
    .DB 0x04
device_irq_06 .VECTOR 0x06
device_irq_06:
    CALL !device_dispatch
    .DB 0x06
device_irq_08 .VECTOR 0x08
device_irq_08:
    CALL !device_dispatch
    .DB 0x08
device_irq_0A .VECTOR 0x0A
device_irq_0A:
    CALL !device_dispatch
    .DB 0x0A
device_irq_0C .VECTOR 0x0C
device_irq_0C:
    CALL !device_dispatch
    .DB 0x0C
device_irq_0E .VECTOR 0x0E
device_irq_0E:
    CALL !device_dispatch
    .DB 0x0E
device_irq_10 .VECTOR 0x10
device_irq_10:
    CALL !device_dispatch
    .DB 0x10
device_irq_12 .VECTOR 0x12
device_irq_12:
    CALL !device_dispatch
    .DB 0x12
device_irq_14 .VECTOR 0x14
device_irq_14:
    CALL !device_dispatch
    .DB 0x14
device_irq_16 .VECTOR 0x16
device_irq_16:
    CALL !device_dispatch
    .DB 0x16
device_irq_18 .VECTOR 0x18
device_irq_18:
    CALL !device_dispatch
    .DB 0x18
device_irq_1A .VECTOR 0x1A
device_irq_1A:
    CALL !device_dispatch
    .DB 0x1A
device_irq_1C .VECTOR 0x1C
device_irq_1C:
    CALL !device_dispatch
    .DB 0x1C
device_irq_1E .VECTOR 0x1E
device_irq_1E:
    CALL !device_dispatch
    .DB 0x1E
device_irq_20 .VECTOR 0x20
device_irq_20:
    CALL !device_dispatch
    .DB 0x20
device_irq_22 .VECTOR 0x22
device_irq_22:
    CALL !device_dispatch
    .DB 0x22
device_irq_24 .VECTOR 0x24
device_irq_24:
    CALL !device_dispatch
    .DB 0x24
device_irq_26 .VECTOR 0x26
device_irq_26:
    CALL !device_dispatch
    .DB 0x26
device_irq_28 .VECTOR 0x28
device_irq_28:
    CALL !device_dispatch
    .DB 0x28
device_irq_2A .VECTOR 0x2A
device_irq_2A:
    CALL !device_dispatch
    .DB 0x2A
device_irq_2C .VECTOR 0x2C
device_irq_2C:
    CALL !device_dispatch
    .DB 0x2C
device_irq_2E .VECTOR 0x2E
device_irq_2E:
    CALL !device_dispatch
    .DB 0x2E
device_irq_30 .VECTOR 0x30
device_irq_30:
    CALL !device_dispatch
    .DB 0x30
device_irq_32 .VECTOR 0x32
device_irq_32:
    CALL !device_dispatch
    .DB 0x32
device_irq_34 .VECTOR 0x34
device_irq_34:
    CALL !device_dispatch
    .DB 0x34
device_irq_36 .VECTOR 0x36
device_irq_36:
    CALL !device_dispatch
    .DB 0x36
device_irq_38 .VECTOR 0x38
device_irq_38:
    CALL !device_dispatch
    .DB 0x38
device_irq_3A .VECTOR 0x3A
device_irq_3A:
    CALL !device_dispatch
    .DB 0x3A
device_irq_3C .VECTOR 0x3C
device_irq_3C:
    CALL !device_dispatch
    .DB 0x3C
device_irq_3E .VECTOR 0x3E
device_irq_3E:
    CALL !device_dispatch
    .DB 0x3E
device_irq_40 .VECTOR 0x40
device_irq_40:
    CALL !device_dispatch
    .DB 0x40
device_irq_42 .VECTOR 0x42
device_irq_42:
    CALL !device_dispatch
    .DB 0x42
device_irq_44 .VECTOR 0x44
device_irq_44:
    CALL !device_dispatch
    .DB 0x44
device_irq_46 .VECTOR 0x46
device_irq_46:
    CALL !device_dispatch
    .DB 0x46
device_irq_48 .VECTOR 0x48
device_irq_48:
    CALL !device_dispatch
    .DB 0x48
device_irq_4A .VECTOR 0x4A
device_irq_4A:
    CALL !device_dispatch
    .DB 0x4A
device_irq_4C .VECTOR 0x4C
device_irq_4C:
    CALL !device_dispatch
    .DB 0x4C
device_irq_4E .VECTOR 0x4E
device_irq_4E:
    CALL !device_dispatch
    .DB 0x4E
device_irq_50 .VECTOR 0x50
device_irq_50:
    CALL !device_dispatch
    .DB 0x50
device_irq_52 .VECTOR 0x52
device_irq_52:
    CALL !device_dispatch
    .DB 0x52
device_irq_54 .VECTOR 0x54
device_irq_54:
    CALL !device_dispatch
    .DB 0x54
device_irq_56 .VECTOR 0x56
device_irq_56:
    CALL !device_dispatch
    .DB 0x56
device_irq_58 .VECTOR 0x58
device_irq_58:
    CALL !device_dispatch
    .DB 0x58
device_irq_5A .VECTOR 0x5A
device_irq_5A:
    CALL !device_dispatch
    .DB 0x5A
device_irq_5C .VECTOR 0x5C
device_irq_5C:
    CALL !device_dispatch
    .DB 0x5C
device_irq_5E .VECTOR 0x5E
device_irq_5E:
    CALL !device_dispatch
    .DB 0x5E
device_irq_60 .VECTOR 0x60
device_irq_60:
    CALL !device_dispatch
    .DB 0x60
device_irq_62 .VECTOR 0x62
device_irq_62:
    CALL !device_dispatch
    .DB 0x62
device_irq_64 .VECTOR 0x64
device_irq_64:
    CALL !device_dispatch
    .DB 0x64
device_irq_66 .VECTOR 0x66
device_irq_66:
    CALL !device_dispatch
    .DB 0x66
device_irq_68 .VECTOR 0x68
device_irq_68:
    CALL !device_dispatch
    .DB 0x68
device_irq_6A .VECTOR 0x6A
device_irq_6A:
    CALL !device_dispatch
    .DB 0x6A
device_irq_6C .VECTOR 0x6C
device_irq_6C:
    CALL !device_dispatch
    .DB 0x6C
device_irq_6E .VECTOR 0x6E
device_irq_6E:
    CALL !device_dispatch
    .DB 0x6E
device_irq_70 .VECTOR 0x70
device_irq_70:
    CALL !device_dispatch
    .DB 0x70
device_irq_72 .VECTOR 0x72
device_irq_72:
    CALL !device_dispatch
    .DB 0x72
device_irq_74 .VECTOR 0x74
device_irq_74:
    CALL !device_dispatch
    .DB 0x74
device_irq_76 .VECTOR 0x76
device_irq_76:
    CALL !device_dispatch
    .DB 0x76
device_irq_78 .VECTOR 0x78
device_irq_78:
    CALL !device_dispatch
    .DB 0x78
device_irq_7A .VECTOR 0x7A
device_irq_7A:
    CALL !device_dispatch
    .DB 0x7A
device_irq_7C .VECTOR 0x7C
device_irq_7C:
    CALL !device_dispatch
    .DB 0x7C
device_dispatch:
    PUSH AX
    PUSH HL
    MOV A,ES
    PUSH AX
    MOV ES,#0
    MOVW AX,[SP+6]
    MOVW HL,AX
    MOV A,ES:[HL]
    MOV X,A
    MOV A,#0
    SHLW AX,1
    ADDW AX,#0x2000
    CMP !0xF710,#0x55
    BZ $device_irq_target
    CMPW AX,#0x205C
    BNZ $device_irq_can
    MOVW AX,#LOWW(_Device_TimerIsr)
    BR $device_irq_target
device_irq_can:
    CMPW AX,#0x2098
    BNZ $device_irq_default
    MOVW AX,#LOWW(_Device_CanRxIsr)
    BR $device_irq_target
device_irq_default:
    MOVW AX,#LOWW(device_default_isr)
device_irq_target:
    MOVW [SP+6],AX
    POP AX
    MOV ES,A
    POP HL
    POP AX
    RET
device_default_isr:
    RETI
_Device_Jump:
    DI
    CALL !!_Device_CanStop
    CALL !!_R_Config_TAU0_1_Stop
    MOVW AX,#0x4E48
    MOVW !0xF714,AX
    MOVW AX,#0x4644
    MOVW !0xF716,AX
    MOV !0xF710,#0x55
    BR !!0x02000
