// Cross-check between the two chains: for each final hypergraph of the max-degree chain,
// follow min-degree deletions down to level K0 (every min-degree vertex choice, DFS) and
// test whether some resulting H_K0 is in the min-degree chain's stored level file.
// usage: crosscheck N MMIN MMAX TAU final.txt levelK0.bin K0
#define main chain_main
#include "chain.cpp"
#undef main

static std::vector<G> lev;
static int K0;

static G del(const G &g, int v) {
  G h;
  h.k = g.k - 1;
  for (int i = 0; i < g.m; i++) {
    if (g.e[i] >> v & 1) continue;
    uint32_t e = g.e[i], lo = e & ((1u << v) - 1), hi = (e >> (v + 1)) << v;
    h.e[h.m++] = lo | hi;
  }
  return h;
}

static bool down(const G &g) {
  if (g.m < lo_m(g.k) || g.m > hi_m(g.k) || !tau_ge(g, lo_tau(g.k))) return false;
  if (g.k == K0) return std::binary_search(lev.begin(), lev.end(), canon(g));
  int deg[32] = {0}, dmin = 99;
  for (int i = 0; i < g.m; i++)
    for (int v = 0; v < g.k; v++) deg[v] += g.e[i] >> v & 1;
  for (int v = 0; v < g.k; v++) dmin = std::min(dmin, deg[v]);
  for (int v = 0; v < g.k; v++)
    if (deg[v] == dmin && down(del(g, v))) return true;
  return false;
}

int main(int argc, char **argv) {
  N = atoi(argv[1]); MMIN = atoi(argv[2]); MMAX = atoi(argv[3]); TAU = atoi(argv[4]);
  lev = read_level(argv[6]);
  K0 = atoi(argv[7]);
  FILE *f = fopen(argv[5], "r");
  char line[4096];
  while (fgets(line, sizeof line, f)) {
    G g;
    int kk, mm, tt, off;
    char *p = line;
    if (sscanf(p, "%d %d %d%n", &kk, &mm, &tt, &off) != 3) continue;
    p += off;
    g.k = kk;
    int a, b, d;
    while (sscanf(p, " %d,%d,%d%n", &a, &b, &d, &off) == 3) g.e[g.m++] = 1u << a | 1u << b | 1u << d, p += off;
    printf("%s", down(g) ? "IN min-chain level\n" : "NOT in min-chain level\n");
  }
}
