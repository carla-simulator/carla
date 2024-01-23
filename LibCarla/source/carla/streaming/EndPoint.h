// Copyright (c) 2024 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>

namespace carla {
namespace streaming {
namespace detail {

  // When in doubt, V4 addresses are returned.

  struct FullyDefinedEndPoint {};
  struct PartiallyDefinedEndPoint {};

  template <typename Protocol, typename EndPointType>
  class EndPoint;

  template <typename Protocol>
  class EndPoint<Protocol, FullyDefinedEndPoint> {
  public:

    explicit EndPoint(boost::asio::ip::basic_endpoint<Protocol> ep)
      : _endpoint(std::move(ep)) {}

    auto address() const {
      return _endpoint.address();
    }

    uint16_t port() const {
      return _endpoint.port();
    }

    operator boost::asio::ip::basic_endpoint<Protocol>() const {
      return _endpoint;
    }

  private:

    boost::asio::ip::basic_endpoint<Protocol> _endpoint;
  };

  template <typename Protocol>
  class EndPoint<Protocol, PartiallyDefinedEndPoint> {
  public:

    explicit EndPoint(uint16_t port) : _port(port) {}

    uint16_t port() const {
      return _port;
    }

    operator boost::asio::ip::basic_endpoint<Protocol>() const {
      return {Protocol::v4(), _port};
    }

  private:

    uint16_t _port;
  };

} // namespace detail

  static inline auto make_localhost_address() {
    return boost::asio::ip::make_address("127.0.0.1");
  }

  static inline auto make_address(const std::string &address) {
    boost::asio::io_context io_context;
    boost::asio::ip::tcp::resolver resolver(io_context);
    boost::asio::ip::tcp::resolver::query query(boost::asio::ip::tcp::v4(), address, "", boost::asio::ip::tcp::resolver::query::canonical_name);
    boost::asio::ip::tcp::resolver::iterator iter = resolver.resolve(query);
    boost::asio::ip::tcp::resolver::iterator end;
    while (iter != end)
    {
      boost::asio::ip::tcp::endpoint endpoint = *iter++;
      return endpoint.address();
    }
    return boost::asio::ip::make_address(address);
  }

  template <typename Protocol>
  static inline auto make_endpoint(boost::asio::ip::basic_endpoint<Protocol> ep) {
    return detail::EndPoint<Protocol, detail::FullyDefinedEndPoint>{std::move(ep)};
  }

  template <typename Protocol>
  static inline auto make_endpoint(const char *address, uint16_t port) {
    return make_endpoint<Protocol>({make_address(address), port});
  }

  template <typename Protocol>
  static inline auto make_endpoint(const std::string &address, uint16_t port) {
    return make_endpoint<Protocol>(address.c_str(), port);
  }

  template <typename Protocol>
  static inline auto make_endpoint(uint16_t port) {
    return detail::EndPoint<Protocol, detail::PartiallyDefinedEndPoint>{port};
  }

} // namespace streaming
} // namespace carla
