"""Global configuration: serial port, timing, asset paths.
  MODE_DEBUG -> port = /dev/cu.HC-06...        baud = 38400
  MODE_USB   -> port = /dev/cu.usbmodemSDA...  baud = 115200
  MODE_BT    -> port = /dev/cu.HC-06...        baud = 115200
"""
import os

# --- Project paths ---
BASE_DIR     = os.path.dirname(os.path.abspath(__file__))
ASSETS_DIR   = os.path.join(BASE_DIR, "assets")
SONGS_DIR    = os.path.join(ASSETS_DIR, "songs")
BEATMAPS_DIR = os.path.join(ASSETS_DIR, "beatmaps")

DEFAULT_BEATMAP = os.path.join(BEATMAPS_DIR, "song1.json")

# --- Serial ---
SERIAL_PORT    = "/dev/cu.usbmodemSDA6C1C1E501"
BAUD_RATE      = 115200
SERIAL_TIMEOUT = 0.05

# --- Clock sync ---
PERIODIC_SYNC_INTERVAL_S = 5.0
PERIODIC_SYNC_ALPHA      = 0.3

# --- Display ---
WIDTH      = 1000
HEIGHT     = 650
FPS        = 60
FULLSCREEN = False         

# --- Game timing (must match firmware app/game_logic.h) ---
PERFECT_WINDOW_MS = 500
GOOD_WINDOW_MS    = 1000
MISS_WINDOW_MS    = 2000

# --- Beat scheduling ---
ARROW_TRAVEL_MS = 1500

TARGET_DISPATCH_LEAD_MS = 2000

# --- UI layout ---
TIMING_LINE_Y = 500
ARROW_SPAWN_Y = 0

LANE_X = {
    "L": 260,
    "D": 420,
    "U": 580,
    "R": 740,
}

FEEDBACK_DURATION_S = 0.8

# --- Scoring ---
SCORE_PERFECT = 100
SCORE_GOOD    = 50

# --- Asset paths ---
BG_IMG = os.path.join(ASSETS_DIR, "background.png")
ARROW_IMGS = {
    "L": os.path.join(ASSETS_DIR, "left.png"),
    "D": os.path.join(ASSETS_DIR, "down.png"),
    "U": os.path.join(ASSETS_DIR, "up.png"),
    "R": os.path.join(ASSETS_DIR, "right.png"),
}
FEEDBACK_IMGS = {
    "P": os.path.join(ASSETS_DIR, "perfect.png"),
    "G": os.path.join(ASSETS_DIR, "good.png"),
    "M": os.path.join(ASSETS_DIR, "miss.png"),
}

DIR_NAMES = {"U": "Up", "D": "Down", "L": "Left", "R": "Right"}
