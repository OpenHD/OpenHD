#include <iostream>
#include <sstream>
#include <stdexcept>

#include "internal/cpu_usage.hpp"

void require(bool value) {
  if (!value) throw std::runtime_error("CPU accounting regression");
}

int main() {
  using namespace openhd::onboard;
  std::istringstream first("cpu 100 20 30 800 40 5 5 0 50 10\ncpu0 1 2 3 4\n");
  auto initial = parse_cpu_times(first);
  require(initial && initial->total == 1000 && initial->idle == 840);
  require(cpu_usage_between(*initial, {1400, 1140}) == 25);
  require(cpu_usage_between({100, 80}, {200, 80}) == 100);
  require(cpu_usage_between({100, 80}, {200, 180}) == 0);
  require(!cpu_usage_between({100, 80}, {100, 80}));
  require(!cpu_usage_between({100, 80}, {50, 40}));
  require(!cpu_usage_between({100, 80}, {200, 70}));
  require(!cpu_usage_between({100, 80}, {110, 100}));
  std::istringstream truncated("cpu 1 2 3 4");
  require(!parse_cpu_times(truncated));
  std::istringstream wrong("cpu0 1 2 3 4 5 6 7 8");
  require(!parse_cpu_times(wrong));
  std::cout << "CPU accounting checks passed\n";
}
