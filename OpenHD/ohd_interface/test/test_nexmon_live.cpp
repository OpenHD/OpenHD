// Hardware smoke test: exercises the same radio lease and capture as Analyse.
// Run on the Pi with sudo, after installing its matching Nexmon bundle.
#include "nexmon_scout.h"
#include "chanmig/ChannelScore.h"
#include "chanmig/ScanPlan.h"
#include <chrono>
#include <iostream>

int main() {
  using namespace devourer::chanmig;
  try {
    openhd::NexmonScout scout;
    ScanPlanConfig plan;
    for (int channel : {1,36,149}) {
      ChannelDef def;
      def.band = channel == 1 ? 2 : 5;
      def.primary = channel;
      plan.candidates.push_back(def);
    }
    RecommendEngine engine(PolicyConfig{}, plan.candidates, plan.plan_hash(), {});
    engine.note_scout_health(false);
    auto now = [] { return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count(); };
    for (int round=0; round<3; ++round) {
      for (const auto& def : plan.candidates) {
        if (!scout.tune(def.center_mhz())) return 2;
        SurveyDwell dwell;
        dwell.def = def; dwell.plan_hash = plan.plan_hash();
        dwell.scout_id = 0x4e45584d; dwell.round = round;
        dwell.t_start_ms = now();
        const auto sample = openhd::observe_nexmon(def.center_mhz(),1100);
        dwell.t_end_ms = now(); dwell.observe_ms = dwell.t_end_ms-dwell.t_start_ms;
        dwell.frames = sample.foreign_packets; dwell.oth_air_us = sample.decoded_airtime_us;
        dwell.flags = kFlagNhmMissing;
        if (sample.unknown_rate_packets || sample.malformed_packets) dwell.flags |= kFlagReadFailed;
        engine.ingest_dwell(dwell,now());
        std::cout << def.str() << " packets=" << sample.foreign_packets
                  << " airtime_us=" << sample.decoded_airtime_us
                  << " unknown_rates=" << sample.unknown_rate_packets << std::endl;
      }
    }
    if (!scout.restore()) return 3;
    const auto decision=engine.decide(now());
    for (const auto& score : decision.ranking)
      std::cout << score.def.str() << " occupancy=" << score.occ_q50
                << " observed_ms=" << score.observe_ms << " rounds=" << score.rounds << std::endl;
    std::cout << "Decision: " << reason_name(decision.primary_reason) << std::endl;
  } catch (const std::exception& error) {
    std::cerr << error.what() << std::endl;
    return 1;
  }
}
