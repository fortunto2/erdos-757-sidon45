# Referee report: `main.tex` (Erdős #757, small (4,5)-sets), 2026-10-21

Hostile pass over `docs/paper-757/main.tex` / `main.pdf` (both 2026-10-21 11:31, PDF text
matches the source) against `research/chain757/{RESULTS.md, maxchain-review.md, review23.md,
realise23/RESULTS.md, *-log.txt, *.json}`, `tasks/sidon-45-set/JOURNAL.md`, the public repo
`~/projects/erdos-757-sidon45` (HEAD 5d2ff35 = origin/main), Zenodo 23494596, arXiv/OpenAlex,
erdosproblems.com/757, erdosproblemaday.com. Labels: *measured here* (I ran it), *re-derived*
(pen and paper), *reported* (from a log or a paper).

## Verdict

The mathematics holds and every number in the PDF that has an artifact behind it matches the
artifact. The paper is not submittable as is: two sentences about Henning–Yeo dependence are
false (one in the theorem statement), the algorithm that carries four of the five cases is not
described, and the "released code" it points to does not contain the n = 18..23 code or data.
All four are fixable in a day without new computation.

## Blocking

### B1. The HY-free claim for n = 16 is misreported (Section 4, para 3)

Text: *"A run that never invokes Henning–Yeo gives the same classes for n=16 and n=18"*.

For n = 18 that is true (`mxfree_18/final.txt` == `n18-final.txt`, diffed in maxchain-review).
For n = 16 it is false: the HY-free run (`n16-maxchain-log.txt`, windows m ∈ {0..14} at k=16,
DCAP 9, MMIN 0) returns **7** classes, not the 2 of Table 1 (`n16-maxchain-realise.json`: 7
entries, all m=14, τ=7, realisable_mu 0 with 555, 495, 477, 504, 513, 5184, 5280 middle
assignments). The 2 classes of Table 1 come from the min-degree chain with m ≥ 13, which is an
HY input. So as written, the paper's only evidence for "the first two cases are independent
of HY" (Theorem 1) is a sentence that does not match the artifact. The 7 ⊇ 2 inclusion is
benign (the two chains propagate m ≤ k−2 along different deletion orders; the 5 extra classes
have a subset spanning > |S|−2 edges), but the reader must be told.

Replacement:

> The same maximum-degree chain run with no Henning–Yeo input at all (every m ≤ 14, and the
> trivial degree cap Δ ≤ ⌊(n−1)/2⌋) returns 7 classes at n = 16, all with m = 14 and τ = 7,
> containing the two of Table 1; none is realisable (0 of 555, 495, 477, 504, 513, 5184 and
> 5280 middle assignments). The five extra classes contain a subset spanning more than |S|−2
> progressions and are excluded by (i) alone. At n = 18 the HY-free run returns the same three
> classes as the capped run.

### B2. Theorem 1 and the abstract state the HY dependence wrongly

