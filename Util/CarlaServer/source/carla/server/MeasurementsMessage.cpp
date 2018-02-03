
#include "carla/server/MeasurementsMessage.h"

#include <cstring>

#include "carla/Debug.h"
#include "carla/Logging.h"

namespace carla {
namespace server {

void MeasurementsMessage::Reset(const uint32_t count) {
  if (_capacity < count) {
    log_info("allocating image buffer of", count, "bytes");
    _buffer = std::make_unique<unsigned char[]>(count);
    _capacity = count;
  }
  _size = count;
}

}
}
