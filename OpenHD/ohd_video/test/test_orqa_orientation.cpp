#include "orqa_orientation.h"

#include <iostream>
#include <utility>

using Point = std::pair<int, int>;

Point rotate(Point point, int degrees) {
  switch (degrees) {
    case 90: return {-point.second, point.first};
    case 180: return {-point.first, -point.second};
    case 270: return {point.second, -point.first};
    default: return point;
  }
}

int main() {
  for (int rotation : {0, 90, 180, 270}) {
    for (int flip : {0, 1, 2, 3}) {
      const auto orientation = orqa_orientation(rotation, flip);
      for (Point corner : {Point{-1, -1}, {-1, 1}, {1, -1}, {1, 1}}) {
        auto expected = corner;
        if (flip & 1) expected.first = -expected.first;
        if (flip & 2) expected.second = -expected.second;
        expected = rotate(expected, rotation);
        auto actual = corner;
        if (orientation.mirror) actual.first = -actual.first;
        actual = rotate(actual, orientation.rotation);
        if (actual != expected || orientation.rotation % 180 != rotation % 180) {
          std::cerr << "Incorrect orientation: " << rotation << '/' << flip << '\n';
          return 1;
        }
      }
    }
  }
  std::cout << "All 16 ORQA rotation/flip combinations passed\n";
}
