# Erdős Problem #757: no 16-point (4,5)-set with Sidon number 9

Computer-assisted results on Erdős Problem #757, the constant C_5b in Tao's repository
of optimisation constants. The constant is the largest c such that every (4,5)-set of
n reals contains a Sidon subset of size c·n. A (4,5)-set is a set in which every 4
points determine at least 5 distinct differences. The known bounds are
9/17 ≤ c ≤ 4/7 (Ma–Tang, arXiv 2602.23282, 2026).

## Results

1. **No (4,5)-set with (n, h) = (16, 9) exists.** Here h is the size of the largest
   Sidon subset. Within n ≤ 18 the only shapes that could beat 4/7 are (16, 9) and
   (18, 10), so the first is now ruled out.
2. **Classification at n = 14.** Every 14-point (4,5)-set with h = 8 has exactly 12
   three-term progressions. Its progression hypergraph is one of 9 isomorphism types,
   and each type is realised by an integer set (listed in `results/RESULTS.md`).
3. A 14-point set with h = 8 that differs from Ma–Tang's set:
   `0 323 416 468 509 517 537 557 566 575 613 646 717 1018`.

(18, 10) is open here. A follow-up search settles it and is under review.

**Context (added 2026-10-20).** The general upper bound has since dropped to **5/9**. On
2026-10-17 a reader of erdosproblemaday.com submitted a self-similar family of (4,5)-sets
with 11 + 9K points and Sidon number 7 + 5K, which the site checked by machine. Its first
finite set below 4/7 has 65 points (K = 6). Our results address the small end. Since
(16, 9) is impossible, any (4,5)-set with h/n < 4/7 has at least 17 points, and an
(18, 10) set is the only candidate below 19.

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

## Reproduce

```bash
brew install nauty
c++ -O3 -mcpu=native -std=c++17 -DWORDSIZE=64 -I/opt/homebrew/include \
    -o chain code/chain.cpp -L/opt/homebrew/lib -lnautyL1
./chain 16 13 14 7 out16 6                       # about 5 min on 6 cores (Apple M5)
python3 code/fetch_verifier.py                   # the platform's public verifier
uv run --with numpy --with sympy python code/realise.py out16/final.txt
```

## How this was made

The search was designed and run with AI agents (Claude, Anthropic) in an autoresearch
setup, with human direction and review. Every claimed number comes from the code in
this repository and the platform verifier, not from model output.

## Citation

See `CITATION.cff`. Data and results are licensed CC BY 4.0, code MIT.
