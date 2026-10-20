# RL78/F24 MCU CAN FD device Transfer

Standalone, non-destructive Custom Transfer firmware for the 4 MHz F24 target board.
Place this project beside `RL78_F24_Boot_loader_CAN_FD`, which supplies its board/clock/FD driver and PowerShell helpers.
The project uses its own SC output and does not rebuild Boot or App.

## Hardware and protocol

| Setting | Value |
| --- | --- |
| MCU | R7F124FPJ5xFB |
| Clock | X1 4 MHz, PLL 80 MHz, CPU/CAN 40 MHz |
| CAN | P10 TX, P11 RX, P12 STB LOW; 500K nominal / 2M data, BRS |
| Request / response | Standard 0x710 / 0x711, exact filter 0x7FF |
| Diagnostic UART | P15 TX / P16 RX, 115200 |
| Base commands | FD+BRS 8-byte DEVICE_INFO / PING / ECHO / PATTERN / GET_STATS / CLEAR_STATS |
| FD64 test | Command 0x07, wire DLC 15, payload 64 bytes |
| Identity | Target 0xF24B, part prefix R7F124FP, capabilities 0x007F |

FD64 layout is request 0xA5 / response 0x5A, major 1, nonzero sequence, command 0x07,
59 echo data bytes and CRC8-ATM over bytes 0..62 at byte 63.
Capability bit6 announces the extension. Identical requests replay the complete
response; conflicting sequence content, invalid format and bad CRC are counted
and dropped. There are no Flash programming commands.

## Build and outputs

Use Windows PowerShell 5.1/.NET and CS+ CC-RL; no Python is required.
The required sibling layout is:

```text
workspace/
  RL78_F24_Boot_loader_CAN_FD/   # shared board/driver and build helpers
  RL78_F24_CAN_FD_Transfer/      # standalone Transfer project
```

The Boot checkout is required as a source dependency; building Transfer does
not build or program Boot. Keep both directory names when relocating checkouts.
From this directory:

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\build.ps1
```

The entry selects only FD Transfer. Pre/post hooks reference FD helpers and
validate explicit stack, RAM/Flash bounds and the linked project.
Run the same entry after SC regeneration.
Programmer delivery is `DefaultBuild/RL78_F24_CAN_FD_Transfer.hex`.
The raw MOT/map are compiler/packaging inputs, not CAN Upgrade BINs.
Installing this standalone image replaces the board's existing program;
check the actual MCU and programmer erase range first.

## Tool operation

Use a verified CAN FD-capable host. F24 Host App 1.7.0 already supports
generic FD transmission; no additional Host App update is required for this test.

1. Program the standalone HEX and inspect UART CAN init OK.
2. Tool CAN tab: Custom Transfer, CAN FD, nominal 500K, data 2M, Metadata/default filter.
3. Discover Test Target; verify live identity and capabilities.
4. Transfer Test checks base commands, three 64-byte ECHO patterns and 12 statistics.
5. Disconnect before changing interface settings.

No update BIN is used. A cached OFFLINE image does not identify the live target.
Custom Transfer does not report Boot/App metadata.
Use Classical CAN with the independent Classical device firmware.

The vendor RXMB interrupt (INTRCANGRVC, physical vector0x4C) acknowledges RMND
and calls `my_can_glb_rxmb_callback()`. The callback uses the project's unchanged
`R_CAN_ReadRXMBInHandler()` to copy the complete frame into an8-entry RAM queue.
One64-byte RXMB0 receives only standard0x710/mask0x7FF; receive FIFOs are disabled.
ISR work contains no protocol parsing, UART output or TX wait. Main-loop
`Device_CanRead()` consumes only the RAM queue; there is no polling fallback.
TX completion remains polled. Clock, rates, IDs and wire protocol are unchanged.
Startup identifies this path as
`CAN RX backend: RXMB0 interrupt -> RAM queue=8 -> main; payload=64 TX=polling`.
The producer publishes head only after copying; the consumer releases tail after
copying. Nine storage slots provide eight usable queue entries. A full queue
drops the incoming frame and increments drops, retaining pending frames in order.
An overwrite detected by InHandler drops that read and increments overwritten.
Hardware RXMB0 remains a single message slot, separate from the RAM queue.
This profile is standalone Transfer only; FD Boot/App retain their own backend.
Repeat Transfer Test in the same session and check all12 statistics per run
and cumulative RX beyond16/32/64 frames without reinitializing RXMB0.

## Trace and verification

Startup prints `FD 500K/2M BRS payload=8/64` and actual initialization diagnostics.
Trace distinguishes wire DLC 15 from 64 payload bytes, and prints all data and CRC status.
Set `CAN_TRANSFER_UART_TRACE_ENABLE` to `0U` for throughput work; error polling/counters remain.
`CAN_TRANSFER_RUNTIME_DIAGNOSTICS_ENABLE` defaults to the same value.
It adds STATE/IRQ/REGS snapshots at initialization, every 16 received
frames and TX timeout/abort events. Idle sampling is limited to one group per
second and three groups until the next received frame. Normal idle also emits
these samples. Snapshot acquisition briefly masks/restores only RXMB IRQ for
coherent16-bit software counters; it never acknowledges flags, dequeues frames
or resets CAN. IRQ entries, queued/dequeued, drops, API errors, depth/peak,
RMIEC/CPU mask/pending distinguish interrupt capture from main-loop processing.
All16-bit counters saturate. before/after bracket InHandler after the ISR's
RMND acknowledgement, so before=0 is normal. A normal request/reply test should
keep drops/overwritten/api_errors=0 and return to depth=0,queued=dequeued.
After a power cycle,
reconnect diagnostic UART and capture INIT/CAN init/STB before interpreting
the next request. Heartbeat LEDs can toggle even if CAN initialization fails.
See [FD runtime diagnostic fields](https://released.github.io/slide_isp_tool_for_RL78/#can-fd-transfer-diagnostics).
Formatting and protocol processing run in the main loop, not an ISR.
Hardware flags are coalesced and UART error summaries are throttled to 250 ms;
their counters are software observations, not precise physical error counts.

Optional developer-only native tests use existing MSVC tools. From the Tool checkout:

```powershell
.\scripts\test_fd_transfer.ps1 -DeviceProject <FD Transfer checkout>
```

This compiles the actual device C protocol in strict C90 and exercises the
production PC client with trace off/on. Register tests also compile the exact
vendor RXMB ISR and InHandler, DLC table and RX types extracted from this driver;
512 modeled receptions check8/64-byte input, queue wrapping/full/order, masked
IRQ with no polling fallback, overwrite/API errors, buffer guards, counter
saturation, stop/reinit, RAM-window restoration and no FIFO-pointer write.
Tests are software models, not physical
FD bus, power, Flash, IRQ or analyzer evidence.
Normal request/reply operation of this RXMB interrupt image has also been
verified with saved user Tool/UART logs. That does not establish queue-full,
sustained-burst,Flash-update or power-fault acceptance.
See [FD Custom Transfer operation](https://released.github.io/slide_isp_tool_for_RL78/#can-fd-custom-transfer)
and [diagnostic fields](https://released.github.io/slide_isp_tool_for_RL78/#transfer-diagnostics).
