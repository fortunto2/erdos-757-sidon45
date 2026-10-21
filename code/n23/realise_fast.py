"""realise.py's algorithm with the exact nullspace done by FLINT (python-flint) instead of Fractions.

No pruning, every SDR mu is tried: random integer point of V, violation(), and a bad form that
vanishes at the point is tested on the whole integer basis of V (exact). Independent of rt.cpp
(no mod-p, no DFS, no Bareiss); used to cross-check rt on a random sample of n=23 classes,
where realise.py itself needs > 10 min per class.

usage: uv run --with python-flint python realise_fast.py FINAL RT_OUT [--k 200] [--seed 1] [--jobs 4]
       (RT_OUT empty string: run every class of FINAL and print counts)
"""

import argparse
import importlib.util
import json
import random
import sys
import time
from multiprocessing import Pool
from pathlib import Path

import flint

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("realise", HERE.parent / "realise.py")
R = importlib.util.module_from_spec(spec)
spec.loader.exec_module(R)


def basis(n, edges_mu):
    rows = []
    for e, b in edges_mu:
        a, c = [v for v in e if v != b]
        r = [0] * n
        r[a] += 1
        r[c] += 1
        r[b] -= 2
        rows.append(r)
    X, k = flint.fmpz_mat(rows).nullspace()
    return [[int(X[i, j]) for i in range(n)] for j in range(k)]


def try_mu(n, edges_mu, rng, tries=6):
    Z = basis(n, edges_mu)
    for _ in range(tries):
        coef = [rng.randint(-10**6, 10**6) for _ in Z]
        xs = [sum(c * z[i] for c, z in zip(coef, Z)) for i in range(n)]
        bad = R.violation(xs, edges_mu)
        if bad is None:
            return True
        if all(sum(v * z[i] for i, v in bad.items()) == 0 for z in Z):
            return False
    raise RuntimeError("undecided after random draws")


def count(args):
    idx, line, seed = args
    parts = line.split()
    n = int(parts[0])
    edges = [tuple(map(int, p.split(","))) for p in parts[3:]]
    rng = random.Random(seed + idx)
    mus = R.sdrs(edges, n)
    ok = sum(try_mu(n, list(zip(edges, mu)), rng) for mu in mus)
    return idx, len(mus), ok


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("final")
    ap.add_argument("rtout")
    ap.add_argument("--k", type=int, default=200)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--jobs", type=int, default=4)
    a = ap.parse_args()
    lines = [ln for ln in Path(a.final).read_text().split("\n") if ln.strip()]
    rt = {}
    if a.rtout:
        for ln in Path(a.rtout).read_text().split("\n"):
            p = ln.split()
            if len(p) >= 4 and p[0].isdigit():
                rt[int(p[0])] = int(p[3])
        idxs = sorted(random.Random(a.seed).sample(range(len(lines)), min(a.k, len(lines))))
    else:
        idxs = list(range(len(lines)))
    t0 = time.time()
    bad = 0
    with Pool(a.jobs) as pool:
        for idx, nmu, ok in pool.imap_unordered(count, [(i, lines[i], a.seed) for i in idxs]):
            mine = rt.get(idx)
            agree = mine is None or ok == mine
            bad += not agree
            print(json.dumps({"idx": idx, "sdrs": nmu, "flint": ok, "rt": mine, "agree": agree}), flush=True)
    print(f"{len(idxs)} classes, {bad} disagree, {time.time() - t0:.0f}s", flush=True)
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
