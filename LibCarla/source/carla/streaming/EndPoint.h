// Copyright (c) 2025 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/udp.hpp>
#include <boost/asio/ip/basic_resolver_query.hpp>

namespace carla::streaming {

namespace detail {

  // When in doubt, V4 addresses are returned.

  struct FullyDefinedEndPoint {};
  struct PartiallyDefinedEndPoint {};

  template <typename Protocol, typename EndPointType>
  class EndPoint;

  template <typename Protocol>
  class EndPoint<Protocol, FullyDefinedEndPoint>
  {
  public:

  	explicit EndPoint(boost::asio::ip::basic_endpoint<Protocol> ep)
  	  : _endpoint(std::move(ep)) {
  	}

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
  	  return { Protocol::v4(), _port };
  	}

  private:

  	uint16_t _port;
  };

} // namespace detail

static inline auto make_localhost_address() {
  return boost::asio::ip::make_address("127.0.0.1");
}

template <typename Protocol = boost::asio::ip::tcp>
static inline auto make_address(std::string_view address)
{
  using namespace boost::asio;
  io_context io_context;
  ip::basic_resolver<Protocol> resolver(io_context);
  auto resolve_results = resolver.resolve(
    ip::tcp::v4(),
    address,
    std::string_view(),
    ip::resolver_base::flags::canonical_name);
  for (auto& result : resolve_results)
  {
    auto endpoint = result.endpoint();
    return endpoint.address();
  }
  return ip::make_address(address);
}

template <typename Protocol = boost::asio::ip::tcp>
static inline auto make_endpoint(boost::asio::ip::basic_endpoint<Protocol> ep) {
  return detail::EndPoint<Protocol, detail::FullyDefinedEndPoint>{std::move(ep)};
}

template <typename Protocol = boost::asio::ip::tcp>
static inline auto make_endpoint(std::string_view address, uint16_t port) {
  return make_endpoint<Protocol>(
  {
    make_address<Protocol>(address), 
    port
  });
}

template <typename Protocol = boost::asio::ip::tcp>
static inline auto make_endpoint(uint16_t port) {
  return detail::EndPoint<Protocol, detail::PartiallyDefinedEndPoint>{port};
}

} // namespace carla::streaming
