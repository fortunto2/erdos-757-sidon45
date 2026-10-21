// Fast realisability test for abstract AP hypergraphs (Erdős #757, (4,5)-sets).
//
// Same mathematics as ../realise.py: H with a distinct-middle assignment mu is realised iff no
// "bad" linear form vanishes on V = {x : x_a + x_c = 2 x_b for every edge (a,b,c) with middle b}.
// Bad forms: x_i - x_j (distinct points), x_a + x_c - 2 x_b for a triple {a,b,c} that is not an
// edge (extra 3-AP), x_i + x_j - x_k - x_l on 4 distinct indices (weak Sidon).
//
// Search: DFS over mu, one edge at a time; V shrinks as equations are added, so a bad form that
// vanishes on the partial V vanishes for every completion and the whole subtree dies.
// Vanishing is detected mod p = 2^61-1 on two random points of V_p, and every prune is
// CONFIRMED EXACTLY over Q (fraction-free Bareiss rank in __int128: f in rowspan(R) iff
// rank(R) == rank(R + f)). A mod-p false alarm is therefore never used to prune. A leaf that
// survives is only a candidate: it is printed and re-checked exactly by verify.py.
//
// usage: rt FINAL_TXT [--count] [--shard i/k]
//   default: per class stop at the first surviving leaf; --count: count surviving leaves (controls)
// output per class: idx verdict(R/U) nodes leaves_surviving density_violation fano hy_f7 |S| [mu if R]

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

typedef unsigned long long u64;
typedef __int128 i128;
static const u64 P = (1ULL << 61) - 1;
static inline u64 mulm(u64 a, u64 b) {
  unsigned __int128 z = (unsigned __int128)a * b;
  u64 lo = (u64)(z & P), hi = (u64)(z >> 61);
  u64 r = lo + hi;
  return r >= P ? r - P : r;
}
static inline u64 addm(u64 a, u64 b) { u64 r = a + b; return r >= P ? r - P : r; }
static inline u64 subm(u64 a, u64 b) { return a >= b ? a - b : a + P - b; }
static u64 powm(u64 a, u64 e) { u64 r = 1; while (e) { if (e & 1) r = mulm(r, a); a = mulm(a, a); e >>= 1; } return r; }
static inline u64 invm(u64 a) { return powm(a, P - 2); }

const int MAXN = 32, MAXM = 40;
int n, m;
int E[MAXM][3];
int third[MAXN][MAXN];  // third vertex of the edge through pair, -1 if none (linear)
int mid[MAXM];
bool usedv[MAXN];
u64 Z[MAXM + 1][MAXN][MAXN];  // basis of V_p per depth
int dimZ[MAXM + 1];
u64 C1[MAXN], C2[MAXN];
bool countMode = false;
long long nodes, leaves, exactChecks, falseAlarms;
int foundMu[MAXM];
bool found;

// rows of assigned equations, exact
static void eqRow(int e, int b, long long *r) {
  for (int i = 0; i < n; i++) r[i] = 0;
  for (int t = 0; t < 3; t++) r[E[e][t]] += 1;
  r[b] -= 3;
}

static int bareissRank(std::vector<std::vector<i128>> M) {
  int rows = M.size(), rank = 0;
  i128 prev = 1;
  for (int c = 0; c < n && rank < rows; c++) {
    int p = -1;
    for (int i = rank; i < rows; i++) if (M[i][c] != 0) { p = i; break; }
    if (p < 0) continue;
    std::swap(M[p], M[rank]);
    for (int i = rank + 1; i < rows; i++) {
      for (int j = c + 1; j < n; j++) {
        M[i][j] = (M[rank][c] * M[i][j] - M[i][c] * M[rank][j]) / prev;
      }
      M[i][c] = 0;
    }
    prev = M[rank][c];
    rank++;
  }
  return rank;
}

// exact: does form f vanish on V of the first k assigned edges (order[0..k-1])?
int order_[MAXM];
static bool vanishesExact(int k, const long long *f) {
  exactChecks++;
  std::vector<std::vector<i128>> R;
  long long r[MAXN];
  for (int i = 0; i < k; i++) {
    eqRow(order_[i], mid[order_[i]], r);
    R.push_back(std::vector<i128>(r, r + n));
  }
  int r0 = bareissRank(R);
  R.push_back(std::vector<i128>(f, f + n));
  int r1 = bareissRank(R);
  return r0 == r1;
}

