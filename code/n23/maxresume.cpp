// Resume the fixed max-degree chain from a stored level file written by the ORIGINAL
// maxchain (76-byte records: k, m, pad, e[18]). Runs exactly one level K0 -> K0+1.
// usage: maxresume N MMIN MMAX TAU DCAP OUTDIR K0 levelK0.bin P

#include "maxchain_fix_lib.cpp"

struct OldG { uint8_t k, m; uint32_t e[18]; };
int main(int argc, char **argv) {
  N = atoi(argv[1]); MMIN = atoi(argv[2]); MMAX = atoi(argv[3]); TAU = atoi(argv[4]);
  DCAP = atoi(argv[5]); std::string dir = argv[6]; int K0 = atoi(argv[7]);
  int P = atoi(argv[9]);
  nauty_check(WORDSIZE, 1, MAXN, NAUTYVERSIONID);
  build_windows();
  std::vector<G> cur;
  FILE *f = fopen(argv[8], "rb"); OldG o;
  static_assert(sizeof(OldG) == 76, "old record size");
  while (fread(&o, sizeof o, 1, f) == 1) { G g; g.k = o.k; g.m = o.m; memcpy(g.e, o.e, sizeof o.e); cur.push_back(g); }
  fclose(f);
  fprintf(stderr, "loaded %zu classes at k=%d (sizeof G now %zu)\n", cur.size(), K0, sizeof(G));
  int k = K0 + 1;
  std::vector<G> nxt;
  auto t0 = std::chrono::steady_clock::now();
  std::vector<pid_t> kids;
  for (int w = 0; w < P; w++) {
    pid_t pid = fork();
    if (pid == 0) { std::vector<G> oo; for (size_t i = w; i < cur.size(); i += P) extend_max(cur[i], oo); dedup(oo);
      write_level(dir + "/shard" + std::to_string(w) + ".bin", oo); _exit(0); }
    kids.push_back(pid);
  }
  for (pid_t p : kids) waitpid(p, nullptr, 0);
  for (int w = 0; w < P; w++) { std::string s = dir + "/shard" + std::to_string(w) + ".bin"; auto oo = read_level(s); nxt.insert(nxt.end(), oo.begin(), oo.end()); unlink(s.c_str()); }
  size_t raw = nxt.size(); dedup(nxt);
  std::map<std::pair<int,int>,long> by; long hy_bad = 0;
  for (auto &g : nxt) { int t = tau_of(g); by[{g.m,t}]++; if (17*t > 5*g.k + 3*g.m) hy_bad++; }
  double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
  printf("level k=%d: %zu classes (raw %zu) tau>=%d HY-violations=%ld t=%.1fs |", k, nxt.size(), raw, lo_tau(k), hy_bad, sec);
  for (auto &[key,c] : by) printf(" m%d/t%d:%ld", key.first, key.second, c);
  printf("\n");
  FILE *ff = fopen((dir + "/final.txt").c_str(), "w");
  for (auto &g : nxt) { fprintf(ff, "%d %d %d", g.k, g.m, tau_of(g));
    for (int i = 0; i < g.m; i++) { fprintf(ff, " "); int c = 0; for (int v = 0; v < g.k; v++) if (g.e[i] >> v & 1) fprintf(ff, c++ ? ",%d" : "%d", v); }
    fprintf(ff, "\n"); }
  fclose(ff);
  return 0;
}
