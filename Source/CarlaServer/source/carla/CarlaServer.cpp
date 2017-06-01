#include "CarlaServer.h"

#include <cstring>
#include <limits>
#include <type_traits>
#include <utility>

#include <iostream>

#include <carla/Logging.h>
#include <carla/server/CarlaCommunication.h>

namespace carla {

  // ===========================================================================
  // -- Image ------------------------------------------------------------------
  // ===========================================================================

  Image::~Image() {
    delete[] _image;
  }

  void Image::make(const ImageType type, const uint32_t width, const uint32_t height) {
    delete[] _image;
    _image = new Color[width * height];
    _type = type;
    _width = width;
    _height = height;
  }

  // ===========================================================================
  // -- Collection -------------------------------------------------------------
  // ===========================================================================

  template <typename T>
  Collection<T>::Collection(const uint32_t count) :
    _size(count) {
    if (_size > 0u) {
      _array = new T[_size];
    }
  }

  template <typename T>
  Collection<T>::~Collection() {
    delete[] _array;
  }

  template <typename T>
  void Collection<T>::clear() {
    _size = 0u;
    delete[] _array;
  }

  template <typename T>
  void Collection<T>::clearAndResize(const uint32_t count) {
    clear();
    if (count > 0u) {
      _array = new T[count];
      _size = count;
    }
  }

  template <>
  void Collection<char>::clearAndResize(const uint32_t count) {
    clear();
    if (count > 0u) {
      _array = new char[count + 1u];
      _size = count;
      _array[_size] = '\0';
    }
  }

  template <typename T>
  void Collection<T>::copyContents(const T *other, const uint32_t size) {
    clearAndResize(size);
    if (size > 0u) {
      if (std::is_trivially_copyable<T>::value) {
        std::memcpy(_array, other, size);
      } else {
        for (auto i = 0u; i < size; ++i) {
          (*this)[i] = other[i];
        }
      }
    }
  }

  template <typename T>
  void Collection<T>::takeOver(T *other, const uint32_t size) {
    clear();
    _array = other;
    _size = size;
  }

  template class Collection<Image>;
  template class Collection<Vector2D>;
  template class Collection<char>;

  // ===========================================================================
  // -- CarlaServer::Pimpl -----------------------------------------------------
  // ===========================================================================

  class CarlaServer::Pimpl {
  public:

    template<typename... Args>
    Pimpl(Args&&... args) : communication(std::forward<Args>(args)...) {}

    carla::server::CarlaCommunication communication;
  };

  // ===========================================================================
  // -- CarlaServer ------------------------------------------------------------
  // ===========================================================================

  CarlaServer::CarlaServer(uint32_t writePort, uint32_t readPort, uint32_t worldPort) :
    _pimpl(new Pimpl(worldPort, writePort, readPort)) {}

  CarlaServer::~CarlaServer() {
    delete _pimpl;
  }

  bool CarlaServer::tryReadEpisodeStart(uint32_t &startIndex, uint32_t &endIndex, bool &readed) {
    if (!worldConnected())
      return false;
    readed = _pimpl->communication.tryReadEpisodeStart(startIndex, endIndex);
    return true;
  }

  bool CarlaServer::tryReadControl(Control_Values &control, bool &readed) {
    if (!clientConnected())
      return false;
    readed = _pimpl->communication.tryReadControl(control);
    return true;
  }

  bool CarlaServer::newEpisodeRequested(CarlaString &init_file, bool &readed) {
    if (!worldConnected())
      return false;

    std::string str;
    readed = _pimpl->communication.tryReadRequestNewEpisode(str);

    if (str.size() < std::numeric_limits<uint32_t>::max()) {
      init_file.copyContents(str.data(), static_cast<uint32_t>(str.size()));
    } else {
      log_error("Received string is too big!");
    }

    return true;
  }

  bool CarlaServer::sendReward(Reward_Values *values) {
    std::unique_ptr<Reward_Values> ptr(values);
    if (!serverConnected())
      return false;
    _pimpl->communication.sendReward(std::move(ptr));
    return true;
  }

  bool CarlaServer::sendSceneValues(const Scene_Values &values) {
    if (!worldConnected())
      return false;
    _pimpl->communication.sendScene(values);
    return true;
  }

  bool CarlaServer::sendEndReset() {
    if (!worldConnected())
      return false;
    _pimpl->communication.sendReset();
    return true;
  }

  bool CarlaServer::worldConnected(){
    return _pimpl->communication.worldConnected() && !_pimpl->communication.needsRestart();
  }

  bool CarlaServer::clientConnected(){
    return _pimpl->communication.clientConnected() && !_pimpl->communication.needsRestart();
  }

  bool CarlaServer::serverConnected(){
    return _pimpl->communication.serverConnected() && !_pimpl->communication.needsRestart();
  }

  bool CarlaServer::needsRestart(){
    return _pimpl->communication.needsRestart();
  }

} // namespace carla
