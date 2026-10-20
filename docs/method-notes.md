# Scientist note: Erdős #757 / C_5b, record shapes (16,9) and (18,10)

Date 2026-10-20. Sources checked: Ma–Tang arXiv 2602.23282 v2 (PDF, §1, §2, §5),
Henning–Yeo, Graphs Combin. 37 (2021) 867–890, doi 10.1007/s00373-021-02285-x (abstract
only, full text paywalled), own rank computations (scratch script, F7/Fano/Pasch).

## Verdict

The reduction to (n,h) ∈ {(16,9), (18,10)} is correct. One label is wrong: **F7 is not the
Fano plane**. In Ma–Tang (and in Henning–Yeo's hypothesis) F7 is AG(2,3) with two points
deleted: vertices 0..6, edges {012, 034, 056, 135, 246}, one vertex of degree 3 and two
disjoint edges. The Fano plane is also unrealisable (checked below), so nothing you proved
is false, but any "Fano-free" clause is not the HY hypothesis. No argument I know kills
(16,9) or (18,10) on paper; both are *within 3 of HY equality*, which pins the structure
hard (below). Settle them by a vertex-deletion chain enumeration of abstract hypergraphs,
with realisability as a pure rank test, not by SAT with 8008 covering clauses.

## Why the reduction holds (re-derived)

- (4,5) ⇔ weak Sidon + H linear: a 4-set carries ≥ 2 difference coincidences iff it is an
  additive quadruple a+d=b+c (gives b−a=d−c and c−a=d−b) or contains two 3-APs. Agreed.
- Distinct middles: two APs with middle y give x+z = x'+z', four distinct points. Agreed.
  Consequence nobody has used yet: **all m APs have pairwise distinct common differences**
  (two APs with the same d and disjoint legs form an additive quadruple; sharing a leg makes
  a 4-AP, which is non-linear), and the doubled difference 2d of an AP is the difference of
  no other pair. So |A−A| ∩ ℕ = C(n,2) − m exactly, with m values repeated exactly twice.
  Ma–Tang's set: 12 APs, differences 136,200,298,528,460,43,49,3,26,77,37,128, all distinct.
- AG(2,3) itself (12 lines, 9 points) is excluded by distinct middles alone; F7 (5 lines,
  7 points) needs Gyárfás–Lehel Prop 2.6, i.e. realisability.
- HY: linear 3-uniform, no F7 subhypergraph ⇒ 17τ ≤ 5n+3m. With m ≤ n−2 and h = n−τ:
  n=14: τ≤6, h≥8 (=4/7, optimal). n=15: τ≤6, h≥9 (9/15>4/7). n=16: τ≤7, h≥9 (9/16<4/7),
  τ=7 needs m≥13. n=17: τ≤7, h≥10 (10/17>4/7). n=18: τ≤8, h≥10, τ=8 needs m≥16=n−2.
  The weaker Tuza/Chvátal–McDiarmid 4τ ≤ n+m gives the same for 14,16,18 but not 15,17.

## What near-equality forces (new, cheap, use it)

Let s(H) = 5n+3m−17τ ≥ 0. Candidates: (16,13,τ7) s=0 (exact HY equality), (16,14,7) s=3,
(18,16,8) s=2. Deleting a vertex v of degree d gives s(H−v) ≤ s+12−3d, and if v lies in
no minimum transversal then s(H−v) = s−5−3d < 0. Hence in every candidate:

1. every vertex is τ-critical (lies in a minimum transversal), Δ ≤ 4 (Δ ≤ 5 for (16,14));
2. for s<3 every edge is τ-critical: deleting any edge drops τ, i.e. **every edge is the
   only edge inside some 10-set (n=16) / 11-set (n=18)**; a (16,14) solution with a
   non-critical edge contains a (16,13) one;
3. δ ≤ 2 since 3m < 3n, and δ ≥ 1 by criticality; d(u)+d(v) ≤ 8+[u,v share an edge];
4. H−v is again HY-near-extremal: (15, 13−d, τ≥6), etc. — this is the chain below.

Ma–Tang's hypergraph (14,12,6) has s=4 and is already fully vertex- and edge-critical,
degrees 2^7 3^6 4^1, one component, no Pasch. Same look as what we need.

Also 4τ ≤ n+m is within 1 for (16,13,7); Henning–Yeo (JGT 59, 2008) characterised the
*connected* equality cases of that bound. Worth checking that characterisation for a
"slack 1" statement in the linear case; I could not access either paper's body.

## Realisability is linear algebra, not LP

Given H with a middle assignment μ, the AP equations x_a + x_c = 2x_b have row space W.
The configuration is realisable as a (4,5)-set with H(A) = H **iff none of these forms
lies in W**: x_u − x_v (distinct points), x_a + x_c − 2x_b for every non-edge triple and
every choice of b (no extra AP), x_a + x_d − x_b − x_c (weak Sidon). Middles are then
automatically between their endpoints, and generic points of the solution space give
integer sets after scaling. Known-answer check: F7 fails for all 54 SDR assignments,
Fano fails for all 24, Pasch is realisable for 24 of 30. So: search *unordered* (H, μ),
test by rank, read the order off the solution. This removes the n!/2 ordering freedom, the
order cuts, the LP, and every diameter cap (CP-SAT over [0,N] can miss the solution if N
is too small; the rank test cannot). Dimension of the solution space is ≥ 2 (affine); for
(18,16) it is typically exactly 2, so the hypergraph determines the set.

## Plan to settle n=16 (then 18)

Stage A, abstract: does a linear 3-graph on 16 vertices with 13 ≤ m ≤ 14, τ ≥ 7, Δ ≤ 5,
an SDR (every k edges span ≥ k vertices) and no F7 exist? Vertex-deletion chain, every
level a "min-degree vertex removed":

```
F(12, 5..7, τ≥3) → F(13, 7..9, τ≥4) → F(14, 9..11, τ≥5) → F(15, 11..12, τ≥6) → F(16, 13..14, 7)
```

(m ranges for the m=13 target; widen by 1 for m=14.) Extend each member by one new vertex
of degree 0, 1 or 2 (two disjoint unused pairs), keep iff τ ≥ level target (bitmask α, µs),
reject isomorphs by canonical form (nauty via pynauty, or Rust + a certificate). Complete
because Σd = 3m < 3n at every level, so a degree-≤2 vertex always exists and τ drops by
at most 1 per deletion. Known-answer control: the same chain with targets shifted by one
must re-find Ma–Tang's hypergraph in F(14, 12, 6). Sizes are my guesses, not measured:
F(12,·,≥3) ~10^4–10^5, the bulge at F(13)/F(14) ~10^6–10^8, F(15, 11..12, ≥6) small
(s ≤ 9), F(16,13,7) tiny or empty. Rust hours, Python days. If F(16,13..14,7) is empty,
(16,9) is dead by pure hypergraph theory and that is a lemma worth writing up.

Stage B: for each survivor, all SDR middle assignments (≤ 3^m but usually few hundred),
rank test as above, then integer scaling and the arena verifier. Stage C: n=18 with
F(13, 6..8, ≥3) → … → F(17, 14..15, ≥7) → F(18, 16, 8); the m=n−2 condition means the
two non-middles are the extreme points, and dimension 2 means one candidate set per (H,μ).

Quick wins before any of that: add to CP-SAT an AllDifferent over the m AP differences
(strongest cheap propagator you are missing), fix x_1 = 0 and break reversal with
x_2 − x_1 < x_n − x_{n−1}; add to the hypergraph SAT the criticality witnesses
(for each vertex v a 7-set containing v that is a transversal) only if propagation, not
clause count, is the bottleneck.

## Risks

- HY equality characterisation may already exist in the 2021 paper and settle (16,13)
  instantly either way; get the PDF (UJ repository or the authors) before coding.
- The chain's middle levels can blow past 10^8; if so, switch to generating F(15,·,≥6)
  directly from F(14,·,≥5) only with the F7-free and Δ≤4 filters pushed down, and add
  the SDR filter early (Hall on edge subsets, or carry μ in the state).
- τ-criticality holds for the final object, not for intermediate levels; do not prune
  intermediate levels with it.
- The rank test assumes generic points avoid every non-forced coincidence; it does, but
  the integer set you print must still go through the arena verifier (it has, for n=14).
