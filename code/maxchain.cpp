// Max-degree vertex-deletion chain (variant of chain.cpp; same filters, canon, controls).
//
// Delete a MAXIMUM-degree vertex at every step instead of a minimum-degree one. tau still
// drops by at most 1 per deletion (tau_k >= TAU - (N-k)), but m now falls fast, so every
// level holds sparse graphs with large tau: near Henning-Yeo-extremal and few.
// Uses Henning-Yeo (17 tau <= 5n + 3m for linear F7-free 3-graphs) once, for the target:
// deleting v of degree d gives 17(tau-1) <= 5(n-1) + 3(m-d), so Delta <= (s+12)/3 with
// s = 5n + 3m - 17 tau. DCAP is that cap; it is hereditary (degrees only drop).
// The m window per level comes from a DP: m_{k+1} = m_k + d, DCAP >= d >= ceil(3 m_{k+1}/(k+1))
// (max degree >= average), m_k <= k-2, ending in [MMIN, MMAX] at N.
// Every level graph is also checked against HY itself (a violation means a bug or a false
// theorem; it is counted, never silently dropped).
//
// usage: maxchain N MMIN MMAX TAU DCAP OUTDIR [P] [control]
#define main chain_main
#include "chain.cpp"
#undef main

static int DCAP;
static bool feas[40][64];

static void build_windows() {
  for (int m = 0; m < 64; m++) feas[N][m] = m >= MMIN && m <= MMAX;
  for (int k = N - 1; k >= 0; k--)
    for (int m = 0; m < 64; m++) {
      feas[k][m] = false;
      if (m > (k >= 2 ? k - 2 : 0)) continue;
      for (int d = 0; d <= DCAP && m + d < 64; d++) {
        int m2 = m + d;
        if (!feas[k + 1][m2]) continue;
        if (3 * m2 > d * (k + 1)) continue;  // d >= average degree of H_{k+1}
        feas[k][m] = true;
      }
    }
}

// all ways to pick d pairwise-disjoint pairs (indices increasing) whose transversal-hit sets
// have empty common intersection (if need) and whose touched vertices keep degree < d+1.
static void pick(const G &g, int k, const std::vector<uint32_t> &pairs, const std::vector<uint64_t> &S,
                 size_t W, bool need, int d, size_t from, uint32_t used, std::vector<uint64_t> &acc,
                 uint32_t *chosen, int c, uint32_t forbid, std::vector<G> &out) {
  if (c == d) {
    if (need)
      for (size_t w = 0; w < W; w++)
        if (acc[w]) return;
    G h = g;
    h.k = k;
    uint32_t nv = 1u << g.k;
    for (int i = 0; i < d; i++) h.e[h.m++] = chosen[i] | nv;
    if (!has_sdr(h) || has_f7(h)) return;
    out.push_back(canon(h));
    return;
  }
  for (size_t i = from; i < pairs.size(); i++) {
    uint32_t p = pairs[i];
    if (p & (used | forbid)) continue;
    std::vector<uint64_t> a2(acc);
    if (need)
      for (size_t w = 0; w < W; w++) a2[w] &= S[i * W + w];
    chosen[c] = p;
    pick(g, k, pairs, S, W, need, d, i + 1, used | p, a2, chosen, c + 1, forbid, out);
  }
}

static void extend_max(const G &g, std::vector<G> &out) {
  int k = g.k + 1;
  int deg[32] = {0};
  for (int i = 0; i < g.m; i++)
    for (int v = 0; v < g.k; v++) deg[v] += g.e[i] >> v & 1;
  int dmax = 0;
  for (int v = 0; v < g.k; v++) dmax = std::max(dmax, deg[v]);
  uint32_t covered[32] = {0};
  for (int i = 0; i < g.m; i++)
    for (int v = 0; v < g.k; v++)
      if (g.e[i] >> v & 1) covered[v] |= g.e[i];
  std::vector<uint32_t> pairs;
  for (int a = 0; a < g.k; a++)
    for (int b = a + 1; b < g.k; b++)
      if (!(covered[a] >> b & 1)) pairs.push_back(1u << a | 1u << b);
  int tl = lo_tau(k);
  bool need = !tau_ge(g, tl);
  if (need && !tau_ge(g, tl - 1)) return;
  std::vector<uint32_t> T;
  if (need) min_transversals(g, tl - 1, T);
  size_t W = std::max<size_t>((T.size() + 63) / 64, 1);
  std::vector<uint64_t> S(pairs.size() * W, 0);
  if (need)
    for (size_t i = 0; i < pairs.size(); i++)
      for (size_t t = 0; t < T.size(); t++)
        if (T[t] & pairs[i]) S[i * W + t / 64] |= 1ull << (t % 64);
  for (int d = std::max(dmax, 0); d <= DCAP; d++) {
    if (!feas[k][g.m + d]) continue;
    if (d == 0 && need) continue;
    // new vertex is a max-degree vertex: touched vertices need deg + 1 <= d
    uint32_t forbid = 0;
    for (int v = 0; v < g.k; v++)
      if (deg[v] + 1 > d) forbid |= 1u << v;
    std::vector<uint64_t> acc(W, ~0ull);
    if (need && T.size() % 64) acc[W - 1] = (1ull << (T.size() % 64)) - 1;
    uint32_t chosen[16];  // was 8: overflow at degree 9 (unreached in runs)
    pick(g, k, pairs, S, W, need, d, 0, 0, acc, chosen, 0, forbid, out);
  }
}

