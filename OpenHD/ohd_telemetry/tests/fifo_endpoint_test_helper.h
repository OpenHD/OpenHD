//
// Created by rsaxvc on 14.08.23.
//

#ifndef OPENHD_OPENHD_OHD_TELEMETRY_TESTS_FIFO_ENDPOINT_TEST_HELPER_H_
#define OPENHD_OPENHD_OHD_TELEMETRY_TESTS_FIFO_ENDPOINT_TEST_HELPER_H_

#include <getopt.h>
#include <sstream>

namespace fifo_endpoint_test_helper{

static const char optstr[] = "?f:";
static const struct option long_options[] = {
    {"fifo", required_argument, nullptr, 'f'},
    {nullptr, 0, nullptr, 0},
};

struct FifoOptions{
  std::string filename="ohdFifoTest";
};

static FifoOptions parse_args(int argc, char *argv[]) {
  std::string fifo_filename="ohdFifoTest";
  int c;
  while ((c = getopt_long(argc, argv, optstr, long_options, NULL)) != -1) {
    const char *tmp_optarg = optarg;
    switch (c) {
      case 's':
        fifo_filename=std::string(tmp_optarg);
        break;
      case '?':
      default:
        std::cout << "Usage: \n" <<
            "--fifo -f [fifo filename, default: ohdFifoTest]\n";
        break;
    }
  }
  return FifoOptions{fifo_filename};
}

static std::string options_to_string(const FifoOptions& option){
  std::stringstream ss;
  ss<<"["<<option.filename<<"]\n";
  return ss.str();
}

}


#endif  // OPENHD_OPENHD_OHD_TELEMETRY_TESTS_FIFO_ENDPOINT_TEST_HELPER_H_