// returns true if some bad form vanishes (exactly confirmed) on V at depth k
static bool deadAt(int k) {
  int d = dimZ[k];
  u64 y1[MAXN], y2[MAXN];
  for (int i = 0; i < n; i++) {
    u64 a = 0, b = 0;
    for (int j = 0; j < d; j++) { a = addm(a, mulm(C1[j], Z[k][j][i])); b = addm(b, mulm(C2[j], Z[k][j][i])); }
    y1[i] = a; y2[i] = b;
  }
  long long f[MAXN];
  // distinct points
  for (int i = 0; i < n; i++)
    for (int j = i + 1; j < n; j++)
      if (y1[i] == y1[j] && y2[i] == y2[j]) {
        memset(f, 0, sizeof f); f[i] = 1; f[j] = -1;
        if (vanishesExact(k, f)) return true;
        falseAlarms++;
      }
  // extra 3-APs: y_a + y_c = 2 y_b, {a,b,c} not an edge
  static const u64 inv2 = invm(2);
  for (int a = 0; a < n; a++)
    for (int c = a + 1; c < n; c++) {
      u64 h1 = mulm(addm(y1[a], y1[c]), inv2), h2 = mulm(addm(y2[a], y2[c]), inv2);
      for (int b = 0; b < n; b++) {
        if (b == a || b == c || y1[b] != h1 || y2[b] != h2) continue;
        if (third[a][c] == b) continue;  // an edge of H: wanted (other middle => distinctness)
        memset(f, 0, sizeof f); f[a] = 1; f[c] = 1; f[b] = -2;
        if (vanishesExact(k, f)) return true;
        falseAlarms++;
      }
    }
  // weak Sidon: equal pair sums on 4 distinct indices
  std::vector<std::pair<std::pair<u64, u64>, int>> S;
  S.reserve(n * n / 2);
  for (int i = 0; i < n; i++)
    for (int j = i + 1; j < n; j++) S.push_back({{addm(y1[i], y1[j]), addm(y2[i], y2[j])}, i * MAXN + j});
  std::sort(S.begin(), S.end());
  for (size_t s = 0; s < S.size();) {
    size_t t = s;
    while (t < S.size() && S[t].first == S[s].first) t++;
    for (size_t u = s; u < t; u++)
      for (size_t v = u + 1; v < t; v++) {
        int i = S[u].second / MAXN, j = S[u].second % MAXN, kk = S[v].second / MAXN, l = S[v].second % MAXN;
        if (i == kk || i == l || j == kk || j == l) continue;  // shares an index => distinctness, caught above
        memset(f, 0, sizeof f); f[i] += 1; f[j] += 1; f[kk] -= 1; f[l] -= 1;
        if (vanishesExact(k, f)) return true;
        falseAlarms++;
      }
    s = t;
  }
  return false;
}

static void dfs(int k) {
  nodes++;
  if (k > 0 && deadAt(k)) return;
  if (k == m) {
    leaves++;
    if (!found) { found = true; for (int i = 0; i < m; i++) foundMu[i] = mid[i]; }
    return;
  }
  int e = order_[k];
  for (int t = 0; t < 3; t++) {
    int b = E[e][t];
    if (usedv[b]) continue;
    // add equation e with middle b to the basis
    u64 r[MAXN];
    for (int i = 0; i < n; i++) r[i] = 0;
    for (int s = 0; s < 3; s++) r[E[e][s]] = 1;
    r[b] = P - 2;
    int d = dimZ[k];
    u64 tv[MAXN];
    int piv = -1;
    for (int j = 0; j < d; j++) {
      u64 s = 0;
      for (int i = 0; i < n; i++) if (r[i]) s = addm(s, mulm(r[i], Z[k][j][i]));
      tv[j] = s;
      if (s && piv < 0) piv = j;
    }
    int nd = 0;
    if (piv < 0) {
      memcpy(Z[k + 1], Z[k], sizeof(Z[k]));
      nd = d;
    } else {
      u64 ip = invm(tv[piv]);
      for (int j = 0; j < d; j++) {
        if (j == piv) continue;
        u64 c = mulm(tv[j], ip);
        for (int i = 0; i < n; i++) Z[k + 1][nd][i] = subm(Z[k][j][i], mulm(c, Z[k][piv][i]));
        nd++;
      }
    }
    dimZ[k + 1] = nd;
    usedv[b] = true;
    mid[e] = b;
    dfs(k + 1);
    usedv[b] = false;
    if (found && !countMode) return;
  }
}

