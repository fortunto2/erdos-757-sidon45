// Vertex-deletion chain enumeration for Erdős #757 (spec: research/scientist-757.md).
//
// Enumerates, up to isomorphism, 3-uniform linear hypergraphs H on k = 0..N vertices with
//   m <= k-2, m <= MMAX, m >= MMIN - 2(N-k), tau >= TAU - (N-k),
//   F7-free, distinct-middle assignment (SDR) exists,
// level k from level k-1 by adding one vertex of degree 0/1/2 that is a minimum-degree
// vertex of the result. Complete for the targets: every target H_N has a min-degree vertex
// of degree <= 2 (3m < 3k), its deletion keeps every hereditary condition above and drops
// tau by at most 1, so the whole deletion sequence lives inside the levels.
//
// Canonical form: nauty (densenauty) on the vertex/edge incidence graph, two colour cells.
// Parallel: input level is sharded over P forked workers (nauty here is not built with TLS).
//
// usage: chain N MMIN MMAX TAU OUTDIR [P] [control_file]
//   control_file lines: "k e1 e2 ..." with edges "a,b,c"; prints whether each is found.

#define MAXN WORDSIZE
#include <nauty/nauty.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

struct G {
  uint8_t k = 0, m = 0;
  static constexpr int MAXE = 32;  // was 18: overflowed for m > 18 (n=22,23), found in maxchain-review.md
  uint32_t e[MAXE] = {0};
  bool operator<(const G &o) const {
    if (m != o.m) return m < o.m;
    return memcmp(e, o.e, sizeof(uint32_t) * m) < 0;
  }
  bool operator==(const G &o) const {
    return m == o.m && memcmp(e, o.e, sizeof(uint32_t) * m) == 0;
  }
};

static int N, MMIN, MMAX, TAU;
// optional hereditary filters (env): MAXDEG=d (HY-derived degree cap of the target),
// DENSE=1 (every vertex set S spans <= |S|-2 edges; true for any real (4,5)-set)
static int MAXDEG = getenv("MAXDEG") ? atoi(getenv("MAXDEG")) : 0;
static int DENSE = getenv("DENSE") ? atoi(getenv("DENSE")) : 0;

struct G;
static bool deg_ok(const G &g, int cap);
static bool sparse_ok(const G &g);

static inline int lo_m(int k) { return MMIN - 2 * (N - k); }
static inline int hi_m(int k) { return std::min(k >= 2 ? k - 2 : 0, MMAX); }
static inline int lo_tau(int k) { return TAU - (N - k); }

static bool deg_ok(const G &g, int cap) {
  for (int v = 0; v < g.k; v++) {
    int d = 0;
    for (int i = 0; i < g.m; i++) d += g.e[i] >> v & 1;
    if (d > cap) return false;
  }
  return true;
}
// Violators of e(S) <= |S|-2 must contain the newest vertex (the parent level passed), so
// only sets S containing vertex k-1 are scanned; only edge unions matter (a violator can
// be shrunk to the union of its edges), enumerated by DFS over edges.
static bool sp_rec(const G &g, int i, uint32_t S, int cnt) {
  if (cnt > 0 && cnt >= __builtin_popcount(S) - 1) return false;
  for (int j = i; j < g.m; j++) {
    uint32_t T = S | g.e[j];
    if (T == S) continue;
    int c = 0;
    for (int t = 0; t < g.m; t++) c += (g.e[t] & ~T) == 0;
    if (!sp_rec(g, j + 1, T, c)) return false;
  }
  return true;
}
static bool sparse_ok(const G &g) {
  uint32_t v = 1u << (g.k - 1);
  for (int i = 0; i < g.m; i++)
    if (g.e[i] & v) {
      if (!sp_rec(g, 0, g.e[i], 1)) return false;
    }
  return true;
}

// transversal of size <= L exists?
static bool hit(const uint32_t *e, int m, uint32_t cov, int L) {
  int i = 0;
  while (i < m && (e[i] & cov)) i++;
  if (i == m) return true;
  if (L == 0) return false;
  uint32_t x = e[i];
  while (x) {
    uint32_t b = x & -x;
    x ^= b;
    if (hit(e, m, cov | b, L - 1)) return true;
  }
  return false;
}

