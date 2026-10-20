# Erdős #757 chain enumeration: results (2026-10-20)

Spec: `docs/method-notes.md`. Code: `chain.cpp` (BFS chain, nauty canon), `dfs.cpp`
(depth-first tail, no dedup), `brute.cpp` (independent check), `realise.py` (stage B rank
test + integer sets + platform verifier), `controls.py`.

## Conclusion

**No (4,5)-set with (n, h) = (16, 9) exists.** Every linear, F7-free 3-graph on 16 vertices
with 13 ≤ m ≤ 14, τ ≥ 7 and a distinct-middle SDR, subject to the hereditary bounds below,
is one of **2** isomorphism classes, both m = 14, τ = 7, none with m = 13. Neither is
realisable: 0 of 5184 and 0 of 5280 middle assignments pass the rank test. Both contain a
7-point, 6-edge piece (Fano minus a line), which already breaks m ≤ |S| − 2 for that subset.
Since HY forces m ≥ 13 for τ = 7 at n = 16, this kills the (16,9) shape. It is a
computer-assisted lemma, and it relies on Henning–Yeo only for the m ≥ 13 range.

**n = 18 is not settled.** At this run's speed it would take about 300 core-hours (estimate below).

By-product: **every 14-point (4,5)-set with h = 8 has exactly 12 APs, and its AP hypergraph
is one of 9 isomorphism types.** The chain with m ∈ [11,12] finds 12 abstract classes, all
with m = 12, and 9 of them are realisable. Each of the 9 gives a set with verifier score
4/7 (`n14-realise.json`; the sets are listed below). Survivors at n=16: `n16-final.txt`, `n16-realise.json`; logs `n16-log.txt`, `n18-log-partial.txt`.

## Method

Level k is the set of linear 3-graphs on k vertices (up to isomorphism) with
- m ≤ min(k − 2, MMAX) (holds for any real set and its subsets),
- m ≥ MMIN − 2(N − k), τ ≥ TAU − (N − k),
- F7-free, distinct-middle SDR exists.

Level k is built from level k−1 by adding a vertex of degree 0, 1 or 2 that is a minimum-degree
vertex of the result. This is complete for every target. Σd = 3m ≤ 3(k−2) < 3k, so a vertex of
degree ≤ 2 always exists. Deleting it keeps every condition above and drops τ by at most 1.
τ is filtered exactly: if τ(parent) = target − 1, the new pairs must break every minimum
transversal of the parent, which is a bitset test.

## Controls (all passed)

| control | result |
|---|---|
| Ma–Tang set and our SAT 14-point set: AP hypergraphs (both m=12, τ=6) | both FOUND in F(14,12,6), which has 12 classes |
| Independent generator (`brute.cpp`: all linear 3-graphs edge by edge, + subset density) vs chain | (10,8,τ4) 73=73; (11,9,τ4) 3250=3250; (12,10,τ5) 221=221; (12,10,τ4) 83549 ⊆ 83554, missing 0 |
| Stage B rank test, scientist's known answers | F7 0/54, Fano 0/24, Pasch 24/30 realisable |
| Two code versions (with and without the bitset τ filter) | level 15 of n=16 is 6,530,370 in both |

## Level sizes

n = 16, MMIN 13, MMAX 14, TAU 7 (6 workers):

| k | classes | m range | τ ≥ | wall |
|---|---|---|---|---|
| 12 | 121,143 | 5–10 | 3 | 0.4 s |
| 13 | 2,532,413 | 7–11 | 4 | 7 s |
| 14 | 33,240,040 | 9–12 | 5 | 151 s |
| 15 | 6,530,370 (m11:251, m12:28,918, m13:6,501,201) | 11–13 | 6 | +105 s |
| 16 | **2** (both m14/τ7) | 13–14 | 7 | +37 s |

Total about 5 min wall time. Stage B on the 2 survivors took 8 s.

n = 14 control (MMIN 12): k=12: 105,314, k=13: 343,502, k=14: 12. With MMIN 11 the result is still 12, all with m=12.

n = 18, MMIN = MMAX = 16, TAU 8: k=12: 121,274; k=13: 2,602,962; k=14: **65,343,355**
(260 s, 5 GB). Depth-first sample of 1001 level-14 roots: 54,901 nodes at k=15, 649,781 at k=16,
1,134 at k=17, 0 at k=18, in 17 s single-core. Extrapolated to all 65M roots that is ≈ 3e5
core-seconds (~300 core-hours). Adding the hereditary filters Δ ≤ 4 (HY, s=2) and
"every S spans ≤ |S|−2 edges" cuts k=16 nodes by only 14% and doubles the time per node, so
neither filter helps.

## Realisable 14-point types (h = 8, score 4/7)

```
[0,106,195,212,229,290,390,408,454,458,474,490,500,526]
[0,146,156,262,277,278,292,306,312,335,378,478,556,956]
[0,92,181,184,187,350,362,374,396,526,656,700,744,962]
[0,542,634,774,859,997,1006,1015,1084,1171,1268,1336,1452,1898]
[0,148,152,256,268,269,282,296,304,323,364,460,536,920]
[0,88,175,176,177,198,219,346,350,354,473,510,547,748]
[0,1004,1112,1414,1527,1614,1640,1666,1716,1792,1805,1970,2008,2224]
[0,192,334,384,432,434,468,499,504,509,530,550,668,868]
[0,164,169,256,292,328,338,487,512,537,546,555,682,818]
```

## Next for n = 18

The tail of the chain is the bottleneck: the k=16 → 17 step kills 99.8% of its inputs,
but generating about 4e10 k=16 nodes is what costs the time. Options:
1. A structural lemma from HY at s = 2. For u, v not in a common minimum transversal,
   s(H−u−v) = 9 − 3·e({u,v}) ≥ 0, so any two vertices touching ≥ 4 edges lie in a common
   8-transversal. Push this down as a constraint on which deletion sequences are possible.
2. Generate F(17, 14..15, τ ≥ 7) directly, e.g. by edge augmentation inside a fixed
   10-point independent set plus an 8-point transversal, instead of via k = 15 and 16.
3. Brute force on a cluster: dfs.cpp shards cleanly (`dfs ... SHARD NSHARDS 1 OUT`), about 300 core-hours.

Reproduce: `c++ -O3 -mcpu=native -std=c++17 -DWORDSIZE=64 -I/opt/homebrew/include -o chain chain.cpp -L/opt/homebrew/lib -lnautyL1`
(`brew install nauty`), then `./chain 16 13 14 7 out16 6` and `uv run python realise.py out16/final.txt`.
