// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <list>
#include <mutex>

namespace carla {
namespace ros2 {

template <typename MESSAGE_TYPE>
class SubscriberBase;

/**
 * Base class for subscriber implementations
 */
template <typename MESSAGE_TYPE>
class SubscriberImplBase {
public:
  /**
   * Default constructor.
   */
  SubscriberImplBase(SubscriberBase<MESSAGE_TYPE> &parent) : _parent(parent) {}
  /**
   * Copy operation not allowed due to active subscriptions
   */
  SubscriberImplBase(const SubscriberImplBase &) = delete;
  /**
   * Assignment operation not allowed due to active subscriptions
   */
  SubscriberImplBase &operator=(const SubscriberImplBase &) = delete;
  /**
   * Move operation not allowed due to active subscriptions
   */
  SubscriberImplBase(SubscriberImplBase &&) = delete;
  /**
   * Move operation not allowed due to active subscriptions
   */
  SubscriberImplBase &operator=(SubscriberImplBase &&) = delete;

  /**
   * Default destructor.
   */
  virtual ~SubscriberImplBase() = default;

  struct MessageEntry {
    // a process local unique identification of the publisher that has sent the message
    std::string  publisher{};
    // the actual message
    MESSAGE_TYPE message{};
  };

  /**
   * Get the list of currently alive publishers in the order of their appearance.
   */
  std::list<std::string> GetConnectedPublishers() const {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    return _connected_publishers;
  }

  /**
   * Check if there are publishers connected to this
   */
  bool HasPublishersConnected() const {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    return !_connected_publishers.empty();
  }

  /**
   * Report how many publishers are connected to this
   */
  std::size_t NumberPublishersConnected() const {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    return _connected_publishers.size();
  }

  /**
   * Check if there is a new message available
   */
  bool HasNewMessage() const {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    return !_messages.empty();
  }

  /**
   * Get the list of the current available message entry.
   */
  std::list<MessageEntry> GetMessageEntries() {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    std::list<MessageEntry> messages;
    messages.swap(_messages);
    return messages;
  }

  /**
   * Implements SubscriberImplBase::GetMessageEntry() interface
   */
  MessageEntry GetMessageEntry() {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    if (_messages.empty()) {
      return MessageEntry();
    }
    auto message = _messages.front();
    _messages.pop_front();
    return message;
  }

  /**
   * Get the next message. This is a conventient function for subscribers that don't care on the identification of the
   * sender.
   */
  const MESSAGE_TYPE GetMessage() {
    return GetMessageEntry().message;
  }

protected:
  void AddMessage(std::string const &publisher_guid, MESSAGE_TYPE &message) {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    _messages.push_back({publisher_guid, message});
    carla::log_debug("SubscriberImplBase[", _parent.get_topic_name(), "]::AddMessage(", publisher_guid,
                      ") number of messages: ", _messages.size());
  }

  void AddPublisher(std::string const &publisher_guid) {
    {
      std::lock_guard<std::mutex> access_lock(_access_mutex);
      _connected_publishers.push_back(publisher_guid);
      carla::log_debug("SubscriberImplBase[", _parent.get_topic_name(), "]::AddPublisher(", publisher_guid,
                       ") number of connected publisher: ", _connected_publishers.size());
    }
    _parent.PublisherConnected(publisher_guid);
  }

  void RemovePublisher(std::string const &publisher_guid) {
    _parent.PublisherDisconnected(publisher_guid);
    {
      std::lock_guard<std::mutex> access_lock(_access_mutex);
      _connected_publishers.remove_if([publisher_guid](std::string const &element) -> bool {
        return publisher_guid == element;
      });
      carla::log_debug("SubscriberImplBase[", _parent.get_topic_name(), "]::RemovePublisher(", publisher_guid,
                       ") number of connected publisher: ", _connected_publishers.size());
    }
  }

  void Clear() {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    for (auto const &publisher_guid: _connected_publishers) {
      _parent.PublisherDisconnected(publisher_guid);
    }
    _connected_publishers.clear();
    _messages.clear();
  }

private:
  // keep the data private to ensure access_mutex is hold while accessing
  mutable std::mutex _access_mutex{};
  SubscriberBase<MESSAGE_TYPE> &_parent;
  std::list<std::string > _connected_publishers;
  std::list<MessageEntry> _messages;
};
}  // namespace ros2
}  // namespace carla
