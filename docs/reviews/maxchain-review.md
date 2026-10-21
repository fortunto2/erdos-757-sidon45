# Review: max-degree chain (`maxchain.cpp`) completeness, n = 18, 20, 22, 23

Reviewer: scientist (Fable), 2026-10-21. Scope: `maxchain.cpp`, `chain.cpp` (shared
primitives), `crosscheck.cpp`, `brute.cpp`, the five logs, RESULTS.md, JOURNAL.md.
Checks run: see "Measured here" at the end. Nothing was posted.

## Verdict

| n | shape | verdict | reason |
|---|---|---|---|
| 18 | (18,10): m=16, τ≥8, DCAP 4 and HY-free DCAP 9 | **GO** | argument correct; both runs give the same 3 classes (`mx18/final.txt` == `mxfree_18/final.txt` == `n18-final.txt`, diffed); m ≤ 16 never touches the struct bound |
| 20 | (20,11): m=18, τ≥9, DCAP 4; HY-free MMIN 0, DCAP 9 | **GO** | parameters re-derived and correct; m=18 fits `e[18]` exactly; re-run with a 24-slot struct reproduces every level count and 0 classes at k=20 (both variants) |
| 22 | (22,12): m=20, τ≥10, DCAP 4 | **GO** (after today's re-run) | the original binary cannot hold 20 edges (see B1), but the fixed binary reproduces every level count (45/318/250/95/3) and 0 at k=22; the original under ASan also reported no overflow on this run, so no candidate ever reached the write |
| 23 | h=13: m 19..21, τ≥10, DCAP 6 | **NO-GO until one re-run** | final level needs 19–21 edges in a struct with 18 slots (B1). Levels ≤ 22 are unaffected (m ≤ 18). Re-run only the step 22→23 from `out23/level22.bin` with the fixed struct: `review/maxresume.cpp`, ~30 min on 4–6 cores. If it prints 0 classes, n=23 is GO with no further change |

The mathematics (invariant, window DP, forbid mask, τ test, DCAP derivation) is correct.
The one blocking item is an array bound in the code, not in the argument.

## Blocking

**B1. `struct G { uint32_t e[18]; }` (chain.cpp:34) holds at most 18 edges; the final
levels for n = 22 (m = 20) and n = 23 (m = 19..21) write edges 19–21.** In `pick`
(maxchain.cpp:46-49) `G h = g; h.e[h.m++] = ...` writes past the struct on the stack; it is
undefined behaviour, and `canon()` then returns a 76-byte `G` that silently drops the edges
beyond index 17. A 0-class result produced this way is not evidence. Demonstrated with
AddressSanitizer (`review/overflow_demo.cpp`: "stack-buffer-overflow, WRITE of size 4").

What this does and does not touch:
- n = 18 (m ≤ 16) and n = 20 (m = 18): never reached. Unaffected.
- n = 22: only the last step 21→22 could overflow, and only if a candidate passes the τ
  test (the write happens after the `acc` check). Measured: the original binary under ASan
  on the full n=22 run reports nothing, and the fixed binary (`e[24]`) gives identical
  level counts and 0 at k = 22. So n = 22 stands, and should be re-logged from the fixed
  binary.
- n = 23: levels 1..22 have m ≤ 18 (window k=22 is {13..18}), so `out23/level22.bin`
  (4,204,760 classes, 76-byte records) is valid input. Only the step 22→23 must be redone.

Fix: `e[18]` → `e[24]` in chain.cpp and `chosen[8]` → `chosen[16]` in maxchain.cpp (B2).
Note the level-file record size changes from 76 to 100 bytes; `review/maxresume.cpp` reads
the old format and runs exactly one level with the fixed struct. Validated on stored
levels: `out22/level20.bin` → 3 classes m17/τ9 (known answer), `out22/level21.bin` → 0,
`out20/level19.bin` → 0.

Command for n = 23 (not run here: it exceeds the review's 5-minute budget):

```
cd research/chain757/review
c++ -O3 -mcpu=native -std=c++17 -DWORDSIZE=64 -I/opt/homebrew/include -o maxresume maxresume.cpp -L/opt/homebrew/lib -lnautyL1
mkdir -p ../out23fix && ./maxresume 23 19 21 10 6 ../out23fix 22 ../out23/level22.bin 6 | tee ../n23-level23-fixed-log.txt
```

Expected cost: the original step took 1,069 s wall with 6 workers; the cost is front-loaded
in the 17 m=16 and 4,176 m=17 parents (d up to 5), so a prefix sample is not a faster
estimate (a 40,000-record prefix had not finished after 5 min on 4 loaded cores).

## Non-blocking

**B2. `uint32_t chosen[8]` (maxchain.cpp:100) overflows at d = 9.** DCAP = 9 is passed in
the HY-free runs. d = 9 needs 18 parent vertices (k ≥ 19) and a window that admits
m + 9; checked against the logged windows for n = 18 and n = 20: no level satisfies both,
so `pick` was never entered with d = 9 and 9 available disjoint pairs. Latent only. Fix
with B1.

**N1. `n18-maxchain-log.txt` is the HY-free run, not the DCAP-4 run** (windows 0..16 at
k=18, 8.6 s, matching the journal's "HY-free variant 8.6 s"). RESULTS.md cites it as the
DCAP-4 log. Both finals are identical (diffed today), so nothing changes; keep both logs
under distinct names.

**N2. The cross-checks test less than RESULTS.md implies.**
- `brute.cpp` vs maxchain (6 configurations, up to (13,11,τ5), 332,157 targets, missing 0)
  is the real completeness test: it exercises the DP direction, the forbid mask and the
  τ bitset test with d up to 5–6 and no degree cap. Good. It cannot reach m ≥ 19 (B1).
- "3 survivors pushed down into min-chain level 14: IN, IN, IN" is a *soundness* check of
  the min-chain's level 14 (its stored level contains the ancestors it must contain). It
  says nothing about max-chain completeness: a missed survivor is invisible to it.
- n = 16: 7 ⊇ 2 and n = 14: 61 ⊇ 12 are consistency checks. The strict inclusions are
  expected and benign: the two chains enforce `m_k ≤ k − 2` along different deletion
  sequences, so each returns a different superset of the true AP hypergraphs; the extra
  classes have a subset spanning > |S|−2 edges and are unrealisable anyway.
- Decisive control proposed and done today: fixed-struct re-run of n = 20 (both variants)
  and n = 22, level counts identical to the logs. For n = 23 the same, pending (B1).

**N3. HY is applied correctly.** Henning–Yeo, Graphs Combin. 37 (2021) 867–890: a linear
3-uniform H with no subhypergraph isomorphic to AG(2,3) minus two vertices (the paper's F7)
has 17τ ≤ 5n + 3m. `has_f7` tests exactly that pattern as a subhypergraph (centre on 3 edges,
two disjoint edges in the 6 other points; linearity forces one point per spoke). Δ cap:
τ(H−v) ≥ τ−1 and HY on H−v give 3d ≤ s + 12. Re-derived per n (τ_max = ⌊(5n+3(n−2))/17⌋,
m_min from 17τ ≤ 5n+3m, s = 5n+3m−17τ):

| n | h < 4n/7 | τ | m range | s | Δ ≤ (s+12)/3 | DCAP used |
|---|---|---|---|---|---|---|
| 18 | ≤ 10 | 8 | 16 | 2 | 4 | 4 (and 9 = HY-free, valid: Δ ≤ ⌊17/2⌋) |
| 20 | ≤ 11 | 9 | 18 | 1 | 4 | 4 (and 9 HY-free, Δ ≤ 9 on 20 points) |
| 22 | ≤ 12 | 10 | 20 | 0 | 4 | 4 |
| 23 | ≤ 13 | 10 | 19, 20, 21 | 2, 5, 8 | 4, 5, 6 | 6 (max over m; superset, safe) |

n = 24: τ ≤ 10, h ≥ 14 > 4/7, no shape; n = 25: τ ≤ 11, h ≥ 14 < 4/7 (next target).
All run parameters match this table (checked against the window lines of each log).

**N4. HY-free claims.** At n = 18 and 20 the DCAP-9, MMIN-0 runs give the same final
level as the HY runs (3 and 0), so (18,10) and (20,11) rest only on Ma–Tang's reduction
and the rank test. n = 22 and 23 use HY twice (m range and DCAP); say so in the write-up.

## Why the argument is right (checked line by line)

1. Hereditary: H_k = H_N − (deleted vertices) is the induced subhypergraph, i.e. the AP
   hypergraph of the k-point subset, hence linear, F7-free, SDR, m_k ≤ k−2, Δ ≤ Δ(H_N) ≤
   DCAP. `del`/the chain remove every edge through v, which is induced deletion.
2. τ(H − v) ≥ τ(H) − 1, so τ_k ≥ TAU − (N−k) = `lo_tau(k)`.
3. Window DP (`build_windows`): feas[k][m] ⇔ ∃ d ≤ DCAP: feas[k+1][m+d] and
   3(m+d) ≤ d(k+1), plus m ≤ k−2. The inequality is `3*m2 > d*(k+1) → skip`, i.e. d ≥
   average degree of H_{k+1}, the right direction. Hand-checked: n=18/DCAP 4 gives
   {12,13}, {8,9,10} at k = 17, 16 as RESULTS.md states; n=20 gives {14,15}, {10,11,12};
   n=23 gives {13..18} at k=22. All match the logs.
4. `extend_max`: d runs from Δ(parent) to DCAP (degrees only drop, so d_k ≥ Δ(H_{k−1}));
   `forbid` excludes vertices with deg+1 > d, so every touched vertex ends ≤ d and the
   new vertex is a maximum-degree vertex of the child; pairs exclude already-covered
   pairs (linearity) and are pairwise disjoint (`used`).
5. τ test: if τ(parent) ≥ tl nothing to test (adding a vertex cannot lower τ). If
   τ(parent) = tl−1, a (tl−1)-transversal of the child either contains the new vertex
   (then tl−2 vertices cover the parent, impossible) or is a minimum transversal of the
   parent hitting all d pairs. `mt_rec` lists every minimum transversal (branch on the
   first unhit edge; a strict subset of T that is a transversal would be smaller than τ).
   `acc` = ∩ S_pair must be empty: correct, including the edge case T = {∅} at tl = 1.
6. Isomorph rejection: every filter is invariant under relabelling of the parent, so the
   stored canonical representative admits the same extension. Induction from H_0 = ∅.

## Measured here

- `mx18/final.txt`, `mxfree_18/final.txt`, `n18-final.txt`: byte-identical (diff).
- Original `maxchain` built with `-fsanitize=address`, run `22 20 20 10 4`: no report,
  level counts as logged, 0 at k=22.
- Fixed binary (`e[24]`, `chosen[16]`): `20 18 18 9 4` → 528/2712/282/17/**0**;
  `20 0 18 9 9` → 528/2731/282/17/**0**; `22 20 20 10 4` → 45/318/250/95/3/**0**.
  All counts equal the logs (raw counts differ only through the worker count).
- `maxresume` on `out22/level20.bin` → 3 classes m17/τ9; on `out22/level21.bin` → 0;
  on `out20/level19.bin` → 0.
- `overflow_demo.cpp` under ASan: stack-buffer-overflow at the 19th edge write.
- HY statement verified against the 2021 paper's abstract (web).

Files added: `research/chain757/review/{chain_fix.cpp, maxchain_fix_lib.cpp, maxresume.cpp, overflow_demo.cpp}`.
`maxchain.cpp` and `chain.cpp` themselves are untouched; apply the two-line fix there and
re-run n = 22 and the n = 23 last step before publishing either.
