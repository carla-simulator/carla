// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once
#include <utility>
#include <boost/asio/connect.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/use_future.hpp>

namespace carla::streaming::detail
{
  template <
    typename SocketType,
    typename StrandType,
    typename BufferType,
    typename F>
  auto AsyncReadWrapper(
    bool use_future,
    StrandType&& strand,
    SocketType&& socket,
    BufferType&& buffer,
    F&& fn)
  {
    auto fnx = boost::asio::bind_executor(
      std::forward<StrandType>(strand),
      std::forward<F>(fn));
    
    if (use_future)
    {
      auto future = boost::asio::async_read(
          socket,
          buffer,
          boost::asio::use_future(std::move(fnx)));
      future.get();
    }
    else
    {
      boost::asio::async_read(
          socket,
          buffer,
          std::move(fnx));
    }
  }

  template <
    typename StrandType,
    typename F>
  auto PostWrapper(
    bool use_future,
    StrandType&& strand,
    F&& fn)
  {
    if (use_future)
    {
      std::promise<void> p;
      auto future = p.get_future();
      auto fnx = [p = std::move(p), fn = std::move(fn)]() mutable
      {
        fn();
        p.set_value();
      };
      boost::asio::post(strand, std::move(fnx));
      future.get();
    }
    else
    {
      boost::asio::post(strand, std::move(fn));
    }
  }
}
