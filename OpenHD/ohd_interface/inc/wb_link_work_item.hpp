#include <utility>

//
// Created by consti10 on 15.02.23.
//

#ifndef OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_WORK_ITEM_H_
#define OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_WORK_ITEM_H_

// I took this pattern from MAVSDK.
// A work item refers to some task that is queued up for a worker thread to
// handle
class WorkItem {
 public:
  /**
   * @param work lambda - the work to perform
   * @param earliest_execution_time earliest time point this work item should be
   * handled.
   */
  explicit WorkItem(
      std::string tag, std::function<void()> work,
      std::chrono::steady_clock::time_point earliest_execution_time)
      : TAG(std::move(tag)),
        m_earliest_execution_time(earliest_execution_time),
        m_work(std::move(work)) {}
  void execute() { m_work(); }
  bool ready_to_be_executed() {
    return std::chrono::steady_clock::now() >= m_earliest_execution_time;
  }
  const std::string TAG;

 private:
  const std::chrono::steady_clock::time_point m_earliest_execution_time;
  const std::function<void()> m_work;
};

#endif  // OPENHD_OPENHD_OHD_INTERFACE_INC_WB_LINK_WORK_ITEM_H_
