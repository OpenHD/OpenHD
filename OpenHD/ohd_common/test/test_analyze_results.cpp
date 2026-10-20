#include "openhd_action_handler.h"
#include <stdexcept>
#include <cstdio>

static void require(bool condition) {
  if (!condition) throw std::runtime_error("Analyze result queue regression");
}

int main() {
  openhd::LinkActionHandler handler;
  openhd::LinkActionHandler::AnalyzeChannelsResult page{};
  page.devourer_survey = true;
  page.rf_batch_id = 1;
  page.rf_total_channels = 60;
  page.channels_mhz.fill(5745);
  // Fast cumulative updates must not build a telemetry backlog.
  for (int progress = 1; progress <= 98; ++progress) {
    page.progress = progress;
    handler.add_analyze_result(page);
  }
  auto results = handler.get_analyze_results();
  require(results.size() == 1 && results[0].progress == 98);
  require(handler.get_analyze_results().empty());
  // Lose the first completed snapshot; the following tick repairs both pages.
  page.progress = 100;
  handler.add_analyze_result(page);
  // Drain between final pages, as a telemetry thread can do concurrently.
  require(handler.get_analyze_results().size() == 1);
  page.rf_page_offset = 30;
  handler.add_analyze_result(page);
  require(handler.get_analyze_results().size() == 1);
  results = handler.get_analyze_results();
  require(results.size() == 2 && results[0].progress == 100);
  require(results[1].rf_page_offset == 30);
  require(handler.get_analyze_results().size() == 2);
  require(handler.get_analyze_results().empty());
  // New measurements cancel completion replay from the preceding run.
  handler.add_analyze_result(page);
  handler.get_analyze_results();
  page.rf_batch_id = 2;
  page.progress = 10;
  handler.add_analyze_result(page);
  results = handler.get_analyze_results();
  require(results.size() == 1 && results[0].rf_batch_id == 2);
  require(handler.get_analyze_results().empty());
  handler.add_analyze_result(page);
  handler.reset_analyze_results();
  require(handler.get_analyze_results().empty());
  std::puts("Analyze result queue passed");
}
