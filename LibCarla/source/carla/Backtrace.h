
#include <execinfo.h>
#include "carla/Logging.h"

inline void PrintBacktrace() {
  int num_addresses = 0;
  char **strings = nullptr;
  void *buffer[1000];
  num_addresses = backtrace(buffer, 1000);
  strings = backtrace_symbols(buffer, num_addresses);
  carla::log_info("==========================================");
  carla::log_info("Backtrace:\n");
  for(int i=0; i < num_addresses; i++) {
    carla::log_info("%s", strings[i]);
  }
  carla::log_info("==========================================");
  free(strings);
}
