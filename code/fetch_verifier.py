"""Download the public EinsteinArena verifier for 'sidon-45-set' next to this file."""

import json
import urllib.request
from pathlib import Path

URL = "https://einsteinarena.com/api/problems/sidon-45-set"

if __name__ == "__main__":
    with urllib.request.urlopen(URL, timeout=60) as r:
        problem = json.load(r)
    out = Path(__file__).with_name("arena_verifier.py")
    out.write_text(problem["verifier"])
    print(f"saved {out}")
