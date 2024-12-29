// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "carla/Exception.h"
#include <boost/assert/source_location.hpp>

// =============================================================================
// -- Define boost::throw_exception --------------------------------------------
// =============================================================================

#ifdef BOOST_NO_EXCEPTIONS

namespace boost {

  void throw_exception(const std::exception &e) {
    carla::throw_exception(e);
  }

  void throw_exception(
      const std::exception &e,
      boost::source_location const & loc) {
// =============================================================================
// -- 异常处理兼容性调整：Boost 和 Boost.Asio 的支持 ---------------------------
// =============================================================================

// 如果 Boost 被配置为禁用异常（BOOST_NO_EXCEPTIONS），
// 提供一个自定义的异常抛出实现。
#ifdef BOOST_NO_EXCEPTIONS

namespace boost {

  // 自定义异常抛出函数。
  // 需要开发者提供具体的异常抛出逻辑。
  void throw_exception(const std::exception& e) {
    // 在用户代码中实现具体逻辑。
    throw_exception(e);
  }

} // namespace boost

#endif // BOOST_NO_EXCEPTIONS

// =============================================================================
// -- 针对 rpclib 使用的 Boost.Asio 的兼容性调整 -------------------------------
// =============================================================================

// 如果 Asio 被配置为禁用异常（ASIO_NO_EXCEPTIONS），
// 定义兼容性层以处理 Asio 中的异常抛出。
#ifdef ASIO_NO_EXCEPTIONS

#include <exception>
#include <system_error>
#include <typeinfo>

namespace clmdep_asio {
namespace detail {

  // 模板函数，用于抛出指定类型的异常。
  template <typename Exception>
  void throw_exception(const Exception& e) {
    carla::throw_exception(e); // 调用 Carla 库提供的异常抛出实现。
  }

  // 针对常见异常类型的显式模板实例化。
  template void throw_exception<std::bad_cast>(const std::bad_cast &);
  template void throw_exception<std::exception>(const std::exception &);
  template void throw_exception<std::system_error>(const std::system_error &);

} // namespace detail
} // namespace clmdep_asio

#endif // ASIO_NO_EXCEPTIONS
