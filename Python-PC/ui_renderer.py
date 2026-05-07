"""Pygame renderer"""
import os

import pygame

from config import (
    ARROW_IMGS,
    ARROW_SPAWN_Y,
    ARROW_TRAVEL_MS,
    BG_IMG,
    FEEDBACK_DURATION_S,
    FEEDBACK_IMGS,
    FPS,
    FULLSCREEN,
    HEIGHT,
    LANE_X,
    MISS_WINDOW_MS,
    TIMING_LINE_Y,
    WIDTH,
)


def _load_alpha(path, scale=1.0):
    img = pygame.image.load(path).convert_alpha()
    w, h = img.get_size()
    return pygame.transform.smoothscale(img, (int(w * scale), int(h * scale)))


class UIRenderer:
    def __init__(self):
        self._is_fullscreen = bool(FULLSCREEN)
        self._apply_display_mode()
        pygame.display.set_caption("Shake It Up!")
        self.clock = pygame.time.Clock()

        if os.path.exists(BG_IMG):
            self.bg = pygame.transform.scale(
                pygame.image.load(BG_IMG).convert(), (WIDTH, HEIGHT)
            )
        else:
            self.bg = None

        self.arrows = {
            d: _load_alpha(p, 0.22) for d, p in ARROW_IMGS.items() if os.path.exists(p)
        }
        self.feedback_imgs = {
            g: _load_alpha(p, 0.35)
            for g, p in FEEDBACK_IMGS.items()
            if os.path.exists(p)
        }

        self.font = pygame.font.SysFont("Arial", 28, bold=True)
        self.small_font = pygame.font.SysFont("Arial", 20)
        self.title_font = pygame.font.SysFont("Arial", 60, bold=True)
        self.rank_font = pygame.font.SysFont("Arial", 110, bold=True)
        self.stat_font = pygame.font.SysFont("Arial", 30, bold=True)
        self.label_font = pygame.font.SysFont("Arial", 22)

        self._feedback_grade = None
        self._feedback_until_ms = 0

    def _apply_display_mode(self):
        scaled_flags = pygame.SCALED
        if self._is_fullscreen:
            scaled_flags |= pygame.FULLSCREEN
        try:
            self.screen = pygame.display.set_mode((WIDTH, HEIGHT), scaled_flags)
        except pygame.error:
            plain_flags = pygame.FULLSCREEN if self._is_fullscreen else 0
            self.screen = pygame.display.set_mode((WIDTH, HEIGHT), plain_flags)

    def toggle_fullscreen(self):
        self._is_fullscreen = not self._is_fullscreen
        self._apply_display_mode()

    def show_feedback(self, grade):
        if grade not in self.feedback_imgs:
            return
        self._feedback_grade = grade
        self._feedback_until_ms = pygame.time.get_ticks() + int(
            FEEDBACK_DURATION_S * 1000
        )

    def draw(self, song_pos_ms, notes, consumed, game_state, started, prompt_text=None):
        # Background
        if self.bg:
            self.screen.blit(self.bg, (0, 0))
        else:
            self.screen.fill((20, 20, 30))

        # Judgment line
        pygame.draw.line(
            self.screen,
            (255, 40, 200),
            (120, TIMING_LINE_Y),
            (880, TIMING_LINE_Y),
            6,
        )

        # Falling arrows: each note that's currently "in flight"
        if started:
            for idx, note in enumerate(notes):
                if idx in consumed:
                    continue
                dt = note.t - song_pos_ms
                if dt > ARROW_TRAVEL_MS:
                    continue  # not yet spawned
                if dt < -MISS_WINDOW_MS:
                    continue  # already past judgment + miss window
                progress = 1.0 - (dt / ARROW_TRAVEL_MS)
                y = ARROW_SPAWN_Y + (TIMING_LINE_Y - ARROW_SPAWN_Y) * progress
                x = LANE_X.get(note.dir, WIDTH // 2)
                img = self.arrows.get(note.dir)
                if img:
                    rect = img.get_rect(center=(x, int(y)))
                    self.screen.blit(img, rect)

        # Hit feedback overlay
        now_ms = pygame.time.get_ticks()
        if self._feedback_grade and now_ms < self._feedback_until_ms:
            img = self.feedback_imgs.get(self._feedback_grade)
            if img:
                rect = img.get_rect(center=(WIDTH // 2, 220))
                self.screen.blit(img, rect)

        # HUD
        hud = self.font.render(
            f"Score: {game_state.score}   Combo: {game_state.combo}   "
            f"Max: {game_state.max_combo}",
            True,
            (255, 255, 255),
        )
        self.screen.blit(hud, (20, 20))

        c = game_state.counts
        sub = self.small_font.render(
            f"P:{c['P']}  G:{c['G']}  M:{c['M']}  Timeout:{c['TIMEOUT']}  "
            f"Acc:{game_state.accuracy_pct():.0f}%",
            True,
            (220, 220, 220),
        )
        self.screen.blit(sub, (20, 55))

        # Pre-start prompt
        if prompt_text and not started:
            label = self.font.render(prompt_text, True, (255, 255, 255))
            rect = label.get_rect(center=(WIDTH // 2, HEIGHT // 2))
            bg_rect = rect.inflate(40, 20)
            pygame.draw.rect(self.screen, (0, 0, 0), bg_rect)
            pygame.draw.rect(self.screen, (255, 40, 200), bg_rect, 3)
            self.screen.blit(label, rect)

        pygame.display.flip()

    def draw_end_screen(self, game_state, song_title=None):
        """Render the post-song results screen.

        Layout: dim background, title, big rank letter, stats list,
        and a hint at the bottom for restart / quit.
        """
        if self.bg:
            self.screen.blit(self.bg, (0, 0))
        else:
            self.screen.fill((20, 20, 30))

        # Dim overlay
        overlay = pygame.Surface((WIDTH, HEIGHT), pygame.SRCALPHA)
        overlay.fill((0, 0, 0, 170))
        self.screen.blit(overlay, (0, 0))

        s = game_state.summary()

        title = self.title_font.render("Song Complete!", True, (255, 80, 200))
        self.screen.blit(title, title.get_rect(center=(WIDTH // 2, 70)))

        if song_title:
            sub = self.label_font.render(song_title, True, (200, 200, 200))
            self.screen.blit(sub, sub.get_rect(center=(WIDTH // 2, 115)))

        rank, rank_color = _rank_for_accuracy(s["accuracy"])
        rank_text = self.rank_font.render(rank, True, rank_color)
        self.screen.blit(rank_text, rank_text.get_rect(center=(WIDTH // 2, 220)))

        rows = [
            ("Score",        f"{s['score']}",            (255, 255, 255)),
            ("Max Combo",    f"{s['max_combo']}",        (255, 255, 255)),
            ("Accuracy",     f"{s['accuracy']:.1f}%",    (255, 220, 0)),
            ("Perfect",      f"{s['perfect']}",          (160, 255, 160)),
            ("Good",         f"{s['good']}",             (160, 220, 255)),
            ("Miss / Wrong", f"{s['miss_wrong']}",       (255, 160, 160)),
            ("Timeout",      f"{s['miss_timeout']}",     (255, 160, 160)),
        ]
        label_x = WIDTH // 2 - 160
        value_x = WIDTH // 2 + 140
        y = 320
        for label, value, color in rows:
            l_surf = self.label_font.render(label, True, (210, 210, 210))
            v_surf = self.stat_font.render(value, True, color)
            self.screen.blit(l_surf, (label_x, y + 4))
            self.screen.blit(v_surf, v_surf.get_rect(topright=(value_x, y)))
            y += 38

        hint = self.label_font.render(
            "SPACE to play again   |   ESC to quit",
            True,
            (200, 200, 200),
        )
        self.screen.blit(hint, hint.get_rect(center=(WIDTH // 2, HEIGHT - 30)))

        pygame.display.flip()

    def tick(self):
        self.clock.tick(FPS)


def _rank_for_accuracy(accuracy):
    """Return (rank_letter, rgb_color) for the end-screen badge."""
    if accuracy >= 95:
        return "S", (255, 215, 0)    # gold
    if accuracy >= 85:
        return "A", (160, 255, 160)  # green
    if accuracy >= 70:
        return "B", (160, 220, 255)  # blue
    if accuracy >= 50:
        return "C", (255, 200, 100)  # orange
    return "D", (255, 120, 120)      # red
