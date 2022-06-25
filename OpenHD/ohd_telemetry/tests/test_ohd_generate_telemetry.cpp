//
// Test the OHD fire and forget telemetry generator - this just prints the generated data to stdout.
//
#include "../src/internal/InternalTelemetry.h"
#include "../src/internal/LogCustomOHDMessages.hpp"
#include <iostream>
#include <thread>

int main() {
  std::cout << "OHDTelemetryGeneratorTest::start" << std::endl;

  InternalTelemetry ohdTelemetryGenerator(false);
  const auto start = std::chrono::steady_clock::now();
  while ((std::chrono::steady_clock::now() - start) < std::chrono::minutes(5)) {
	const auto msges = ohdTelemetryGenerator.generateUpdates();
	LogCustomOHDMessages::logOpenHDMessages(msges);
	std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  std::cout << "OHDTelemetryGeneratorTest::end" << std::endl;
  return 0;
}
