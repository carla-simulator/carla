// Copyright (c) 2022 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <string>

#include <fastrtps/types/TypesBase.h>

namespace std {

inline std::string to_string(eprosima::fastrtps::types::ReturnCode_t rcode) {
  switch (rcode()) {
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OK:
      return "RETCODE_OK";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_ERROR:
      return "RETCODE_ERROR";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_UNSUPPORTED:
      return "RETCODE_UNSUPPORTED";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_BAD_PARAMETER:
      return "RETCODE_BAD_PARAMETER";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_PRECONDITION_NOT_MET:
      return "RETCODE_PRECONDITION_NOT_MET";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_OUT_OF_RESOURCES:
      return "RETCODE_OUT_OF_RESOURCES";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_NOT_ENABLED:
      return "RETCODE_NOT_ENABLED";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_IMMUTABLE_POLICY:
      return "RETCODE_IMMUTABLE_POLICY";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_INCONSISTENT_POLICY:
      return "RETCODE_INCONSISTENT_POLICY";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_ALREADY_DELETED:
      return "RETCODE_ALREADY_DELETED";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_TIMEOUT:
      return "RETCODE_TIMEOUT";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_NO_DATA:
      return "RETCODE_NO_DATA";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_ILLEGAL_OPERATION:
      return "RETCODE_ILLEGAL_OPERATION";
    case eprosima::fastrtps::types::ReturnCode_t::ReturnCodeValue::RETCODE_NOT_ALLOWED_BY_SECURITY:
      return "RETCODE_NOT_ALLOWED_BY_SECURITY";
    default:
      return "UNKNOWN";
  }
}

}  // namespace std