(a) *"n = 20, 22, 23 do [use HY]"* / *"the other three use it"*: n = 20 does not. The HY-free
run `n20-hyfree-log.txt` (MMIN 0, DCAP 9; Δ ≤ 9 is the trivial cap on 20 points) gives
528/2731/282/17/**0** classes at k = 16..20, and maxchain-review reproduced it with the
fixed binary (its N4 says the same). Only n = 22 and n = 23 use HY, and each uses it twice
(the m range and the degree cap).

(b) Theorem 1 says "(n,h) equal to (16,9), …". What the runs prove is τ ≥ 7, 8, 9, 10, 10,
i.e. **h ≤ 9, 10, 11, 12, 13**. The "≤" form is what the HY-free statements need (without HY,
h ≤ 8 at n = 16 is not excluded by anything else), and it is what was computed.

(c) *"Consequently f(n)/n ≥ 4/7 for all n ≤ 24"* and Corollary 2 use HY for every n not in the
list (n = 13 is the first: h = 7 is not excluded by Erdős–Sós c ≥ 1/2, only by HY), and the
abstract's "the only shapes that could go below 4/7" is an HY statement. Say so once.

Replacement for Theorem 1:

> **Theorem 1.** No (4,5)-set has (|A|, h(A)) with |A| = 16 and h ≤ 9, |A| = 18 and h ≤ 10,
> |A| = 20 and h ≤ 11, |A| = 22 and h ≤ 12, or |A| = 23 and h ≤ 13. The cases n = 16, 18, 20
> use only the Ma–Tang reduction and the rank test; the cases n = 22 and n = 23 also use the
> Henning–Yeo bound, for the range of m and for the degree cap. Combined with Henning–Yeo for
> the remaining n, f(n)/n ≥ 4/7 for all n ≤ 24.

Abstract, last sentence of the HY clause: *"The cases n = 16, 18 and 20 do not use the
Henning–Yeo transversal bound; n = 22 and 23 do, and so does the list of shapes itself."*

### B3. The maximum-degree chain is not described, and it carries four of the five cases

Section 3.1 proves completeness for the minimum-degree chain (used for n = 16 only). For
n = 18, 20, 22, 23 the paper says one sentence: *"enumerating by deleting a vertex of maximum
degree keeps the levels small"*. Nothing tells the reader what the extension rule is, why every
target hypergraph appears in the final level, how the m-window per level is computed, or how
τ is filtered. A referee cannot check Theorem 1 for n ≥ 18 from the paper. The argument exists
and was reviewed twice (RESULTS.md "completeness argument", maxchain-review §"Why the argument
is right", review23 §2); it needs one paragraph. Suggested text, to replace or follow 3.1:

> **Maximum-degree chain.** Let H = H(A) on N points and let H_{k−1} = H_k − v_k with v_k a
> vertex of maximum degree d_k in H_k. Each H_k is the progression hypergraph of a k-point
> subset, hence linear, F7-free, with injective middles and m_k ≤ k − 2; τ(H_k) ≥ τ(H) − (N−k)
> since deleting a vertex lowers τ by at most one; Δ(H_k) ≤ Δ(H); and d_k ≥ 3m_k/k since a
> maximum degree is at least the average. Level k is built from level k − 1 by adding a vertex
> joined to d pairwise disjoint pairs that no edge already covers, with d ≥ Δ(parent), d at
> most the cap, and every touched vertex ending with degree ≤ d (so the new vertex is a
> maximum-degree vertex of the child). A child is kept only if it is F7-free, has an injective
> middle assignment, has τ ≥ τ_target − (N − k), and its (k, m) lies in the window computed
> backwards from m_N ∈ [m_min, m_max] by the recurrence m_{k−1} = m_k − d, 3m_k ≤ d·k, m_k ≤ k−2.
> τ is tested exactly: if the parent already meets the bound nothing changes; if it is one
> below, the child meets it iff no minimum transversal of the parent hits all d new pairs
> (a smaller transversal containing the new vertex would cover the parent with τ − 2 vertices).
> Every filter is invariant under relabelling of the parent, so by induction from the empty
> hypergraph the canonical form of each H_k is in level k. Degree cap: applying Henning–Yeo
> to H − v for a vertex of degree d gives 3d ≤ s + 12 with s = 5N + 3m − 17τ, i.e. Δ ≤ 4 for
> (n,m) = (16,13), (18,16), (20,18), (22,20), (23,19); Δ ≤ 5 for (16,14), (23,20); Δ ≤ 6 for
> (23,21). The HY-free runs use the trivial cap ⌊(n−1)/2⌋ instead.

Also give, for n = 20 and n = 22, the level sizes the way n = 18 has them (n=20:
47/528/2712/282/17/0 for k = 15..20; n=22: 45/318/250/95/3/0 for k = 17..22; n=23: 813 /
19,968 / 201,615 / 12,059,483 / 4,204,760 / 86,094 for k = 18..23, the last from the fixed
binary) so the enumeration is checkable against the released logs.

### B4. "Code, level data and controls … every number above comes from the released code" is not true today

Measured here: the public repo (and Zenodo v1.0.1, 2026-10-20, titled "no 16-point (4,5)-set
with Sidon number 9, and the 14-point classification") contains `chain.cpp`, `maxchain.cpp`
(fixed), `brute.cpp`, `dfs.cpp`, `realise.py`, `controls.py`, and only n = 14 / n = 16 data.
Missing: `rt.cpp`, `realise_fast.py`, `verify.py`, `maxresume.cpp`, `crosscheck.cpp`,
`n18-final.txt`, `n18-realise.json`, `n16-maxchain-realise.json`, the n = 20/22/23 logs,
`n23-level23-fixed-log.txt`, `out23fix/final.txt` (the 86,094 classes), `rt-n23.txt`,
`sample200.jsonl`. README does not mention n ≥ 18. Either publish v1.1 with these (and a new
Zenodo version; the concept DOI then resolves to it) before the paper goes out, or rewrite the
sentence to say what is released and what is available on request. The 86,094-class file and
the per-class `rt` output are the whole evidence for n = 23 and must be downloadable.

## Non-blocking

### N1. Theorem 3 (n = 14 classification) silently depends on HY; one 0.2-second run removes that

The 12 classes come from the min-chain with m ∈ [11,12]; m ≥ 11 is Henning–Yeo (17·6 ≤ 70+3m).
Measured here: `maxchain 14 0 12 6 6` (no HY: every m ≤ 12, trivial cap) gives 61 classes at
k = 14, all m = 12, τ = 6, in 0.2 s; `rt` on them: 9 R, 52 U, the same nine. Since the run is
τ ≥ 6 and finds no τ ≥ 7 class, it also gives f(14) ≥ 8 without HY. Suggested addition after
Theorem 3 or in Controls: *"A Henning–Yeo-free run over all m ≤ 12 returns 61 classes, all with
m = 12; the same nine are realisable, so Theorem 3 and f(14) = 8 do not depend on [HY21]."*
Also list the nine integer sets (they are in `n14-realise.json`; the paper names only the SAT
set), otherwise "one of our nine types" is unverifiable from the PDF. Measured here: all nine
are (4,5)-sets with h = 8 and 12 progressions, pairwise non-isomorphic; the SAT set is
isomorphic to the 6th, the EA280 set to the 8th (in `n14-realise.json` order).

### N2. Controls paragraph: which chain the brute-force generator was compared against

*"An independent edge-by-edge generator agrees with the chain on four smaller families"*
describes `brute.cpp` vs the min-degree chain, and one of the four is an inclusion, not
equality ((12,10,τ4): 83,549 ⊆ 83,554). The comparison that matters for Theorem 1 is
`brute.cpp` vs the max-degree chain: six families up to (13,11,τ5), 332,157 target classes,
none missing. Say both, and say the residual: `brute.cpp` cannot reach m ≥ 19, so the 22→23
step is the one level no independent generator has reproduced (review23 N2). Replacement:

> An independent generator that builds all linear F7-free 3-graphs edge by edge contains every
> class of the minimum-degree chain on four families and every class of the maximum-degree
> chain on six families (up to 13 points, 11 edges, 332,157 classes, none missing). It cannot
> reach m ≥ 19, so the final step of the n = 23 enumeration rests on the same code path as the
> checked levels, with the array bound of Remark 4 corrected.

### N3. "A second, independent implementation of the full enumeration agrees on 200 random classes"

Ambiguous ("full enumeration" reads as the chain). It is `realise_fast.py`: realise.py's
algorithm (every middle assignment, random point, exact vanishing on the integer nullspace)
with a FLINT nullspace, sharing `violation()` and `sdrs()` with realise.py and nothing with
`rt.cpp`; 200 classes, 11,838,299 assignments, 200/200 agree (`sample200.jsonl`, measured
here: 200 lines, all `agree: true`). Replacement: *"An exhaustive enumeration of all middle
assignments (11.8 million over 200 random classes), implemented separately from the
depth-first search, gives the same verdict on each."*

### N4. "15.6 million exact checks, no false alarm" is only in prose

`rt-n23.txt` carries per-class DFS nodes (sum 61,863,808, mean 718.6, max 38,993; measured
here) but not the exact-check counter. The 15,645,571 appears only in `realise23/RESULTS.md`.
Either have `rt` emit the counter in the released output or cite the node count instead.

### N5. Section 2: degree cap stated without derivation; "For these" is misleading

The cap Δ ≤ (s+12)/3 is correct (re-derived: HY on H − v, which is linear, F7-free, has
n − 1 points, m − d edges and τ ≥ τ − 1). One line in the paper. "For these [19 ≤ n ≤ 24]" is
wrong in scope: the n = 18 capped run uses Δ ≤ 4 too. Use the text in B3.

### N6. Section 3.1 does not say how the bounds propagate

State the two rules the min-chain uses: m_k ≥ m_target − 2(N − k) (a deleted vertex has
degree ≤ 2) and τ_k ≥ τ_target − (N − k).

### N7. Shape table and HY arithmetic: all correct (re-derived, every n ≤ 25, every h < 4n/7)

n = 16: τ = 7, m ∈ {13,14}, s ∈ {0,3}, Δ ≤ 4/5. n = 18: τ = 8, m = 16, s = 2, Δ ≤ 4.
n = 20: τ = 9, m = 18, s = 1, Δ ≤ 4. n = 22: τ = 10, m = 20, s = 0, Δ ≤ 4. n = 23: τ = 10,
m ∈ {19,20,21}, s ∈ {2,5,8}, Δ ≤ 4/5/6 (τ = 11 needs m ≥ 24 > 21). Every other n ≤ 24 is
empty (n = 24: τ ≥ 11 needs m ≥ 23 > 22). n = 25: τ = 11, m ∈ {21..23} is the next shape.
n = 14: τ ≤ 6 so h ≥ 8. All as in the paper.

### N8. Numbers checked against artifacts (all match)

Table 1 level sizes and "about 5 minutes" (151 + 105 + 37 s); 5184/5280; n = 18 levels
41/280/250/80/3 and 11,880/11,664/12,100; n = 20 and n = 22 zero at the top level; 86,094 =
152 + 85,942 (fixed log, 969 s); "up to 2.5·10^5 middle assignments per class" (max 253,440);
all 86,094 verdicts U with 0 surviving leaves; Fano-minus-a-line: each n = 16 survivor has
exactly one 7-subset carrying 6 edges (measured here); Ma–Tang lemma numbers 2.3, 2.4, 2.5,
5.2 and Theorem 5.3 = HY Theorem 7 (arXiv HTML v2); c = inf f(n)/n is Ma–Tang Theorem 1.5;
GL95 bounds and journal data; HY21 Graphs Combin. 37(3) 867–890; Ma–Tang v2 (6 Mar 2026) is
still the current version as of today.

### N9. Citations

- `Er97`: a paper with exactly these data exists (DOI 10.1016/s0012-365x(96)00173-2, Discrete
  Math. 165–166, 227–231). erdosproblems.com labels the source of #757 "[Er97b]"; its
  bibliography is loaded by script and I could not confirm the label maps to this paper.
  Check once and add the DOI.
- `EPAD`: the post is dated 19 Oct 2026 and reports an anonymous reader submission of 17 Oct;
  the set is S_K = O ∪ φ(O) ∪ … with 11 + 9K points and h = 7 + 5K (K = 5 is exactly 4/7 at 56
  points, K = 6 gives 37/65). Cite as *"Erdős Problem a Day, #757: a reader's self-similar set
  lowers the upper bound to 5/9 (anonymous submission 17 Oct 2026, posted 19 Oct 2026)"*.
- Add Erdős–Sós c ≥ 1/2 and the Fibonacci example for 3/5 (both on the problem page) to the
  first paragraph; one sentence.
- JOURNAL credits EinsteinArena threads 268/269 (h = n − τ reformulation) and 264; the paper
  cites only 280. Fine if nothing from them is used.

### N10. Related work sweep (arXiv API, OpenAlex, 2026-10-21): nothing missing that bears on the result

Queries: "weak Sidon", "Sidon subset", "(4,5)-set", "Erdős 757", "largest Sidon subsets".
Only Ma–Tang (2602.23282v2) and the EPAD post concern #757. Tangential 2026 items a referee
might mention but need not: "On the largest Sidon subset in a finite subset of R^N"
(arXiv 2605.03181, Eur. J. Combin. 2026, H(n) ≳ 0.19√n for arbitrary integer sets);
"Interval capacity and second-order bounds for distinct-difference configurations" (Zenodo
23112228, weak Sidon subsets of an interval). Tao's optimization-constants database (5b) still
lists 4/7. No conflict with the paper's claims.

### N11. Small things

- Remark 4: say the earlier n = 23 run reported 0 and the corrected step reports 86,094; the
  reader otherwise cannot tell how large the effect of the bug was.
- Section 3.2: the "if and only if" needs "with μ as its middle map and H(A) = H exactly";
  the forward direction (a realisation avoids every form) is the one the kill uses.
- Theorem numbering: amsart shares one counter, so the PDF has Theorem 1, Corollary 2,
  Theorem 3, Remark 4. Fine, but the abstract's "We also classify" refers to Theorem 3.
- "the τ = 7 designs found there realise only with coincident points" (attributed to EA280):
  not checked here.

## Measured here (commands)

```
python3 -I  shape table for n ≤ 25 (HY, m ≤ n−2, h < 4n/7) → matches paper
grep over n16/n18/n20/n20-hyfree/n22/n23/n23-fixed logs → level counts above
python3 -I  n16-realise.json, n18-realise.json, n16-maxchain-realise.json, n14-realise.json
awk  realise23/out/rt-n23.txt → 86,094 U, leaves 0, nodes 61,863,808; sample200.jsonl 200 agree
uv run --with networkx python -I check14.py → 9 types (4,5), h=8, m=12, pairwise non-iso;
     SAT set ≅ type 6, EA280 set ≅ type 8
python3 -I  n16-final.txt → each survivor has exactly one 7-subset with 6 edges
./maxchain 14 0 12 6 6 <scratch> 4 → 61 classes (m12/τ6), 0.2 s; ./realise23/rt → 9 R, 52 U
pdftotext main.pdf → matches main.tex
curl arxiv.org/abs/2602.23282, arxiv.org/html/2602.23282v2, export.arxiv.org API,
     api.openalex.org, erdosproblems.com/757, erdosproblemaday.com/day/757-..., zenodo API
ls ~/projects/erdos-757-sidon45 (HEAD 5d2ff35 == origin/main) → file list in B4
```