int tau_of(const G &g) {
  for (int L = 0;; L++)
    if (hit(g.e, g.m, 0, L)) return L;
}

static bool tau_ge(const G &g, int t) { return t <= 0 || !hit(g.e, g.m, 0, t - 1); }

// all transversals of size exactly L, assuming none smaller exists (so they are minimal and
// every one is reached by branching on the first unhit edge)
static void mt_rec(const uint32_t *e, int m, uint32_t cov, int L, std::vector<uint32_t> &out) {
  int i = 0;
  while (i < m && (e[i] & cov)) i++;
  if (i == m) {
    out.push_back(cov);
    return;
  }
  if (L == 0) return;
  uint32_t x = e[i];
  while (x) {
    uint32_t b = x & -x;
    x ^= b;
    mt_rec(e, m, cov | b, L - 1, out);
  }
}
static void min_transversals(const G &g, int L, std::vector<uint32_t> &out) {
  mt_rec(g.e, g.m, 0, L, out);
  std::sort(out.begin(), out.end());
  out.erase(std::unique(out.begin(), out.end()), out.end());
}

// SDR: each edge gets a distinct vertex of its own (Kuhn).
static bool aug(const G &g, int i, uint32_t &seen, int *owner) {
  uint32_t x = g.e[i];
  while (x) {
    int v = __builtin_ctz(x);
    x &= x - 1;
    if (seen >> v & 1) continue;
    seen |= 1u << v;
    if (owner[v] < 0 || aug(g, owner[v], seen, owner)) {
      owner[v] = i;
      return true;
    }
  }
  return false;
}
static bool has_sdr(const G &g) {
  int owner[32];
  for (int i = 0; i < 32; i++) owner[i] = -1;
  for (int i = 0; i < g.m; i++) {
    uint32_t seen = 0;
    if (!aug(g, i, seen, owner)) return false;
  }
  return true;
}

// F7 = {012,034,056,135,246}: centre c on three edges, two disjoint edges avoiding c inside
// the six other points.
static bool has_f7(const G &g) {
  for (int c = 0; c < g.k; c++) {
    int through[18], t = 0;
    for (int i = 0; i < g.m; i++)
      if (g.e[i] >> c & 1) through[t++] = i;
    if (t < 3) continue;
    uint32_t cb = 1u << c;
    for (int a = 0; a < t; a++)
      for (int b = a + 1; b < t; b++)
        for (int d = b + 1; d < t; d++) {
          uint32_t P = (g.e[through[a]] | g.e[through[b]] | g.e[through[d]]) & ~cb;
          uint32_t inside[G::MAXE];
          int s = 0;
          for (int i = 0; i < g.m; i++)
            if (!(g.e[i] & cb) && (g.e[i] & ~P) == 0) inside[s++] = g.e[i];
          for (int x = 0; x < s; x++)
            for (int y = x + 1; y < s; y++)
              if (!(inside[x] & inside[y])) return true;
        }
  }
  return false;
}

static G canon(const G &g) {
  int n = g.k + g.m;
  graph gr[MAXN], cg[MAXN];
  int lab[MAXN], ptn[MAXN], orb[MAXN];
  EMPTYGRAPH(gr, 1, n);
  for (int j = 0; j < g.m; j++)
    for (int v = 0; v < g.k; v++)
      if (g.e[j] >> v & 1) ADDONEEDGE(gr, v, g.k + j, 1);
  for (int i = 0; i < n; i++) lab[i] = i, ptn[i] = 1;
  if (g.k) ptn[g.k - 1] = 0;
  ptn[n - 1] = 0;
  static DEFAULTOPTIONS_GRAPH(opt);
  opt.getcanon = TRUE;
  opt.defaultptn = FALSE;
  statsblk st;
  densenauty(gr, lab, ptn, orb, &opt, &st, 1, n, cg);
  G h;
  h.k = g.k;
  h.m = g.m;
  for (int j = 0; j < g.m; j++) {
    setword row = cg[g.k + j];
    uint32_t mask = 0;
    for (int v = 0; v < g.k; v++)
      if (row & bit[v]) mask |= 1u << v;
    h.e[j] = mask;
  }
  std::sort(h.e, h.e + h.m);
  return h;
}

