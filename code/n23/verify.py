"""Exact (over Q) cross-checks for rt.cpp, reusing ../realise.py.

usage:
  verify.py sample FINAL RT_OUT [--k 200] [--seed 1] [--jobs 3]
      random sample of classes: realise.py verdict + realisable-mu count vs rt --count
  verify.py candidates FINAL RT_OUT [--json OUT]
      every class rt calls R: exact try_mu on its mu, build integers, verify (4,5) and h twice
"""

import argparse
import importlib.util
import itertools
import json
import random
import sys
import time
from multiprocessing import Pool
from pathlib import Path

HERE = Path(__file__).resolve().parent
spec = importlib.util.spec_from_file_location("realise", HERE.parent / "realise.py")
R = importlib.util.module_from_spec(spec)
spec.loader.exec_module(R)


def parse(line):
    parts = line.split()
    n, m, tau = map(int, parts[:3])
    return n, m, tau, [tuple(map(int, p.split(","))) for p in parts[3:]]


def realise_count(args):
    idx, line, seed = args
    n, m, tau, edges = parse(line)
    rng = random.Random(seed)
    ok = 0
    mus = R.sdrs(edges, n)
    for mu in mus:
        xs, _ = R.try_mu(n, list(zip(edges, mu)), rng)
        ok += xs is not None
    return idx, len(mus), ok


def is_45(xs):
    for four in itertools.combinations(sorted(xs), 4):
        if len({b - a for a, b in itertools.combinations(four, 2)}) < 5:
            return False
    return True


def h_bitmask(xs):
    """Largest Sidon subset: branch and bound over conflicts (a 3-AP or a+b=c+d is a conflict set)."""
    n = len(xs)
    pos = {x: i for i, x in enumerate(xs)}
    bad = set()
    for i, j in itertools.combinations(range(n), 2):
        s = xs[i] + xs[j]
        if s % 2 == 0 and s // 2 in pos:
            bad.add(frozenset((i, j, pos[s // 2])))
    sums = {}
    for i, j in itertools.combinations(range(n), 2):
        sums.setdefault(xs[i] + xs[j], []).append((i, j))
    for lst in sums.values():
        for (a, b), (c, d) in itertools.combinations(lst, 2):
            bad.add(frozenset((a, b, c, d)))
    masks = [sum(1 << v for v in b) for b in bad]
    best = 0
    for S in range(1 << n):  # n <= 23: 8M masks, acceptable once per candidate
        if bin(S).count("1") <= best:
            continue
        if all(S & mk != mk for mk in masks):
            best = bin(S).count("1")
    return best


def h_brute(xs):
    n = len(xs)
    for size in range(n, 0, -1):
        for sub in itertools.combinations(sorted(xs), size):
            sums = set()
            okk = True
            for i, a in enumerate(sub):
                for b in sub[i:]:
                    if a + b in sums:
                        okk = False
                        break
                    sums.add(a + b)
                if not okk:
                    break
            if okk:
                return size
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["sample", "candidates"])
    ap.add_argument("final")
    ap.add_argument("rtout")
    ap.add_argument("--k", type=int, default=200)
    ap.add_argument("--seed", type=int, default=1)
    ap.add_argument("--jobs", type=int, default=3)
    ap.add_argument("--json")
    a = ap.parse_args()
    lines = [ln for ln in Path(a.final).read_text().split("\n") if ln.strip()]
    rt = {}
    for ln in Path(a.rtout).read_text().split("\n"):
        p = ln.split()
        if len(p) >= 8 and p[0].isdigit():
            rt[int(p[0])] = p
    if a.mode == "sample":
        idxs = sorted(random.Random(a.seed).sample(sorted(rt), min(a.k, len(rt))))
        t0 = time.time()
        bad = 0
        with Pool(a.jobs) as pool:
            for idx, nmu, ok in pool.imap_unordered(realise_count, [(i, lines[i], a.seed) for i in idxs]):
                mine = int(rt[idx][3])
                agree = ok == mine
                bad += not agree
                print(json.dumps({"idx": idx, "sdrs": nmu, "realise_py": ok, "rt": mine, "agree": agree}), flush=True)
        print(f"sample {len(idxs)}: {len(idxs) - bad} agree, {bad} disagree, {time.time() - t0:.0f}s", flush=True)
        return 1 if bad else 0
    out = []
    rng = random.Random(a.seed)
    for idx, p in sorted(rt.items()):
        if p[1] != "R":
            continue
        n, m, tau, edges = parse(lines[idx])
        mu = [int(v) for v in p[8].split("=")[1].split(",")]
        xs, dim = R.try_mu(n, list(zip(edges, mu)), rng)
        rec = {"idx": idx, "n": n, "m": m, "tau": tau, "edges": edges, "mu": mu, "exact_realisable": xs is not None}
        if xs is not None:
            best = R.shrink(n, list(zip(edges, mu)), xs, rng)
            rec.update(elements=best, is_45=is_45(best), h_bitmask=h_bitmask(best), h_brute=h_brute(best))
        out.append(rec)
        print(json.dumps({k: v for k, v in rec.items() if k != "edges"}), flush=True)
    print(f"{len(out)} candidates, {sum(r['exact_realisable'] for r in out)} exactly realisable")
    if a.json:
        Path(a.json).write_text(json.dumps(out, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
