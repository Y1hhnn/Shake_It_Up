"""Background music player]"""
import os

import pygame

class AudioPlayer:
    def __init__(self):
        self._loaded = False
        self._start_tick_ms = None

    def load(self, path):
        if not os.path.exists(path):
            print(f"[Audio] File not found: {path}  (running without music)")
            self._loaded = False
            return False
        try:
            if not pygame.mixer.get_init():
                pygame.mixer.init()
            pygame.mixer.music.load(path)
            self._loaded = True
            print(f"[Audio] Loaded {path}")
            return True
        except pygame.error as e:
            print(f"[Audio] Failed to load: {e}")
            self._loaded = False
            return False

    def play(self):
        if self._loaded:
            pygame.mixer.music.play()
        self._start_tick_ms = pygame.time.get_ticks()

    def stop(self):
        if self._loaded:
            pygame.mixer.music.stop()
        self._start_tick_ms = None

    def is_started(self):
        return self._start_tick_ms is not None

    def get_pos_ms(self):
        """Return ms elapsed since play(). Falls back to wall-clock if no audio."""
        if self._start_tick_ms is None:
            return 0
        if self._loaded:
            pos = pygame.mixer.music.get_pos()
            if pos >= 0:
                return pos
        return pygame.time.get_ticks() - self._start_tick_ms
