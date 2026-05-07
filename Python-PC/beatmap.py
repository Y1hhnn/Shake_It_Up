"""Beatmap Json Loader"""
import json
import os
from dataclasses import dataclass
from typing import List


@dataclass
class Note:
    t: int      # perfect_time in ms (relative to S/start)
    dir: str    # 'U', 'D', 'L', 'R'


@dataclass
class Beatmap:
    title: str
    song_file: str
    lead_in_ms: int
    notes: List[Note]


def load_beatmap(path) -> Beatmap:
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    notes = [
        Note(t=int(n["t"]), dir=n["dir"].upper())
        for n in data.get("notes", [])
        if n.get("dir", "").upper() in ("U", "D", "L", "R")
    ]
    notes.sort(key=lambda n: n.t)

    return Beatmap(
        title=data.get("title", os.path.basename(path)),
        song_file=data.get("song", ""),
        lead_in_ms=int(data.get("lead_in_ms", 0)),
        notes=notes,
    )
