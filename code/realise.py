"""Stage B: realisability of abstract hypergraphs as (4,5)-sets (spec: docs/method-notes.md).

For H with a distinct-middle assignment mu, the AP equations x_a + x_c = 2 x_b span W; the
solution space is V = W^perp. H is realised (H(A) = H exactly, A weak Sidon, distinct points)
iff no "bad form" vanishes identically on V. We draw random integer points of V and check
the actual set; when a bad form vanishes at the point we test whether it vanishes on all of
V (exact, over Q) — that kills mu; otherwise we redraw.

usage: realise.py FINAL_TXT [--out JSON] [--limit-mu N]
"""

import argparse
import importlib.util
import itertools
import json
import random
import sys
import time
from fractions import Fraction
from math import gcd
from pathlib import Path



def load_verifier():
    spec = importlib.util.spec_from_file_location("verifier", Path(__file__).with_name("arena_verifier.py"))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)
    return mod


def nullspace(rows, n):
    """Integer basis of {x : r.x = 0 for r in rows}, as list of n-vectors."""
    M = [[Fraction(v) for v in r] for r in rows]
    piv = []
    r = 0
    for c in range(n):
        p = next((i for i in range(r, len(M)) if M[i][c] != 0), None)
        if p is None:
            continue
        M[r], M[p] = M[p], M[r]
        inv = 1 / M[r][c]
        M[r] = [v * inv for v in M[r]]
        for i in range(len(M)):
            if i != r and M[i][c] != 0:
                f = M[i][c]
                M[i] = [a - f * b for a, b in zip(M[i], M[r])]
        piv.append(c)
        r += 1
        if r == len(M):
            break
    free = [c for c in range(n) if c not in piv]
    basis = []
    for f in free:
        v = [Fraction(0)] * n
        v[f] = Fraction(1)
        for i, c in enumerate(piv):
            v[c] = -M[i][f]
        den = 1
        for x in v:
            den = den * x.denominator // gcd(den, x.denominator)
        basis.append([int(x * den) for x in v])
    return basis


def ap_triples(xs):
    pos = {x: i for i, x in enumerate(xs)}
    out = set()
    for i, j in itertools.combinations(range(len(xs)), 2):
        s = xs[i] + xs[j]
        if s % 2 == 0 and s // 2 in pos:
            out.add((frozenset((i, j, pos[s // 2])), pos[s // 2]))
    return out


def violation(xs, edges_mu):
    """None if xs realises (H, mu) exactly; else a bad linear form (dict index->coef)."""
    n = len(xs)
    seen = {}
    for i in range(n):
        if xs[i] in seen:
            return {i: 1, seen[xs[i]]: -1}
        seen[xs[i]] = i
    want = {(frozenset(e), b) for e, b in edges_mu}
    got = ap_triples(xs)
    for t, b in got - want:
        a, c = sorted(t - {b})
        return {a: 1, c: 1, b: -2}
    for t, b in want - got:  # cannot happen on V, equations hold
        raise AssertionError("AP equation not satisfied")
    sums = {}
    for i, j in itertools.combinations(range(n), 2):
        s = xs[i] + xs[j]
        if s in sums:
            k, l = sums[s]
            if len({i, j, k, l}) == 4:
                return {i: 1, j: 1, k: -1, l: -1}
        else:
            sums[s] = (i, j)
    return None


def try_mu(n, edges_mu, rng, tries=6):
    rows = []
    for e, b in edges_mu:
        a, c = [v for v in e if v != b]
        r = [0] * n
        r[a] += 1
        r[c] += 1
        r[b] -= 2
        rows.append(r)
    Z = nullspace(rows, n)
    for _ in range(tries):
        coef = [rng.randint(-10**6, 10**6) for _ in Z]
        xs = [sum(c * z[i] for c, z in zip(coef, Z)) for i in range(n)]
        bad = violation(xs, edges_mu)
        if bad is None:
            return xs, len(Z)
        if all(sum(bad.get(i, 0) * z[i] for i in range(n)) == 0 for z in Z):
            return None, len(Z)
    raise RuntimeError("undecided after random draws")


def sdrs(edges, n):
    out = []
    used = [False] * n
    mu = []

    def rec(i):
        if i == len(edges):
            out.append(list(mu))
            return
        for b in edges[i]:
            if not used[b]:
                used[b] = True
                mu.append(b)
                rec(i + 1)
                mu.pop()
                used[b] = False

    rec(0)
    return out


def normalise(xs):
    lo = min(xs)
    ys = [x - lo for x in xs]
    g = 0
    for y in ys:
        g = gcd(g, y)
    return sorted(y // g for y in ys)


def shrink(n, edges_mu, xs, rng, rounds=300):
    """Re-draw small points of V to get a set with small diameter."""
    best = normalise(xs)
    rows = []
    for e, b in edges_mu:
        a, c = [v for v in e if v != b]
        r = [0] * n
        r[a] += 1
        r[c] += 1
        r[b] -= 2
        rows.append(r)
    Z = nullspace(rows, n)
    for R in (2, 3, 5, 10, 30, 100):
        for _ in range(rounds):
            coef = [rng.randint(-R, R) for _ in Z]
            ys = [sum(c * z[i] for c, z in zip(coef, Z)) for i in range(n)]
            if violation(ys, edges_mu) is None:
                ys = normalise(ys)
                if ys[-1] < best[-1]:
                    best = ys
    return best


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("final")
    ap.add_argument("--out")
    ap.add_argument("--seed", type=int, default=1)
    args = ap.parse_args()
    rng = random.Random(args.seed)
    ver = load_verifier()
    results = []
    t0 = time.time()
    for line in Path(args.final).read_text().split("\n"):
        if not line.strip():
            continue
        parts = line.split()
        n, m, tau = map(int, parts[:3])
        edges = [tuple(map(int, p.split(","))) for p in parts[3:]]
        mus = sdrs(edges, n)
        ok = []
        dims = set()
        for mu in mus:
            xs, dim = try_mu(n, list(zip(edges, mu)), rng)
            dims.add(dim)
            if xs is not None:
                ok.append((mu, xs))
        rec = {"n": n, "m": m, "tau": tau, "edges": edges, "sdrs": len(mus),
               "realisable_mu": len(ok), "dims": sorted(dims)}
        if ok:
            mu, xs = ok[0]
            best = shrink(n, list(zip(edges, mu)), xs, rng)
            score = ver.evaluate({"elements": best}) if best[-1] <= 10**9 else None
            rec.update(mu=mu, elements=best, score=score)
        results.append(rec)
        print(json.dumps({k: v for k, v in rec.items() if k != "edges"}), flush=True)
    print(f"{len(results)} hypergraphs, {sum(r['realisable_mu'] > 0 for r in results)} realisable, "
          f"{time.time() - t0:.1f}s", flush=True)
    if args.out:
        Path(args.out).write_text(json.dumps(results, indent=1))


if __name__ == "__main__":
    sys.exit(main())
