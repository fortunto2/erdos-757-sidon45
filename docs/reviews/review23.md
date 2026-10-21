# Adversarial review: "no 23-point (4,5)-set has h = 13" (Erdős #757)

Referee: Fable (second, adversarial pass), 2026-10-21. Scope: the claim as a chain of
`maxchain.cpp` / `review/maxresume.cpp` (level 23 of the (23, τ≥10, m 19..21, DCAP 6) run,
`n23-level23-fixed-log.txt`, `out23fix/final.txt`) followed by `realise23/rt.cpp`
(`realise23/out/rt-n23.txt`). I tried to refute it; I did not modify any existing file.
Everything below is labelled *measured here* (I ran it), *re-derived* (pen and paper on
the code) or *reported* (taken from a log or a paper).

## Verdict: GO

No blocking issue. The claim is correct **modulo Henning–Yeo** (17τ ≤ 5n+3m for linear
3-uniform F7-free hypergraphs, F7 = {012,034,056,135,246}), which is used twice (m ≥ 19 and
Δ ≤ 6) and is the same published theorem Ma–Tang use for their 9/17 bound. Everything that
can be checked without HY was checked and holds.

## 1. Shape (re-derived, then measured)

- h < 4·23/7 = 13.14 ⇒ h ≤ 13 ⇒ τ = 23 − h ≥ 10.
- m ≤ n − 2 = 21 (min and max of A are never middles; Ma–Tang Lemma 2.4). With HY:
  17τ ≤ 115 + 63 = 178 ⇒ τ ≤ 10. So τ = 10 exactly and 170 ≤ 115 + 3m ⇒ m ≥ 19.
  τ = 11 would need m ≥ 24 > 21. No m or τ is excluded wrongly; the run's τ ≥ 10 is a superset.
- DCAP: delete v of degree d; H − v is linear, F7-free (hereditary), has n−1 points, m−d
  edges and τ ≥ τ−1; HY on it gives 3d ≤ s + 12 with s = 5n + 3m − 17τ ∈ {2, 5, 8} for
  m ∈ {19, 20, 21}, so Δ ≤ 4, 5, 6. DCAP 6 is the max over m: a superset, safe. It is a
  consequence of HY, not an extra assumption.
- Measured here on all 86,094 survivors: Δ = 3 (8,378), 4 (72,405), 5 (5,311), none with 6.
- HY statement and F7 verified from the Ma–Tang arXiv HTML (alttext of the math):
  `E(F_7) = {{0,1,2},{0,3,4},{0,5,6},{1,3,5},{2,4,6}}`, Theorem 5.3 = `17τ(H) ≤ 5n_H + 3m_H`,
  citing Henning–Yeo, Graphs Combin. 37(3) (2021) 867–890, Theorem 7. Consistency check:
  F7 itself has τ = 3 and 17·3 = 51 > 50 = 5·7 + 3·5, so the excluded configuration cannot be
  the Fano plane (τ = 3, 51 ≤ 56); `has_f7` tests exactly this 5-edge pattern as a
  (non-induced) subhypergraph, and on 7 points m ≤ 5 makes induced and non-induced the same.
- Erdős #757 definition confirmed (erdosproblems.com): every 4-subset has ≥ 5 distinct
  absolute differences. (4,5) ⇔ weak Sidon + linear AP hypergraph in both directions
  (a difference coincidence is either a 4-distinct additive quadruple or a 3-AP; two APs in a
  4-set share two points). Only the forward direction is needed for the kill.

## 2. Max-degree chain completeness (re-derived line by line)

Let A be a (4,5)-set, |A| = 23, h = 13, H = H(A), v_k a max-degree vertex of H_k,
H_{k−1} = H_k − v_k (induced = AP hypergraph of a 22-, 21-, … point subset). Every filter in
`extend_max`/`pick` is a property H_k has, so by induction canon(H_k) is in level k:

