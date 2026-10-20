"""AP hypergraphs of the two known 14-point (4,5)-sets, used as known-answer controls."""
import itertools, json, sys

SETS = {
    "ma_tang": [0, 136, 200, 243, 246, 249, 272, 286, 298, 323, 400, 528, 596, 1056],
    "sat14": [0, 323, 416, 468, 509, 517, 537, 557, 566, 575, 613, 646, 717, 1018],
}


def ap_edges(xs):
    xs = sorted(xs)
    idx = {x: i for i, x in enumerate(xs)}
    out = []
    for i, j in itertools.combinations(range(len(xs)), 2):
        s = xs[i] + xs[j]
        if s % 2 == 0 and s // 2 in idx:
            out.append(tuple(sorted((i, idx[s // 2], j))))
    return out


def tau(n, edges):
    best = [n]
    def rec(cov, size):
        if size >= best[0]:
            return
        for e in edges:
            if not (cov & sum(1 << v for v in e)):
                for v in e:
                    rec(cov | 1 << v, size + 1)
                return
        best[0] = size
    rec(0, 0)
    return best[0]


if __name__ == "__main__":
    for name, xs in SETS.items():
        e = ap_edges(xs)
        print(name, len(xs), len(e), tau(len(xs), e), " ".join("%d,%d,%d" % t for t in e))
