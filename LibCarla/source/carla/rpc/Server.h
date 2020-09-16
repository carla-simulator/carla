// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/MoveHandler.h"
#include "carla/Time.h"
#include "carla/rpc/Metadata.h"
#include "carla/rpc/Response.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/post.hpp>

#include <rpc/server.h>

#include <future>

namespace carla {
namespace rpc {

  // ===========================================================================
  // -- Server -----------------------------------------------------------------
  // ===========================================================================

  /// An RPC server in which functions can be bind to run synchronously or
  /// asynchronously.
  ///
  /// Use `AsyncRun` to start the worker threads, and use `SyncRunFor` to
  /// run a slice of work in the caller's thread.
  ///
  /// Functions that are bind using `BindAsync` will run asynchronously in the
  /// worker threads. Functions that are bind using `BindSync` will run within
  /// `SyncRunFor` function.
  class Server {
  public:

    template <typename... Args>
    explicit Server(Args &&... args);

    template <typename FunctorT>
    void BindSync(const std::string &name, FunctorT &&functor);

    template <typename FunctorT>
    void BindAsync(const std::string &name, FunctorT &&functor);

    void AsyncRun(size_t worker_threads) {
      _server.async_run(worker_threads);
    }

    void SyncRunFor(time_duration duration) {
      _sync_io_context.reset();
      _sync_io_context.run_for(duration.to_chrono());
    }

    /// @warning does not stop the game thread.
    void Stop() {
      _server.stop();
    }

  private:

    boost::asio::io_context _sync_io_context;

    ::rpc::server _server;
  };

  // ===========================================================================
  // -- Server implementation --------------------------------------------------
  // ===========================================================================

namespace detail {

  template <typename T>
  struct FunctionWrapper : FunctionWrapper<decltype(&T::operator())> {};

  template <typename C, typename R, typename... Args>
  struct FunctionWrapper<R (C::*)(Args...)> : FunctionWrapper<R (*)(Args...)> {};

  template <typename C, typename R, typename... Args>
  struct FunctionWrapper<R (C::*)(Args...) const> : FunctionWrapper<R (*)(Args...)> {};

  template<class T>
  struct FunctionWrapper<T &> : public FunctionWrapper<T> {};

  template<class T>
  struct FunctionWrapper<T &&> : public FunctionWrapper<T> {};

  template <typename R, typename... Args>
  struct FunctionWrapper<R (*)(Args...)> {

    /// Wraps @a functor into a function type with equivalent signature. The
    /// wrap function returned. When called, posts @a functor into the
    /// io_context; if the client called this method synchronously, waits for
    /// the posted task to finish, otherwise returns immediately.
    ///
    /// This way, no matter from which thread the wrap function is called, the
    /// @a functor provided is always called from the context of the io_context.
    /// I.e., we can use the io_context to run tasks on a specific thread (e.g.
    /// game thread).
    template <typename FuncT>
    static auto WrapSyncCall(boost::asio::io_context &io, FuncT &&functor) {
      return [&io, functor=std::forward<FuncT>(functor)](Metadata metadata, Args... args) -> R {
        auto task = std::packaged_task<R()>([functor=std::move(functor), args...]() {
          return functor(args...);
        });
        if (metadata.IsResponseIgnored()) {
          // Post task and ignore result.
          boost::asio::post(io, MoveHandler(task));
          return R();
        } else {
          // Post task and wait for result.
          auto result = task.get_future();
          boost::asio::post(io, MoveHandler(task));
          return result.get();
        }
      };
    }

    /// Wraps @a functor into a function type with equivalent signature that
    /// handles the metadata sent by the client. If the client called this
    /// method asynchronously, the result is ignored.
    template <typename FuncT>
    static auto WrapAsyncCall(FuncT &&functor) {
      return [functor=std::forward<FuncT>(functor)](::carla::rpc::Metadata metadata, Args... args) -> R {
        if (metadata.IsResponseIgnored()) {
          functor(args...);
          return R();
        } else {
          return functor(args...);
        }
      };
    }
  };

} // namespace detail

  template <typename ... Args>
  inline Server::Server(Args && ... args)
    : _server(std::forward<Args>(args) ...) {
    _server.suppress_exceptions(true);
  }

  template <typename FunctorT>
  inline void Server::BindSync(const std::string &name, FunctorT &&functor) {
    using Wrapper = detail::FunctionWrapper<FunctorT>;
    _server.bind(
        name,
        Wrapper::WrapSyncCall(_sync_io_context, std::forward<FunctorT>(functor)));
  }

  template <typename FunctorT>
  inline void Server::BindAsync(const std::string &name, FunctorT &&functor) {
    using Wrapper = detail::FunctionWrapper<FunctorT>;
    _server.bind(
        name,
        Wrapper::WrapAsyncCall(std::forward<FunctorT>(functor)));
  }

} // namespace rpc
} // namespace carla