| filter | why H_k satisfies it | code |
|---|---|---|
| linear, F7-free, SDR, m_k ≤ k−2 | hereditary (induced subhypergraph of an AP hypergraph) | `has_sdr`, `has_f7`, DP `m ≤ k−2` |
| τ_k ≥ 10 − (23−k) | τ(H − v) ≥ τ(H) − 1 | `lo_tau`, `tau_ge`, bitset test |
| Δ(H_k) ≤ 6 | degrees only drop | `d ≤ DCAP`, forbid mask |
| d_k ≥ Δ(H_{k−1}) and every touched vertex ends with degree ≤ d_k | v_k is a max-degree vertex of H_k | `d = max(dmax,…)`, `forbid = {deg+1 > d}` |
| the d_k pairs are pairwise disjoint and uncovered in H_{k−1} | linearity | `used`, `covered` |
| (k, m_k) ∈ feas[k] | DP of necessary conditions only: d ≤ 6, 3m_{k+1} ≤ d(k+1) (max ≥ average), m ≤ k−2, m_23 ∈ [19,21] | `build_windows` |
| τ bitset test: child has τ ≥ tl iff no minimum transversal of the parent hits all d pairs | a tl−1 transversal of the child either contains the new vertex (then tl−2 cover the parent: impossible) or is a minimum transversal of the parent hitting every pair | `mt_rec` lists all minimum transversals (branch on first unhit edge), `acc = ∩ S_p` |

Isomorph rejection is safe: every rule is invariant under relabelling the parent.
Windows re-derived independently (`dp.py`, 20 lines): identical to the 24 window lines
in the log at every level, in particular k=22 → {13..18}, k=23 → {19,20,21}.
The DP inequality direction (`3*m2 > d*(k+1) → skip`) is the right one.

`maxresume`: 319,561,760 B / 76 B = 4,204,760 records, matches "loaded 4204760". The
level-22 window is m ≤ 18, so the original `e[18]` struct never overflowed before the
22→23 step; the resumed step uses the 100-byte struct. The absence of m = 19 at level 23
is not suspicious: an m = 19 child needs a parent with m = 16 and d = 3 (average degree
57/23 > 2), and level 22 has only 17 such parents.

One thing the first review did not say: `extend_max` never checks 3m' ≤ d·k for the
child itself. It does not need to: a child whose new vertex is a maximum-degree vertex
satisfies it automatically, and the DP uses it only as a relaxation.

## 3. The survivors (measured here, independent code `check23.cpp`)

All 86,094 lines of `out23fix/final.txt` parse, have k = 23, 3-uniform edges on vertex
ids < 23, are linear, have an SDR, contain no F7, have Δ ≤ 5, and have τ = 10 exactly
(no 9-transversal; a 10-transversal exists), recomputed with my own branch-and-bound.
m = 20: 152, m = 21: 85,942. 0 bad lines; 53 s.

`realise23/out/rt-n23.txt`: 86,094 lines, idx 0..86,093 contiguous, no duplicates, all
verdict U, all `leaves_surviving` = 0, max nodes 38,993; 7 Fano-containing, 0 HY-F7,
699 without a dense subset. Every class of `final.txt` was processed.

## 4. `rt.cpp` soundness (re-derived)

The kill needs only one direction: if A realises (H, μ_A) then x = A is a point of V
avoiding every bad form, so no bad form vanishes identically on V, nor on any V_k ⊇ V
of a prefix of μ_A. `rt` prunes a subtree only after `vanishesExact` confirms, over Q,
that a bad form lies in the row space of the k assigned equations. Checked:

- Bad forms: x_i − x_j; x_a + x_c − 2x_b for {a,b,c} **not an edge** (`third[a][c] == b`
  skips every edge triple whatever its eventual middle — right, since a wrong middle on an
  edge collapses to x_a = x_b and is caught by distinctness); x_i + x_j − x_k − x_l on 4
  distinct indices. Pairs sharing an index reduce to distinctness. Complete for "A is a
  (4,5)-set with H(A) = H exactly", which is what the chain enumerates (H(A) itself, not a
  supergraph), so "exactly H" is the right requirement.
- DFS: for each edge in `order_`, all 3 vertices not already a middle. Distinct middles
  is forced for every (4,5)-set (Ma–Tang Lemma 2.4), so the DFS covers μ_A. Edge order
  only changes the node count.
