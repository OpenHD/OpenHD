#include <cstdio>
#include <vector>

// These must be initialized before main. An executable can link and run with
// every constructor skipped when newer startup objects meet an older libc.
static int constructed = 0;
struct Startup {
  Startup() { constructed = 42; }
};
static Startup startup;
static const std::vector<int> rates{2, 12, 128};
static const std::vector<int> sections[]{rates};

int main() {
  if (constructed != 42 || rates.size() != 3 || sections[0].size() != 3 ||
      sections[0][2] != 128) {
    std::fprintf(stderr, "FAIL: executable constructors did not initialize state\n");
    return 1;
  }
  std::puts("PASS: executable constructors and copied rate tables initialized");
  return 0;
}