// diagnostics -----------------------------------------------------------------------------
// density: exists S with |E(S)| >= |S| - 1 and at least one edge? max-closure via min cut,
// forcing each edge in turn.
struct Flow {
  int N; std::vector<std::vector<int>> g; std::vector<int> to; std::vector<long long> cap;
  Flow(int N) : N(N), g(N) {}
  void add(int a, int b, long long c) { g[a].push_back(to.size()); to.push_back(b); cap.push_back(c); g[b].push_back(to.size()); to.push_back(a); cap.push_back(0); }
  long long maxflow(int s, int t) {
    long long tot = 0;
    while (true) {
      std::vector<int> pe(N, -1); std::vector<int> q{s}; pe[s] = -2;
      for (size_t h = 0; h < q.size() && pe[t] == -1; h++)
        for (int id : g[q[h]]) if (cap[id] > 0 && pe[to[id]] == -1) { pe[to[id]] = id; q.push_back(to[id]); }
      if (pe[t] == -1) return tot;
      long long b = 1LL << 60;
      for (int v = t; v != s; v = to[pe[v] ^ 1]) b = std::min(b, cap[pe[v]]);
      for (int v = t; v != s; v = to[pe[v] ^ 1]) { cap[pe[v]] -= b; cap[pe[v] ^ 1] += b; }
      tot += b;
    }
  }
};
// densS: vertex mask of a dense set found (smallest over forced edges), 0 if none
static unsigned densS;
static bool densityViolation() {
  const long long BIG = 1000;
  densS = 0;
  int bestSize = 1 << 30;
  for (int fe = 0; fe < m; fe++) {
    // nodes: s=0, edges 1..m, vertices m+1..m+n, t=m+n+1. maximise |E(S)| - |S|
    Flow F(m + n + 2);
    int s = 0, t = m + n + 1;
    long long wsum = 0;
    for (int e = 0; e < m; e++) {
      long long w = (e == fe) ? BIG : 1;
      F.add(s, 1 + e, w); wsum += w;
      for (int q = 0; q < 3; q++) F.add(1 + e, m + 1 + E[e][q], 1LL << 50);
    }
    for (int v = 0; v < n; v++) F.add(m + 1 + v, t, 1);
    long long best = wsum - F.maxflow(s, t) - (BIG - 1);  // value of best closure containing fe
    if (best < -1) continue;
    // source side of the min cut = an optimal closure
    std::vector<char> vis(F.N, 0); std::vector<int> q{s}; vis[s] = 1;
    for (size_t h = 0; h < q.size(); h++)
      for (int id : F.g[q[h]]) if (F.cap[id] > 0 && !vis[F.to[id]]) { vis[F.to[id]] = 1; q.push_back(F.to[id]); }
    unsigned mask = 0;
    for (int v = 0; v < n; v++) if (vis[m + 1 + v]) mask |= 1u << v;
    int sz = __builtin_popcount(mask);
    if (sz < bestSize) { bestSize = sz; densS = mask; }
  }
  return densS != 0;
}
static bool hasFano() {
  auto ed = [&](int a, int b) { return (a < MAXN && b < MAXN && a != b) ? third[a][b] : -1; };
  for (int e1 = 0; e1 < m; e1++)
    for (int e2 = e1 + 1; e2 < m; e2++)
      for (int p : E[e1]) {
        bool in2 = false;
        for (int q : E[e2]) if (q == p) in2 = true;
        if (!in2) continue;
        int a = -1, b = -1, c = -1, d = -1;
        for (int q : E[e1]) if (q != p) { if (a < 0) a = q; else b = q; }
        for (int q : E[e2]) if (q != p) { if (c < 0) c = q; else d = q; }
        int x = ed(a, c), y = ed(a, d);
        if (x < 0 || y < 0) continue;
        if (ed(b, c) == y && ed(b, d) == x && ed(p, x) == y) return true;
      }
  return false;
}