int main(int argc, char **argv) {
  if (argc < 7) {
    fprintf(stderr, "usage: maxchain N MMIN MMAX TAU DCAP OUTDIR [P] [control]\n");
    return 1;
  }
  N = atoi(argv[1]);
  MMIN = atoi(argv[2]);
  MMAX = atoi(argv[3]);
  if (MMAX > G::MAXE) { fprintf(stderr, "MMAX %d exceeds edge capacity %d\n", MMAX, G::MAXE); return 2; }  // MAXE guard
  TAU = atoi(argv[4]);
  DCAP = atoi(argv[5]);
  std::string dir = argv[6];
  int P = argc > 7 ? atoi(argv[7]) : 1;
  nauty_check(WORDSIZE, 1, MAXN, NAUTYVERSIONID);
  build_windows();
  for (int k = 0; k <= N; k++) {
    printf("window k=%d: m in {", k);
    for (int m = 0; m < 64; m++)
      if (feas[k][m]) printf(" %d", m);
    printf(" }\n");
  }
  std::vector<G> cur(1);
  auto t0 = std::chrono::steady_clock::now();
  for (int k = 1; k <= N; k++) {
    std::vector<G> nxt;
    if (P <= 1 || cur.size() < 1000) {
      for (auto &g : cur) extend_max(g, nxt);
    } else {
      std::vector<pid_t> kids;
      for (int w = 0; w < P; w++) {
        pid_t pid = fork();
        if (pid == 0) {
          std::vector<G> o;
          for (size_t i = w; i < cur.size(); i += P) extend_max(cur[i], o);
          dedup(o);
          write_level(dir + "/shard" + std::to_string(w) + ".bin", o);
          _exit(0);
        }
        kids.push_back(pid);
      }
      for (pid_t p : kids) waitpid(p, nullptr, 0);
      for (int w = 0; w < P; w++) {
        std::string s = dir + "/shard" + std::to_string(w) + ".bin";
        auto o = read_level(s);
        nxt.insert(nxt.end(), o.begin(), o.end());
        unlink(s.c_str());
      }
    }
    size_t raw = nxt.size();
    dedup(nxt);
    std::map<std::pair<int, int>, long> by;
    long hy_bad = 0;
    for (auto &g : nxt) {
      int t = tau_of(g);
      by[{g.m, t}]++;
      if (17 * t > 5 * g.k + 3 * g.m) hy_bad++;
    }
    double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    printf("level k=%d: %zu classes (raw %zu) tau>=%d HY-violations=%ld t=%.1fs |", k, nxt.size(),
           raw, lo_tau(k), hy_bad, sec);
    for (auto &[key, c] : by) printf(" m%d/t%d:%ld", key.first, key.second, c);
    printf("\n");
    fflush(stdout);
    write_level(dir + "/level" + std::to_string(k) + ".bin", nxt);
    cur.swap(nxt);
  }
  FILE *f = fopen((dir + "/final.txt").c_str(), "w");
  for (auto &g : cur) {
    fprintf(f, "%d %d %d", g.k, g.m, tau_of(g));
    for (int i = 0; i < g.m; i++) {
      fprintf(f, " ");
      int c = 0;
      for (int v = 0; v < g.k; v++)
        if (g.e[i] >> v & 1) fprintf(f, c++ ? ",%d" : "%d", v);
    }
    fprintf(f, "\n");
  }
  fclose(f);
  if (argc > 8) {
    FILE *c = fopen(argv[8], "r");
    char line[4096];
    while (fgets(line, sizeof line, c)) {
      G g;
      char *p = line;
      char name[64];
      int off, kk;
      if (sscanf(p, "%63s %d%n", name, &kk, &off) != 2) continue;
      p += off;
      g.k = kk;
      int a, b, d;
      while (sscanf(p, " %d,%d,%d%n", &a, &b, &d, &off) == 3) g.e[g.m++] = 1u << a | 1u << b | 1u << d, p += off;
      G h = canon(g);
      printf("control %s: %s\n", name, std::binary_search(cur.begin(), cur.end(), h) ? "FOUND" : "MISSING");
    }
  }
  return 0;
}