static void extend(const G &g, std::vector<G> &out) {
  int k = g.k + 1;
  uint32_t nv = 1u << g.k;
  int deg[32] = {0};
  for (int i = 0; i < g.m; i++)
    for (int v = 0; v < g.k; v++) deg[v] += g.e[i] >> v & 1;
  uint32_t z0 = 0, z1 = 0;  // degree 0 / degree <= 1
  for (int v = 0; v < g.k; v++) {
    if (deg[v] == 0) z0 |= 1u << v;
    if (deg[v] <= 1) z1 |= 1u << v;
  }
  uint32_t covered[32] = {0};  // covered[a] = vertices sharing an edge with a
  for (int i = 0; i < g.m; i++)
    for (int v = 0; v < g.k; v++)
      if (g.e[i] >> v & 1) covered[v] |= g.e[i];
  std::vector<uint32_t> pairs;
  for (int a = 0; a < g.k; a++)
    for (int b = a + 1; b < g.k; b++)
      if (!(covered[a] >> b & 1)) pairs.push_back(1u << a | 1u << b);

  int tl = lo_tau(k);
  // tau(h) >= tl. tau(g) >= tl - 1 by the level bound; if tau(g) = tl - 1, the new edges
  // must break every minimum transversal T of g (a set containing the new vertex cannot
  // be smaller than tau(g) + 1). So a new pair p must miss T, recorded as bitsets S_p.
  bool need = !tau_ge(g, tl);
  std::vector<uint32_t> T;
  if (need) min_transversals(g, tl - 1, T);
  size_t W = (T.size() + 63) / 64;
  std::vector<uint64_t> S(pairs.size() * std::max<size_t>(W, 1), 0);
  if (need)
    for (size_t i = 0; i < pairs.size(); i++)
      for (size_t t = 0; t < T.size(); t++)
        if (T[t] & pairs[i]) S[i * W + t / 64] |= 1ull << (t % 64);
  auto empty = [&](size_t i) {
    for (size_t w = 0; w < W; w++)
      if (S[i * W + w]) return false;
    return true;
  };
  auto disjoint = [&](size_t i, size_t j) {
    for (size_t w = 0; w < W; w++)
      if (S[i * W + w] & S[j * W + w]) return false;
    return true;
  };
  auto take = [&](G &h) {
    if (!has_sdr(h) || has_f7(h)) return;
    if (MAXDEG && !deg_ok(h, MAXDEG)) return;
    if (DENSE && !sparse_ok(h)) return;
    out.push_back(canon(h));
  };
  auto mok = [&](int d) { return g.m + d >= lo_m(k) && g.m + d <= hi_m(k); };
  // degree 0
  if (mok(0) && !need) {
    G h = g;
    h.k = k;
    take(h);
  }
  // degree 1: every degree-0 vertex must be touched
  if (mok(1))
    for (size_t i = 0; i < pairs.size(); i++) {
      uint32_t p = pairs[i];
      if (z0 & ~p) continue;
      if (need && !empty(i)) continue;
      G h = g;
      h.k = k;
      h.e[h.m++] = p | nv;
      take(h);
    }
  // degree 2: no degree-0 vertex, every degree-1 vertex touched
  if (z0 || !mok(2)) return;
  for (size_t i = 0; i < pairs.size(); i++)
    for (size_t j = i + 1; j < pairs.size(); j++) {
      uint32_t p = pairs[i], q = pairs[j];
      if (p & q) continue;
      if (z1 & ~(p | q)) continue;
      if (need && !disjoint(i, j)) continue;
      G h = g;
      h.k = k;
      h.e[h.m++] = p | nv;
      h.e[h.m++] = q | nv;
      take(h);
    }
}