// HY's F7 = {012,034,056,135,246} (AG(2,3) minus two points): centre on three edges, two
// disjoint edges avoiding the centre inside the other six points. The chain forbids it.
static bool hasHYF7() {
  for (int c = 0; c < n; c++) {
    std::vector<int> th;
    for (int e = 0; e < m; e++) if (E[e][0] == c || E[e][1] == c || E[e][2] == c) th.push_back(e);
    for (size_t a = 0; a < th.size(); a++) for (size_t b = a + 1; b < th.size(); b++) for (size_t d = b + 1; d < th.size(); d++) {
      unsigned Pm = 0;
      for (int e : {th[a], th[b], th[d]}) for (int q = 0; q < 3; q++) Pm |= 1u << E[e][q];
      Pm &= ~(1u << c);
      std::vector<unsigned> in;
      for (int e = 0; e < m; e++) {
        unsigned em = (1u << E[e][0]) | (1u << E[e][1]) | (1u << E[e][2]);
        if (!(em >> c & 1) && (em & ~Pm) == 0) in.push_back(em);
      }
      for (size_t x = 0; x < in.size(); x++) for (size_t y = x + 1; y < in.size(); y++) if (!(in[x] & in[y])) return true;
    }
  }
  return false;
}

int main(int argc, char **argv) {
  if (argc < 2) { fprintf(stderr, "usage: rt FINAL [--count] [--shard i/k]\n"); return 1; }
  int si = 0, sk = 1;
  for (int a = 2; a < argc; a++) {
    if (!strcmp(argv[a], "--count")) countMode = true;
    else if (!strcmp(argv[a], "--shard") && a + 1 < argc) sscanf(argv[++a], "%d/%d", &si, &sk);
  }
  std::mt19937_64 rng(757);
  for (int j = 0; j < MAXN; j++) { C1[j] = rng() % P; C2[j] = rng() % P; }
  std::ifstream in(argv[1]);
  std::string line;
  long long idx = -1;
  while (std::getline(in, line)) {
    if (line.find_first_not_of(" \t\r") == std::string::npos) continue;
    idx++;
    if (idx % sk != si) continue;
    std::istringstream ss(line);
    int tau;
    ss >> n >> m >> tau;
    if (n > MAXN || m > MAXM) { fprintf(stderr, "too big at %lld\n", idx); return 2; }
    for (int i = 0; i < MAXN; i++) for (int j = 0; j < MAXN; j++) third[i][j] = -1;
    for (int e = 0; e < m; e++) {
      std::string tok; ss >> tok;
      if (sscanf(tok.c_str(), "%d,%d,%d", &E[e][0], &E[e][1], &E[e][2]) != 3) { fprintf(stderr, "parse %lld\n", idx); return 2; }
      for (int x = 0; x < 3; x++) for (int y = 0; y < 3; y++) if (x != y) {
        if (third[E[e][x]][E[e][y]] != -1) { fprintf(stderr, "not linear %lld\n", idx); return 2; }
      }
      for (int x = 0; x < 3; x++) for (int y = 0; y < 3; y++) if (x != y) third[E[e][x]][E[e][y]] = E[e][3 - x - y];
    }
    bool dv = densityViolation(), fano = hasFano(), hyf7 = hasHYF7();
    // order: edges inside the dense set first, then greedy by vertices already covered
    bool cov[MAXN] = {}, taken[MAXM] = {};
    for (int k = 0; k < m; k++) {
      int best = -1, bs = -1;
      for (int e = 0; e < m; e++) if (!taken[e]) {
        bool inS = ((densS >> E[e][0]) & 1) && ((densS >> E[e][1]) & 1) && ((densS >> E[e][2]) & 1);
        int s = cov[E[e][0]] + cov[E[e][1]] + cov[E[e][2]] + (inS ? 100 : 0);
        if (s > bs) { bs = s; best = e; }
      }
      taken[best] = true; order_[k] = best;
      for (int q = 0; q < 3; q++) cov[E[best][q]] = true;
    }
    for (int j = 0; j < n; j++) for (int i = 0; i < n; i++) Z[0][j][i] = (i == j);
    dimZ[0] = n;
    memset(usedv, 0, sizeof usedv);
    nodes = leaves = 0; found = false;
    dfs(0);
    printf("%lld %c %lld %lld %d %d %d %d", idx, leaves ? 'R' : 'U', nodes, leaves, dv, fano, hyf7, __builtin_popcount(densS));
    if (leaves) { printf(" mu="); for (int i = 0; i < m; i++) printf("%s%d", i ? "," : "", foundMu[i]); }
    printf("\n");
    fflush(stdout);
  }
  fprintf(stderr, "exact_checks=%lld false_alarms=%lld\n", exactChecks, falseAlarms);
  return 0;
}
