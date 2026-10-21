# Erdős Problem #757: no (4,5)-set below 4/7 on fewer than 25 points

[![DOI](https://zenodo.org/badge/1426742593.svg)](https://doi.org/10.5281/zenodo.23494596)

Computer-assisted results on Erdős Problem #757, the constant C_5b in Tao's repository
of optimisation constants. The constant is the largest c such that every (4,5)-set of
n reals contains a Sidon subset of size c·n. A (4,5)-set is a set in which every 4
points determine at least 5 distinct differences. Ma–Tang (arXiv 2602.23282, 2026) prove
9/17 ≤ c ≤ 4/7; a self-similar family (erdosproblemaday.com, 2026-10-17) gives c ≤ 5/9,
with 65 points in its first set below 4/7.

## Results

1. **No (4,5)-set has (n, h) with n = 16, h ≤ 9; n = 18, h ≤ 10; n = 20, h ≤ 11;
   n = 22, h ≤ 12; or n = 23, h ≤ 13.** Here h is the size of the largest Sidon subset.
   By Henning–Yeo these are the only shapes with n ≤ 24 that could beat 4/7, so **every
   (4,5)-set with h/n < 4/7 has at least 25 points.** n = 16, 18, 20 are Henning–Yeo-free;
   n = 22, 23 use it (m range and degree cap). n = 23 leaves 86,094 hypergraph classes, none
   realisable (`results/n18-23/`).
2. **Classification at n = 14.** Every 14-point (4,5)-set with h = 8 has exactly 12
   three-term progressions. Its progression hypergraph is one of 9 isomorphism types,
   and each type is realised by an integer set (listed in `results/RESULTS.md`).
3. A 14-point set with h = 8 that differs from Ma–Tang's set:
   `0 323 416 468 509 517 537 557 566 575 613 646 717 1018`.

**Erratum (v1.1).** `chain.cpp`/`maxchain.cpp` before v1.1 stored at most 18 edges per
hypergraph. Runs with m ≤ 18 (all of n ≤ 18, the v1.0 results) were unaffected; an
internal n = 23 run had wrongly reported 0 classes. Fixed in v1.1 (`MAXE = 32` + guard),
n = 20 and 22 reproduced. Reviews: `docs/reviews/`.

## Method in one paragraph

For a (4,5)-set, h equals the independence number of its 3-AP hypergraph H (Ma–Tang,
Lemma 2.3). H is linear and F7-free (F7 = AG(2,3) minus two points), and every point is
the middle of at most one progression, so m ≤ n − 2. Henning and Yeo (Graphs and
Combinatorics 37, 2021) show 17τ ≤ 5n + 3m, which leaves the two shapes above. We
enumerate linear 3-graphs up to isomorphism with nauty, adding one vertex of degree ≤ 2
per level. This is complete because 3m ≤ 3(n − 2) < 3n. Hereditary filters prune the
search. Each surviving (hypergraph, middle assignment) is tested for realisability by a
rank criterion, and integer sets are scored with the EinsteinArena verifier. Full
details: `results/RESULTS.md`, `docs/method-notes.md`.

## Controls

- The search re-finds the AP hypergraphs of both known 14-point sets with h = 8.
- An independent edge-by-edge generator (`code/brute.cpp`) agrees with the chain on
  four smaller families.
- The rank test reproduces known cases (F7 0/54, Fano 0/24, Pasch 24/30 realisable).
- n = 18..23 use the max-degree chain (`code/maxchain.cpp`, `code/maxresume.cpp` for the
  last n = 23 step); `brute.cpp` contains every max-chain class on six families up to
  (13 points, 11 edges). n = 23 realisability: `code/n23/rt.cpp` (DFS over middle
  assignments, mod-p screening, every prune confirmed exactly), cross-checked against an
  exhaustive enumeration on 200 random classes (`code/n23/realise_fast.py`).

## Reproduce

```bash
brew install nauty
c++ -O3 -mcpu=native -std=c++17 -DWORDSIZE=64 -I/opt/homebrew/include \
    -o chain code/chain.cpp -L/opt/homebrew/lib -lnautyL1
./chain 16 13 14 7 out16 6                       # about 5 min on 6 cores (Apple M5)
python3 code/fetch_verifier.py                   # the platform's public verifier
uv run --with numpy --with sympy python code/realise.py out16/final.txt
```

n = 18..23 (max-degree chain; arguments `N MMIN MMAX TAU DCAP OUTDIR [P]`, see the
header comment of `code/maxchain.cpp` and the logs in `results/n18-23/` for the exact runs):

```bash
c++ -O3 -mcpu=native -std=c++20 -DWORDSIZE=64 -I/opt/homebrew/include \
    -o maxchain code/maxchain.cpp -L/opt/homebrew/lib -lnautyL1
c++ -O3 -std=c++17 -o rt code/n23/rt.cpp
gunzip -k results/n18-23/n23-level23-classes.txt.gz
./rt results/n18-23/n23-level23-classes.txt > rt-n23.txt   # ~5 min on 4 cores
```


## How this was made

The search was designed and run with AI agents (Claude, Anthropic) in an autoresearch
setup, with human direction and review. Every claimed number comes from the code in
this repository and the platform verifier, not from model output.

## Citation

See `CITATION.cff`. Data and results are licensed CC BY 4.0, code MIT.