static void write_level(const std::string &path, const std::vector<G> &v) {
  FILE *f = fopen(path.c_str(), "wb");
  if (!f) { perror(path.c_str()); exit(3); }  // missing OUTDIR used to segfault after level 1
  for (auto &g : v) fwrite(&g, sizeof(G), 1, f);
  fclose(f);
}
static std::vector<G> read_level(const std::string &path) {
  std::vector<G> v;
  FILE *f = fopen(path.c_str(), "rb");
  if (!f) return v;
  G g;
  while (fread(&g, sizeof(G), 1, f) == 1) v.push_back(g);
  fclose(f);
  return v;
}

static void dedup(std::vector<G> &v) {
  std::sort(v.begin(), v.end());
  v.erase(std::unique(v.begin(), v.end()), v.end());
}

int main(int argc, char **argv) {
  if (argc < 6) {
    fprintf(stderr, "usage: chain N MMIN MMAX TAU OUTDIR [P] [control]\n");
    return 1;
  }
  N = atoi(argv[1]);
  MMIN = atoi(argv[2]);
  MMAX = atoi(argv[3]);
  if (MMAX > G::MAXE) { fprintf(stderr, "MMAX %d exceeds edge capacity %d\n", MMAX, G::MAXE); return 2; }  // MAXE guard
  TAU = atoi(argv[4]);
  std::string dir = argv[5];
  int P = argc > 6 ? atoi(argv[6]) : 1;
  nauty_check(WORDSIZE, 1, MAXN, NAUTYVERSIONID);

  std::vector<G> cur(1);  // k = 0, empty
  int k0 = 1;
  if (getenv("START")) {  // resume: START=k loads OUTDIR/level<k>.bin
    int s = atoi(getenv("START"));
    cur = read_level(dir + "/level" + std::to_string(s) + ".bin");
    k0 = s + 1;
    fprintf(stderr, "resumed level %d: %zu\n", s, cur.size());
  }
  auto t0 = std::chrono::steady_clock::now();
  for (int k = k0; k <= N; k++) {
    std::vector<G> nxt;
    if (P <= 1 || cur.size() < 1000) {
      for (auto &g : cur) extend(g, nxt);
    } else {
      std::vector<pid_t> kids;
      for (int w = 0; w < P; w++) {
        pid_t pid = fork();
        if (pid == 0) {
          std::vector<G> o;
          for (size_t i = w; i < cur.size(); i += P) extend(cur[i], o);
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
    for (auto &g : nxt) by[{g.m, tau_of(g)}]++;
    double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
    printf("level k=%d: %zu classes (raw %zu)  m in [%d,%d] tau>=%d  t=%.1fs |", k, nxt.size(),
           raw, std::max(0, lo_m(k)), hi_m(k), lo_tau(k), sec);
    for (auto &[key, c] : by) printf(" m%d/t%d:%ld", key.first, key.second, c);
    printf("\n");
    fflush(stdout);
    write_level(dir + "/level" + std::to_string(k) + ".bin", nxt);
    cur.swap(nxt);
  }
  // final level as text
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
  if (argc > 7) {
    FILE *c = fopen(argv[7], "r");
    char line[4096];
    while (fgets(line, sizeof line, c)) {
      G g;
      char *p = line;
      char name[64];
      int off;
      int kk;
      if (sscanf(p, "%63s %d%n", name, &kk, &off) != 2) continue;
      p += off;
      g.k = kk;
      int a, b, d;
      while (sscanf(p, " %d,%d,%d%n", &a, &b, &d, &off) == 3) {
        g.e[g.m++] = 1u << a | 1u << b | 1u << d;
        p += off;
      }
      G h = canon(g);
      bool found = std::binary_search(cur.begin(), cur.end(), h);
      printf("control %s (k=%d m=%d tau=%d sdr=%d f7=%d): %s\n", name, g.k, g.m, tau_of(g),
             has_sdr(g), has_f7(g), found ? "FOUND" : "MISSING");
    }
  }
  return 0;
}
