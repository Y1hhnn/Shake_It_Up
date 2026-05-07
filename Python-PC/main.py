""" Game entry point """
import os
import sys
import time
import pygame

from audio_player import AudioPlayer
from beatmap import load_beatmap
from config import (
    DEFAULT_BEATMAP,
    DIR_NAMES,
    MISS_WINDOW_MS,
    SONGS_DIR,
    TARGET_DISPATCH_LEAD_MS,
)
from game_state import GameState
from serial_manager import SerialManager
from ui_renderer import UIRenderer


PHASE_WAITING = 0
PHASE_PLAYING = 1
PHASE_ENDED   = 2


def _print_summary(state):
    s = state.summary()
    print()
    print("=" * 42)
    print("Final Score")
    print(f"  Score:       {s['score']}")
    print(f"  Max combo:   {s['max_combo']}")
    print(f"  Perfect:     {s['perfect']}")
    print(f"  Good:        {s['good']}")
    print(f"  Miss/Wrong:  {s['miss_wrong']}")
    print(f"  Timeout:     {s['miss_timeout']}")
    print(f"  Accuracy:    {s['accuracy']:.1f}%")
    print("=" * 42)


# Map grade letter (or "TIMEOUT") -> human label printed in the terminal.
_LABELS = {"P": "PERFECT", "G": "GOOD   ", "M": "MISS   ", "T": "TIMEOUT"}


def _log_event(label, idx, direction, state):
    name = DIR_NAMES.get(direction, direction or "?")
    print(f"  {_LABELS[label]}  {name:5s}  #{idx:<3d}  "
          f"combo={state.combo}  score={state.score}")


def _handle_board_event(ev, notes, state, ui, consumed):
    """Apply one event"""
    t = ev["type"]
    idx = ev.get("idx")

    if t == "hit":
        grade = state.on_event(ev)        # 'P' / 'G' / 'M'
        consumed.add(idx)
        if grade:
            ui.show_feedback(grade)
            d = notes[idx].dir if 0 <= idx < len(notes) else None
            _log_event(grade, idx, d, state)
    elif t == "miss":
        state.on_event(ev)                # always returns 'M'
        consumed.add(idx)
        ui.show_feedback("M")
        d = notes[idx].dir if 0 <= idx < len(notes) else None
        _log_event("T", idx, d, state)
    elif t == "busy":
        consumed.add(idx)
        print(f"  BUSY     #{idx}  -- queue overflow")
    elif t == "boot":
        return False
    # 'swing' is decoration; ignore here (no scoring impact).
    return True


def main():
    pygame.init()
    pygame.mixer.init()

    if not os.path.exists(DEFAULT_BEATMAP):
        print(f"[Game] Beatmap not found: {DEFAULT_BEATMAP}")
        sys.exit(1)

    beatmap = load_beatmap(DEFAULT_BEATMAP)
    notes = beatmap.notes
    print(f"[Game] '{beatmap.title}' -- {len(notes)} notes")

    # Connect to the board
    ser = SerialManager()
    if not ser.connect():
        print("[Game] Could not open serial port.")
        sys.exit(1)

    audio = AudioPlayer()
    if beatmap.song_file:
        audio.load(os.path.join(SONGS_DIR, beatmap.song_file))

    state = GameState()
    ui = UIRenderer()

    # Per-song state
    dispatch_idx = 0     
    consumed = set()    
    phase = PHASE_WAITING

    print(" Press SPACE to start, ESC to quit.")

    def begin_song():
        nonlocal dispatch_idx
        state.reset()
        consumed.clear()
        dispatch_idx = 0
        audio.stop()
        ser.poll_events()                 # drain anything left over
        audio.play()

    running = True
    while running:
        # --- Pygame events ---
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_ESCAPE:
                    running = False
                elif event.key == pygame.K_F11:
                    ui.toggle_fullscreen()
                elif event.key == pygame.K_SPACE:
                    if phase in (PHASE_WAITING, PHASE_ENDED):
                        begin_song()
                        phase = PHASE_PLAYING
                        print("[Game] Started")

        song_pos_ms = audio.get_pos_ms() if phase == PHASE_PLAYING else 0

        if phase == PHASE_PLAYING:
            host_now_ms = time.time() * 1000.0
            song_start_host_ms = host_now_ms - song_pos_ms
            while (
                dispatch_idx < len(notes)
                and notes[dispatch_idx].t - song_pos_ms <= TARGET_DISPATCH_LEAD_MS
            ):
                n = notes[dispatch_idx]
                host_target_ms = song_start_host_ms + n.t
                ser.send_beat(dispatch_idx, n.dir, host_target_ms)
                dispatch_idx += 1

            # Drain board events
            board_reset = False
            for ev in ser.poll_events():
                if not _handle_board_event(ev, notes, state, ui, consumed):
                    board_reset = True
                    break

            if board_reset:
                print("  [Game] Board reset mid-song -- ending. ")
                phase = PHASE_ENDED
                audio.stop()
                _print_summary(state)
                continue

            # End-of-song detection
            if (
                dispatch_idx >= len(notes)
                and notes
                and song_pos_ms - notes[-1].t > MISS_WINDOW_MS + 1500
            ):
                phase = PHASE_ENDED
                audio.stop()
                print("[Game] Song complete")
                _print_summary(state)

        # Render
        if phase == PHASE_ENDED:
            ui.draw_end_screen(state, song_title=beatmap.title)
        else:
            prompt = "Press SPACE to start" if phase == PHASE_WAITING else None
            ui.draw(
                song_pos_ms=song_pos_ms,
                notes=notes,
                consumed=consumed,
                game_state=state,
                started=(phase == PHASE_PLAYING),
                prompt_text=prompt,
            )
        ui.tick()

    # Teardown
    ser.disconnect()
    audio.stop()
    pygame.quit()


if __name__ == "__main__":
    main()
