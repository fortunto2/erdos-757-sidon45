> Paths in this file are from the working repo. Here: `../out23fix/final.txt` = `n23-level23-classes.txt.gz`, `out/rt-n23.txt` = `rt-n23.txt.gz`, `out/sample200.jsonl` = `sample200.jsonl`, `rt.cpp`/`realise_fast.py`/`verify.py` = `code/n23/`, `../maxchain-review.md` = `docs/reviews/`. The chain review is done: `docs/reviews/review23.md` (GO).

# n = 23 realisability of the max-degree chain survivors (Erdős #757)

**Verdict: 0 of 86,094 classes in `../out23fix/final.txt` are realisable.** No 23-point
(4,5)-set with h = 13 comes out of the (23, τ≥10, m 19..21, DCAP 6) chain. Combined with the
chain itself (pending its own completeness review, `../maxchain-review.md`): no (4,5)-set with
h/n < 4/7 on 23 points.

## Method (`rt.cpp`)

Same mathematics as `../realise.py`: (H, mu) is realised iff no bad form (x_i−x_j; non-edge
3-AP x_a+x_c−2x_b; weak-Sidon x_i+x_j−x_k−x_l) vanishes on V = solution space of the AP
equations. Instead of enumerating all SDRs (73k–253k per n=23 class), a DFS assigns middles one
edge at a time; V only shrinks, so a bad form vanishing on a partial V kills the subtree.
Vanishing is spotted mod 2^61−1 on two random points of V_p, and **every prune is confirmed
exactly over Q** (fraction-free Bareiss rank in `__int128`: f ∈ rowspan(R) ⇔ rank R = rank R+f;
all minors ≤ 6^11, no overflow). A mod-p false alarm is never used; there were 0 of them in
15,645,571 exact checks. A leaf that survives would be printed as a candidate for exact
construction (`verify.py candidates`); none survived.
Edge order: edges of a dense subset first (found by max-closure / min-cut), then greedy.

## Filters (diagnostic counts, all 86,094 classes)

| Property | Classes | Status |
|---|---|---|
| Subset S with \|E(S)\| ≥ \|S\|−1 (min-cut, forced edge) | 85,395 (99.2%) | not used as a kill: the edge-count form needs the AP rows of S to be independent, which I did not prove (e.g. the Fano plane with an SDR has rows summing to 0). The rank form, rank ≤ \|S\|−2 for each mu, is proven and is exactly what the DFS prunes on |
| No such subset | 699 | all die in the exact test anyway |
| HY-F7 (AG(2,3) minus 2 points) | 0 | chain enforces it at every level; detector fires on F7 itself (control) |
| Fano plane | 7 (idx 20474, 35445, 47336, 49486-49488, 84170) | the chain's "F7" is HY's F7, not Fano, so Fano copies pass the chain; they are unrealisable (Fano 0/24) |

Size of the smallest dense set found: 7 (686 classes), 9 (61,994), 10 (1,428), 11 (18,085),
13 (2,663), larger (539).

## Known-answer controls (all exact match)

| Input | Expected (realise.py) | rt.cpp | realise_fast.py |
|---|---|---|---|
| n=14 MMIN-11, 12 classes (`n14-realise.json`) | realisable-mu 0,282,446,0,430,506,569,698,498,560,702,0 | identical counts | identical counts and SDR totals |
| n=16 2 survivors | 0, 0 | 0, 0 | 0, 0 (5184, 5280 SDRs) |
| n=18 3 survivors | 0/11880, 0/11664, 0/12100 | 0,0,0 | identical |
| Fano / F7 / Fano−line / Pasch (`controls-small.txt`) | 0/24, 0/54, 0, 24/30 | 0,0,0,24 | identical |
| n=14 candidate path: `rt` (stop at first leaf) → `verify.py candidates` | 9 realisable | 9 R | 9/9 exact, (4,5) holds, h = 8 by MIS bitmask and brute force (8/14 = 4/7) |
| n=23 random 200 classes (seed 1) | — | 0 each | 0 each, 200/200 agree, 11,838,299 SDRs tried |

`realise_fast.py` is realise.py's own algorithm (every SDR, random point, exact vanishing on the
integer basis), with the nullspace by FLINT instead of Fractions; it shares `violation()` and
`sdrs()` with realise.py and nothing with rt.cpp. realise.py itself needs > 10 min for one n=23
class (253,440 SDRs), so the 200-class check against it directly was replaced by this.

## Runtime

rt.cpp: whole file in ~5 min wall on 4 cores under load 10–28 (avg 719 DFS nodes per class,
max 38,993). realise_fast sample: 400 s on 5 cores. realise.py: > 10 min per class.

## Files

- `rt.cpp` — DFS + mod-p + exact Bareiss; `clang++ -O2 -std=c++17 -o rt rt.cpp`
- `realise_fast.py` — independent exhaustive cross-check (`uv run --with python-flint`)
- `verify.py` — exact construction and h checks (MIS bitmask + brute force) for any R candidate
- `out/rt-n23.txt` — per class: idx verdict nodes surviving_leaves dense fano hy_f7 |S|
- `out/sample200.jsonl` — the 200-class cross-check
