# Shake It Up!

## ECE3140 Final Project by ff267, yz2788

### [Check our Web-Page Report Here](https://pages.github.coecis.cornell.edu/ece3140-spr2026/ff267-yz2788/)


Just-Dance-style rhythm game. The FRDM-KL46Z board reads the on-board MMA8451 accelerometer, detects directional swings (U/D/L/R), and grades them against a beatmap streamed from a Python host over USB-CDC serial (or HC-06 Bluetooth). The host plays the song, draws falling arrows, and displays the score.

## Repo layout

```
MCUXpresso-Board/      C firmware for the FRDM-KL46Z (MCUXpresso IDE project)
  source/
    main.c             init + main loop (RX -> dispatch, accel -> DSP -> judge)
    drivers/
      uart_comm.c      UART0 driver (OpenSDA / USB or BT)
      proto.c          UART2 driver (HC-06 BT, debug-mode wire)
      mma8451.c        I2C accelerometer driver
      timer_pit.c      1-ms PIT tick (millis())
    app/
      config.h         BUILD_MODE selector (DEBUG / USB / BT)
      dsp_filter.c     EMA + state machine -> swing direction
      game_logic.c     16-slot beat queue, P/G/M judging, auto-miss
Python-PC/             pygame host
  main.py              Dispatch beats, drain results, render
  serial_manager.py    SerialDispatcher + PeriodicSync + SerialManager
  game_state.py        score / combo / accuracy
  audio_player.py      pygame.mixer wrapper, audio cursor as host clock
  beatmap.py           JSON loader -> [{t_ms, dir}, ...]
  ui_renderer.py       pygame screen: arrows, judgment line, end screen
  config.py            paths, ports, timing windows (mirrors firmware)
  assets/              PNGs, song1.mp3, song1.json
  demo.ipynb           Game Sandbox
  test.ipynb           Test suite: latency, drift, false-trigger, accuracy, timing
```

## Wire protocol (board <-> host)

| | |
|---|---|
| `SYN\n` | host -> board: clock-sync request |
| `ACK:<board_ms>\n` | board -> host: SYN reply |
| `B:<idx>:<dir>:<board_t_ms>\n` | host -> board: queue a beat |
| `H:<idx>:<grade>:<actual_ms>\n` | board -> host: graded P/G/M |
| `M:<idx>\n` | board -> host: auto-miss timeout |
| `BUSY:<idx>\n` | board -> host: queue full, dropped |
| `SWING:<dir>:<ms>\n` | board -> host: raw swing detected |

Beats are scheduled in *board time*; `PeriodicSync` issues SYN every 5 s and EMA-smooths the host↔board offset.

## Build modes (firmware)

Set in [`MCUXpresso-Board/source/app/config.h`](MCUXpresso-Board/source/app/config.h):

| `BUILD_MODE` | Game wire | When to use |
|---|---|---|---|
| `MODE_DEBUG` | UART2 (J3[7]/J3[9]) | For development of bluetooth, both buses lives|
| `MODE_USB`   | UART0 (USB-OpenSDA) | For development other than bluetooth |
| `MODE_BT`    | UART0 (J1[2]/J1[4]) | Real Game Play |


## Build and flash the firmware

1. Open `MCUXpresso-Board/` as a project in MCUXpressoIDE.
2. (Optional) edit `source/app/config.h` to choose `BUILD_MODE` (default `MODE_USB`).
3. **Project → Build** (⌘B), then **Run** (or **Debug** then terminate) to flash via OpenSDA.

## Run the host

```bash
cd Python-PC
pip install pyserial pygame
# Edit config.py: set SERIAL_PORT to your /dev/cu.* and BAUD_RATE to match BUILD_MODE
python3 main.py
```

Controls: **SPACE** start / replay, **ESC** quit, **F11** toggle fullscreen. Swing the board on each falling arrow as it crosses the judgment line. Per-beat results print to the terminal; the end screen shows score, max combo, accuracy, and a letter rank.

## Timing windows
Current values: `PERFECT=500ms`, `GOOD=1000ms`, `MISS=1500ms` (correct dir but late = `M`, no swing in window = `TIMEOUT`). 