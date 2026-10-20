// Independent completeness check for chain.cpp on small N: generate ALL linear 3-graphs on k
// fixed vertices edge by edge (nauty canon), keep F7-free, SDR, and "every vertex subset S
// spans <= |S|-2 edges" (true for any real (4,5)-set), then at m = M require tau >= TAU.
// Every survivor must be in chain's final level for (N=k, MMIN=MMAX=M, TAU).
// usage: brute k M TAU chain_final.txt
#define main chain_main
#include "chain.cpp"
#undef main
#include <set>

static bool subsets_ok(const G &g) {
  for (uint32_t S = 0; S < (1u << g.k); S++) {
    int s = __builtin_popcount(S);
    if (s < 3) continue;
    int c = 0;
    for (int i = 0; i < g.m; i++) c += (g.e[i] & ~S) == 0;
    if (c > s - 2) return false;
  }
  return true;
}

int main(int argc, char **argv) {
  int k = atoi(argv[1]), M = atoi(argv[2]), T = atoi(argv[3]);
  std::vector<uint32_t> tri;
  for (int a = 0; a < k; a++)
    for (int b = a + 1; b < k; b++)
      for (int c = b + 1; c < k; c++) tri.push_back(1u << a | 1u << b | 1u << c);
  std::vector<G> cur(1);
  cur[0].k = k;
  for (int j = 1; j <= M; j++) {
    std::vector<G> nxt;
    for (auto &g : cur)
      for (uint32_t t : tri) {
        bool lin = true;
        for (int i = 0; i < g.m && lin; i++) lin = __builtin_popcount(g.e[i] & t) <= 1;
        if (!lin) continue;
        G h = g;
        h.e[h.m++] = t;
        if (!has_sdr(h) || has_f7(h) || !subsets_ok(h)) continue;
        nxt.push_back(canon(h));
      }
    dedup(nxt);
    printf("edges=%d: %zu\n", j, nxt.size());
    cur.swap(nxt);
  }
  // chain final
  std::vector<G> fin;
  FILE *f = fopen(argv[4], "r");
  char line[4096];
  while (fgets(line, sizeof line, f)) {
    G g;
    int kk, mm, tt, off;
    char *p = line;
    sscanf(p, "%d %d %d%n", &kk, &mm, &tt, &off);
    p += off;
    g.k = kk;
    int a, b, d;
    while (sscanf(p, " %d,%d,%d%n", &a, &b, &d, &off) == 3) g.e[g.m++] = 1u << a | 1u << b | 1u << d, p += off;
    fin.push_back(canon(g));
  }
  dedup(fin);
  long need = 0, miss = 0;
  for (auto &g : cur) {
    if (!tau_ge(g, T)) continue;
    need++;
    if (!std::binary_search(fin.begin(), fin.end(), g)) miss++;
  }
  printf("brute: %ld targets (tau>=%d), chain final has %zu, missing from chain: %ld\n", need, T,
         fin.size(), miss);
  return miss != 0;
}
