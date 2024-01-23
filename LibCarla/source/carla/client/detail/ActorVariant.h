// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Debug.h"
#include "carla/Memory.h"
#include "carla/client/Actor.h"
#include "carla/client/detail/EpisodeProxy.h"
#include "carla/rpc/Actor.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4583)
#pragma warning(disable:4582)
#include <variant>
#pragma warning(pop)
#else
#include <variant>
#endif

namespace carla {
namespace client {
namespace detail {

  /// Holds an Actor, but only instantiates it when needed.
  class ActorVariant {
  public:

    ActorVariant(rpc::Actor actor)
      : _value(actor) {}

    ActorVariant(SharedPtr<client::Actor> actor)
      : _value(actor) {}

    ActorVariant &operator=(rpc::Actor actor) {
      _value = actor;
      return *this;
    }

    ActorVariant &operator=(SharedPtr<client::Actor> actor) {
      _value = actor;
      return *this;
    }

    SharedPtr<client::Actor> Get(EpisodeProxy episode) const {
      if (_value.index() == 0u) {
        MakeActor(episode);
      }
      DEBUG_ASSERT(_value.index() == 1u);
      return std::get<SharedPtr<client::Actor>>(_value);
    }

    const rpc::Actor &Serialize() const {
      return std::visit(Visitor(), _value);
    }

    ActorId GetId() const {
      return Serialize().id;
    }

    ActorId GetParentId() const {
      return Serialize().parent_id;
    }

    const std::string &GetTypeId() const {
      return Serialize().description.id;
    }

    bool operator==(ActorVariant rhs) const {
      return GetId() == rhs.GetId();
    }

    bool operator!=(ActorVariant rhs) const {
      return !(*this == rhs);
    }

  private:

    struct Visitor {
      const rpc::Actor &operator()(const rpc::Actor &actor) const {
        return actor;
      }
      const rpc::Actor &operator()(const SharedPtr<const client::Actor> &actor) const {
        return actor->Serialize();
      }
    };

    void MakeActor(EpisodeProxy episode) const;

    mutable std::variant<rpc::Actor, SharedPtr<client::Actor>> _value;
  };

} // namespace detail
} // namespace client
} // namespace carla
