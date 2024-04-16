// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <list>
#include <mutex>
#include "carla/rpc/ServerSynchronizationTypes.h"

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
    carla::rpc::synchronization_client_id_type publisher{nullptr};
    // the actual message
    MESSAGE_TYPE message{};
  };

  /**
   * Get the list of currently alive publishers in the order of their appearance.
   */
  std::list<carla::rpc::synchronization_client_id_type> GetConnectedPublishers() const {
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
  void AddMessage(carla::rpc::synchronization_client_id_type publisher, MESSAGE_TYPE &message) {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    _messages.push_back({publisher, message});
  }

  void AddPublisher(carla::rpc::synchronization_client_id_type publisher) {
    {
      std::lock_guard<std::mutex> access_lock(_access_mutex);
      _connected_publishers.push_back(publisher);
    }
    _parent.PublisherConnected(publisher);
  }

  void RemovePublisher(carla::rpc::synchronization_client_id_type publisher) {
    _parent.PublisherDisconnected(publisher);
    {
      std::lock_guard<std::mutex> access_lock(_access_mutex);
      _connected_publishers.remove_if([publisher](carla::rpc::synchronization_client_id_type const &element) -> bool {
        return publisher == element;
      });
    }
  }

  void Clear() {
    std::lock_guard<std::mutex> access_lock(_access_mutex);
    _connected_publishers.clear();
    _messages.clear();
  }

private:
  // keep the data private to ensure access_mutex is hold while accessing
  mutable std::mutex _access_mutex{};
  SubscriberBase<MESSAGE_TYPE> &_parent;
  std::list<carla::rpc::synchronization_client_id_type> _connected_publishers;
  std::list<MessageEntry> _messages;
};
}  // namespace ros2
}  // namespace carla
