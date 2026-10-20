// Depth-first tail of the chain: from a stored level (k0) recurse to N without per-level
// dedup (memory-free; duplicates cost time only). Same extend() as chain.cpp.
// usage: dfs N MMIN MMAX TAU LEVELFILE k0 SHARD NSHARDS STRIDE OUTFILE
//   processes graphs i of the level with i % (NSHARDS*STRIDE) == SHARD*STRIDE... i.e. a
//   STRIDE>1 run is a uniform sample for cost estimates.
#define main chain_main
#include "chain.cpp"
#undef main

static long nodes[32];
static std::vector<G> finals;

static void rec(const G &g) {
  nodes[g.k]++;
  if (g.k == N) {
    finals.push_back(g);
    return;
  }
  std::vector<G> ch;
  extend(g, ch);
  dedup(ch);
  for (auto &h : ch) rec(h);
}

int main(int argc, char **argv) {
  N = atoi(argv[1]);
  MMIN = atoi(argv[2]);
  MMAX = atoi(argv[3]);
  TAU = atoi(argv[4]);
  std::string lf = argv[5];
  int k0 = atoi(argv[6]);
  long shard = atol(argv[7]), ns = atol(argv[8]), stride = atol(argv[9]);
  FILE *f = fopen(lf.c_str(), "rb");
  G g;
  long i = 0, done = 0;
  auto t0 = std::chrono::steady_clock::now();
  while (fread(&g, sizeof(G), 1, f) == 1) {
    if (i++ % (ns * stride) != shard) continue;
    rec(g);
    if (++done % 100000 == 0) {
      double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
      fprintf(stderr, "shard %ld: %ld roots %.0fs finals %zu\n", shard, done, s, finals.size());
    }
  }
  dedup(finals);
  double s = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  printf("shard %ld roots %ld time %.1fs", shard, done, s);
  for (int k = k0; k <= N; k++) printf(" n%d=%ld", k, nodes[k]);
  printf(" finals=%zu\n", finals.size());
  FILE *o = fopen(argv[10], "wb");
  for (auto &h : finals) fwrite(&h, sizeof(G), 1, o);
  fclose(o);
  return 0;
}
