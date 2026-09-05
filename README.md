# CUERT Firmware Task — FreeRTOS Command Pipeline on ESP32

## Overview

This project implements a small, safety-relevant FreeRTOS pipeline on a real ESP32 Dev Module board. It simulates how a vehicle's control system would receive commands (throttle, steer, brake) over a communication link, actuate an output, and fail safe if the link goes quiet — using UART (USB-serial) in place of CAN, and an LED in place of a motor.

Four FreeRTOS tasks work together as a pipeline:

| Task | Priority | Role |
|---|---|---|
| `COMMAND_RX` | 4 (highest) | Reads lines from UART, parses them, pushes valid commands onto a queue |
| `ACTUATE` | 3 | Consumes the queue, drives output, enforces brake override |
| `WATCHDOG` | 2 | Detects link loss (no command in 500ms), forces fail-safe |
| `STATUS` | 1 (lowest) | Prints a status line once a second |

## Hardware

- **Board:** ESP32 Dev Module (Arduino core, ESP-IDF's FreeRTOS built in)
- **Connection:** USB cable only, no external components
- **Baud rate:** 115200, 8N1

### A note on the LED

I don't currently have a working external LED available, so I first tried using the ESP32 Dev Module's onboard LED (commonly wired to GPIO2 on most of these boards). On my specific board, this did not work — the onboard LED did not respond to PWM output on GPIO2. Rather than lose time chasing the exact wiring of my particular clone board, I redirected the "actuation output" to the serial console instead: every time `ACTUATE` or `WATCHDOG` would have changed the LED's brightness or blink state, it instead prints the equivalent value (duty cycle / blink state) over UART. This keeps the actual task logic — the part being evaluated — identical to what it would be with a real LED; only the final output step changed from `ledcWrite()` to a `UART_Display()` print. If given more time, my first fix would be to confirm the correct LED pin for this exact board (or wire up an external LED with a resistor) and restore the `ledcWrite()` call.

## Software Architecture

### Files

- `main.ino` — `setup()`/`loop()`, creates the shared queue and mutex, creates all four tasks with their priorities
- `Uart.h` / `Uart.cpp` — UART init, blocking `UART_GetLine()`, `UART_Display()` for logging
- `command_prase.h` / `command_prase.cpp` — `Command_t` struct and `Command_Parse()`, turns a raw text line into a validated command
- `command_rx_task.h` / `command_rx_task.cpp` — Task 1 (COMMAND_RX)
- `actuate_task.h` / `actuate_task.cpp` — Task 2 (ACTUATE)
- `watchdog_task.h` / `watchdog_task.cpp` — Task 3 (WATCHDOG)
- `status_task.h` / `status_task.cpp` — Task 4 (STATUS)
- `shared.h` — shared queue handle, mutex handle, and global state (`current_throttle`, `brake_active`, `last_valid_command`, `last_command_time`) used across tasks

### Command struct

```c
typedef struct {
    char type;                     // 'T' throttle, 'S' steer, 'B' brake, 'P' ping
    signed int value;              // 0..100 or -100..100
    unsigned long int reciving_time; // ms timestamp the command was parsed
} Command_t;
```

### Shared state and synchronization

`ACTUATE` writes `current_throttle`, `brake_active`, and `last_valid_command`; `STATUS` and `WATCHDOG` read some of them. All reads/writes of this shared state are wrapped in a FreeRTOS mutex (`stateMutex`) so no task ever observes a half-updated value.

### Queue and the BRAKE-priority rule

Commands flow from `COMMAND_RX` to `ACTUATE` through a 10-item FreeRTOS queue. The task brief requires that a BRAKE command must never be dropped, even under load. My rule:

**The newest BRAKE command always wins.** If the queue is full when a BRAKE arrives, `COMMAND_RX` unconditionally discards the oldest item in the queue (whatever it is) to make room for the new BRAKE, then sends it. I chose this over only discarding non-BRAKE items because a queued BRAKE can be stale — a newer BRAKE command reflects the most current instruction, so a vehicle should always act on the freshest stop command it has, not an older one that happened to already be queued.

## How to Build and Run

1. Install **Arduino IDE** (2.x).
2. In Boards Manager, install the **esp32 by Espressif Systems** package.
3. Select **Tools → Board → ESP32 Dev Module**.
4. Select the correct **Tools → Port** for your board.
5. Open `firmware/main/main.ino`.
6. Upload.
7. Open the **Serial Monitor** at **115200 baud**, line ending set to **Newline**.

## Testing It Yourself

Type each line below into the Serial Monitor and press Enter (matching the official test script):

| # | Type this | Expected result |
|---|---|---|
| 1 | `PING` | `"PONG"` printed |
| 2 | `THROTTLE 40` | ACTUATE prints ~40% duty |
| 3 | `STEER -60` | STEER logged, no effect on output |
| 4 | `THROTTLE 90` then immediately `BRAKE 100` | Output snaps to 0 — BRAKE overrides |
| 5 | `BRAKE 0` then `THROTTLE 55` | Output resumes to ~55% |
| 6 | Type nothing for 600ms+ | `"LINK LOST - failing safe"` logged, output switches to blink pattern |
| 7 | `THROTTLE 20` | `"LINK RECOVERED"` logged, normal output resumes |
| 8 | `THROTTLE abc` (malformed) | Ignored/logged, no crash |
| 9 | Paste `BRAKE 100` and `THROTTLE 100` as two lines at once | BRAKE still wins |

While running the above, `STATUS` should keep printing once a second in the background the entire time, uninterrupted.

## Required Answers

**1. Why did you assign the task priorities the way you did?**

`COMMAND_RX` is highest priority (4) because it must never miss an incoming byte on UART, and especially must never drop a BRAKE command — this is the one task where any added latency is a real safety risk. `ACTUATE` is next (3) because it needs to react to queued commands promptly, especially cutting output to zero the instant a BRAKE arrives, but it can tolerate small delays that `COMMAND_RX` cannot. `WATCHDOG` is low priority (2) — it only needs to check timing roughly ten times a second, not sub-millisecond precision, but it still needs to run reliably enough to catch a real link loss. `STATUS` is lowest (1) because it is purely a diagnostic dashboard; if its once-a-second print is occasionally a little late under heavy load, nothing unsafe happens.

**2. Why does a stale or missing BRAKE matter more than a stale STEER? How does your watchdog design guarantee the system actually fails safe, rather than just going quiet?**

A stale STEER just means the vehicle continues turning the way it already was — not ideal, but not immediately dangerous by itself. A stale or missing BRAKE means the vehicle keeps moving under whatever throttle was last set, with no way to stop it — that is a direct safety hazard. My watchdog guarantees an actual fail-safe response, not silence, by actively forcing the output to a distinct blink pattern and logging "LINK LOST - failing safe" the moment 500ms passes with no new command — rather than simply leaving the last output value in place. It also actively recovers the instant a new valid command is parsed, so the system does not stay in fail-safe longer than necessary.

**3. What would you add or fix first if you had one more day?**

First, I would resolve the LED issue properly — either confirm the correct onboard LED pin for this specific board revision or wire up an external LED with a resistor, and restore the PWM output instead of the serial-print substitute. Second, I would add a shared `link_lost` flag so that `ACTUATE` skips writing to the output entirely while `WATCHDOG` owns fail-safe mode, closing a small race condition where a queued command processed during a link-loss blink could momentarily overwrite the blink pattern before the next watchdog tick corrects it. Third, I'd add basic unit tests for `Command_Parse()` covering more malformed-input edge cases.
