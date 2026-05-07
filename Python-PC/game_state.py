"""Score / combo / counters"""

from config import SCORE_GOOD, SCORE_PERFECT


class GameState:
    def __init__(self):
        self.reset()

    def reset(self):
        self.score = 0
        self.combo = 0
        self.max_combo = 0
        self.counts = {"P": 0, "G": 0, "M": 0, "TIMEOUT": 0}

    def on_event(self, event):
        t = event.get("type")

        if t == "hit":
            grade = event.get("grade")
            if grade == "P":
                self.counts["P"] += 1
                self.combo += 1
                self.max_combo = max(self.max_combo, self.combo)
                self.score += SCORE_PERFECT
                return "P"
            if grade == "G":
                self.counts["G"] += 1
                self.combo += 1
                self.max_combo = max(self.max_combo, self.combo)
                self.score += SCORE_GOOD
                return "G"
            # 'M' = wrong direction or out-of-window swing
            self.counts["M"] += 1
            self.combo = 0
            return "M"

        if t == "miss":
            self.counts["TIMEOUT"] += 1
            self.combo = 0
            return "M"

        return None  # busy / swing / boot / unknown -- no scoring impact

    def total_targets(self):
        c = self.counts
        return c["P"] + c["G"] + c["M"] + c["TIMEOUT"]

    def accuracy_pct(self):
        total = self.total_targets()
        if not total:
            return 0.0
        return 100.0 * (self.counts["P"] + self.counts["G"]) / total

    def summary(self):
        return {
            "score": self.score,
            "max_combo": self.max_combo,
            "perfect": self.counts["P"],
            "good": self.counts["G"],
            "miss_wrong": self.counts["M"],
            "miss_timeout": self.counts["TIMEOUT"],
            "accuracy": self.accuracy_pct(),
        }