- mod p = 2^61−1 on two random points of V_p: a false alarm costs an exact check (0 in
  15.6M, reported); a p-dependent *drop* of rank can only enlarge V_p and lose prunes,
  never add one. A surviving leaf would be printed as R; none was.
- Bareiss: fraction-free with column skipping; the Sylvester identity still makes every
  division exact (a skipped column is zero in all later pivot rows, so its stale zeros equal
  the formula's value). Entries are in {0, ±1, ±2}; rows ≤ 22 (21 equations + f), so every
  intermediate value is a minor of size ≤ 22 with Hadamard bound 6^11 ≈ 3.6e8 and every
  product ≤ 1.3e17 < 2^63. `__int128` has 1e21 of headroom.
- `mid[]`/`order_[]` are consistent at every depth (set before the recursive call, read only
  for indices < k).

## 5. Independent re-check of realisability (measured here)

`indep_realise.py` (scratchpad): my own DFS over middles with a different edge order,
nullspace over Q by `fractions.Fraction`, coincidences spotted at 3 random integer points,
every prune confirmed on the exact integer basis. Shares no code with `rt.cpp`,
`realise.py` or `realise_fast.py`.

- Known answers: Fano U, F7 U, Fano−line U, Pasch R; the 12 n=14 classes of
  `out14m11/final.txt` give U R R U R R R R R R R U, exactly `n14-realise.json`
  (realisable_mu 0, 282, 446, 0, 430, …, 0).
- n = 23: 67 classes (random seed 2/3, plus 9 of the 699 "no dense subset" classes and the
  Fano-containing idx 20474 and 35445; indices chosen by me, not by the authors): **all U**,
  261,296 DFS nodes, 56,102 exact prunes, 0 false alarms, 1,186 CPU-seconds. My greedy edge
  order is much worse than `rt`'s dense-first order (max 87,394 nodes against `rt`'s 1,535
  on the same classes), so the sample is small; it was stopped at the 30-minute budget.
  Together with the 200-class FLINT cross-check (`out/sample200.jsonl`, 0 disagree) that is
  267 classes under two independent exact implementations.

## 6. Non-blocking remarks

- N1. HY enters twice (m ≥ 19, Δ ≤ 6). An HY-free run (MMIN 0, DCAP 11) is out of reach
  at n = 23; the write-up must state the dependency, as the first review already said.
- N2. The 22→23 step is the one step of the whole pipeline that no independent generator
  has reproduced (`brute.cpp` cannot reach m ≥ 19). It runs the same `extend_max` as the
  brute-checked levels, and the fix is a 3-line diff (`e[24]`, `inside[]`, `chosen[16]`),
  which I re-read. That is where the residual risk sits, and it is small.
- N3. `has_f7`'s `through[18]` is safe only because Δ ≤ 6; a future HY-free run with DCAP
  ≥ 19 would overflow it (not reachable here).
- N4. `realise23/RESULTS.md` says "all minors ≤ 6^11": correct, and the margin is far larger
  than stated (int64 would already do).
- N5. Since no survivor has Δ = 6, a DCAP-5 run would have given the same final level; the
  DCAP-6 run is simply a superset.

## Measured here (commands)

```
c++ -O2 -std=c++17 -o check23 check23.cpp && ./check23 out23fix/final.txt 6 10   # 86094 lines, 0 bad
python3 -I dp.py                                                                 # windows == log
python3 -I indep_realise.py realise23/controls-small.txt 0 1 2 3                 # U U U R
python3 -I indep_realise.py out14m11/final.txt 0..11                             # matches n14-realise.json
python3 -I indep_realise.py out23fix/final.txt <idx...>                          # sample, all U
awk on realise23/out/rt-n23.txt                                                  # 86094 idx, all U, leaves 0
curl arxiv.org/html/2602.23282v2 | grep alttext                                  # F7 edges, 17τ ≤ 5n+3m
```

Scratch code (not committed): `check23.cpp`, `dp.py`, `indep_realise.py` in the session
scratchpad; copies can be dropped into `research/chain757/review/` on request.
