// Copyright 2023 Proyectos y Sistemas de Mantenimiento SL (eProsima).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef _FASTCDR_CDRSIZECALCULATOR_HPP_
#define _FASTCDR_CDRSIZECALCULATOR_HPP_

#include <array>
#include <bitset>
#include <cstdint>
#include <limits>
#include <map>
#include <vector>

#include "fastcdr/fastcdr_dll.h"

#include "fastcdr/CdrEncoding.hpp"
#include "fastcdr/cdr/fixed_size_string.hpp"
#include "fastcdr/detail/container_recursive_inspector.hpp"
#include "fastcdr/exceptions/BadParamException.h"
#include "fastcdr/xcdr/external.hpp"
#include "fastcdr/xcdr/MemberId.hpp"
#include "fastcdr/xcdr/optional.hpp"

namespace eprosima {
namespace fastcdr {

class CdrSizeCalculator;

template<class _T>
extern size_t calculate_serialized_size(
        CdrSizeCalculator&,
        const _T&,
        size_t&);

/*!
 * @brief This class offers an interface to calculate the encoded size of a type serialized using a support encoding
 * algorithm.
 * @ingroup FASTCDRAPIREFERENCE
 */
class CdrSizeCalculator
{
public:

    /*!
     * @brief Constructor.
     * @param[in] cdr_version Represents the version of the encoding algorithm that will be used for the encoding.
     * The default value is CdrVersion::XCDRv2.
     */
    Cdr_DllAPI CdrSizeCalculator(
            CdrVersion cdr_version);

    /*!
     * @brief Constructor.
     * @param[in] cdr_version Represents the version of the encoding algorithm that will be used for the encoding.
     * The default value is CdrVersion::XCDRv2.
     * @param[in] encoding Represents the initial encoding.
     */
    Cdr_DllAPI CdrSizeCalculator(
            CdrVersion cdr_version,
            EncodingAlgorithmFlag encoding);

    /*!
     * @brief Retrieves the version of the encoding algorithm used by the instance.
     * @return Configured CdrVersion.
     */
    Cdr_DllAPI CdrVersion get_cdr_version() const;

    /*!
     * @brief Retrieves the current encoding algorithm used by the instance.
     * @return Configured EncodingAlgorithmFlag.
     */
    Cdr_DllAPI EncodingAlgorithmFlag get_encoding() const;

    /*!
     * @brief Generic template which calculates the encoded size of an instance of an unknown type.
     * @tparam _T Instance's type.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T, typename std::enable_if<!std::is_enum<_T>::value>::type* = nullptr, typename = void>
    size_t calculate_serialized_size(
            const _T& data,
            size_t& current_alignment)
    {
        return eprosima::fastcdr::calculate_serialized_size(*this, data, current_alignment);
    }

    /*!
     * @brief Template which calculates the encoded size of an instance of an enumeration of 32bits.
     * @tparam _T Instance's type.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            int32_t>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const _T& data,
            size_t& current_alignment)
    {
        return calculate_serialized_size(static_cast<int32_t>(data), current_alignment);
    }

    /*!
     * @brief Template which calculates the encoded size of an instance of an enumeration of unsigned 32bits.
     * @tparam _T Instance's type.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            uint32_t>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const _T& data,
            size_t& current_alignment)
    {
        return calculate_serialized_size(static_cast<uint32_t>(data), current_alignment);
    }

    /*!
     * @brief Template which calculates the encoded size of an instance of an enumeration of 16bits.
     * @tparam _T Instance's type.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            int16_t>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const _T& data,
            size_t& current_alignment)
    {
        return calculate_serialized_size(static_cast<int16_t>(data), current_alignment);
    }

    /*!
     * @brief Template which calculates the encoded size of an instance of an enumeration of unsigned 16bits.
     * @tparam _T Instance's type.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            uint16_t>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const _T& data,
            size_t& current_alignment)
    {
        return calculate_serialized_size(static_cast<uint16_t>(data), current_alignment);
    }

    /*!
     * @brief Template which calculates the encoded size of an instance of an enumeration of 8bits.
     * @tparam _T Instance's type.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            int8_t>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const _T& data,
            size_t& current_alignment)
    {
        return calculate_serialized_size(static_cast<int8_t>(data), current_alignment);
    }

    /*!
     * @brief Template which calculates the encoded size of an instance of an enumeration of unsigned 8bits.
     * @tparam _T Instance's type.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            uint8_t>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const _T& data,
            size_t& current_alignment)
    {
        return calculate_serialized_size(static_cast<uint8_t>(data), current_alignment);
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an int8_t.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const int8_t& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        ++current_alignment;
        return 1;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an uint8_t.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const uint8_t& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        ++current_alignment;
        return 1;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a char.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const char& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        ++current_alignment;
        return 1;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a bool.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const bool& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        ++current_alignment;
        return 1;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a wchar.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const wchar_t& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {2 + alignment(current_alignment, 2)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a int16_t.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const int16_t& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {2 + alignment(current_alignment, 2)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a uint16_t.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const uint16_t& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {2 + alignment(current_alignment, 2)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a int32_t.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const int32_t& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {4 + alignment(current_alignment, 4)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a uint32_t.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const uint32_t& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {4 + alignment(current_alignment, 4)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a int64_t.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const int64_t& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {8 + alignment(current_alignment, align64_)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a uint64_t.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const uint64_t& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {8 + alignment(current_alignment, align64_)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a float.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const float& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {4 + alignment(current_alignment, 4)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a double.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const double& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {8 + alignment(current_alignment, align64_)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a long double.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const long double& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {16 + alignment(current_alignment, align64_)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a std::string.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const std::string& data,
            size_t& current_alignment)
    {
        size_t calculated_size {4 + alignment(current_alignment, 4) + data.size() + 1};
        current_alignment += calculated_size;
        serialized_member_size_ = SERIALIZED_MEMBER_SIZE;

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a std::wstring.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const std::wstring& data,
            size_t& current_alignment)
    {
        size_t calculated_size {4 + alignment(current_alignment, 4) + data.size() * 2};
        current_alignment += calculated_size;

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a fixed_string.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template <size_t MAX_CHARS>
    size_t calculate_serialized_size(
            const fixed_string<MAX_CHARS>& data,
            size_t& current_alignment)
    {
        size_t calculated_size {4 + alignment(current_alignment, 4) + data.size() + 1};
        current_alignment += calculated_size;
        serialized_member_size_ = SERIALIZED_MEMBER_SIZE;

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a sequence of non-primitives.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T, class _Alloc, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const std::vector<_T, _Alloc>& data,
            size_t& current_alignment)
    {
        size_t initial_alignment {current_alignment};

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            // DHEADER
            current_alignment += 4 + alignment(current_alignment, 4);
        }

        current_alignment += 4 + alignment(current_alignment, 4);

        size_t calculated_size {current_alignment - initial_alignment};
        calculated_size += calculate_array_serialized_size(data.data(), data.size(), current_alignment);

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            // Inform DHEADER can be joined with NEXTINT
            serialized_member_size_ = SERIALIZED_MEMBER_SIZE;
        }

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a sequence of primitives.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T, class _Alloc, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const std::vector<_T, _Alloc>& data,
            size_t& current_alignment)
    {
        size_t initial_alignment {current_alignment};

        current_alignment += 4 + alignment(current_alignment, 4);

        size_t calculated_size {current_alignment - initial_alignment};
        calculated_size += calculate_array_serialized_size(data.data(), data.size(), current_alignment);

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            serialized_member_size_ = get_serialized_member_size<_T>();
        }

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a sequence of bool.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_serialized_size(
            const std::vector<bool>& data,
            size_t& current_alignment)
    {
        size_t calculated_size {data.size() + 4 + alignment(current_alignment, 4)};
        current_alignment += calculated_size;

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T, size_t _Size>
    size_t calculate_serialized_size(
            const std::array<_T, _Size>& data,
            size_t& current_alignment)
    {
        size_t initial_alignment {current_alignment};

        if (CdrVersion::XCDRv2 == cdr_version_ &&
                !is_multi_array_primitive(&data))
        {
            // DHEADER
            current_alignment += 4 + alignment(current_alignment, 4);
        }

        size_t calculated_size {current_alignment - initial_alignment};
        calculated_size += calculate_array_serialized_size(data.data(), data.size(), current_alignment);

        if (CdrVersion::XCDRv2 == cdr_version_ &&
                !is_multi_array_primitive(&data))
        {
            // Inform DHEADER can be joined with NEXTINT
            serialized_member_size_ = SERIALIZED_MEMBER_SIZE;
        }

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a map of non-primitives.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _K, class _V, typename std::enable_if<!std::is_enum<_V>::value &&
            !std::is_arithmetic<_V>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const std::map<_K, _V>& data,
            size_t& current_alignment)
    {
        size_t initial_alignment {current_alignment};

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            // DHEADER
            current_alignment += 4 + alignment(current_alignment, 4);
        }

        current_alignment += 4 + alignment(current_alignment, 4);

        size_t calculated_size {current_alignment - initial_alignment};
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            calculated_size += calculate_serialized_size(it->first, current_alignment);
            calculated_size += calculate_serialized_size(it->second, current_alignment);
        }

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            // Inform DHEADER can be joined with NEXTINT
            serialized_member_size_ = SERIALIZED_MEMBER_SIZE;
        }

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a map of primitives.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _K, class _V, typename std::enable_if<std::is_enum<_V>::value ||
            std::is_arithmetic<_V>::value>::type* = nullptr>
    size_t calculate_serialized_size(
            const std::map<_K, _V>& data,
            size_t& current_alignment)
    {
        size_t initial_alignment {current_alignment};

        current_alignment += 4 + alignment(current_alignment, 4);

        size_t calculated_size {current_alignment - initial_alignment};
        for (auto it = data.begin(); it != data.end(); ++it)
        {
            calculated_size += calculate_serialized_size(it->first, current_alignment);
            calculated_size += calculate_serialized_size(it->second, current_alignment);
        }

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a bitset of 8bits.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<size_t N, typename std::enable_if < (N < 9) > ::type* = nullptr>
    size_t calculate_serialized_size(
            const std::bitset<N>& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        ++current_alignment;
        return 1;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a bitset of 16bits.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<size_t N, typename std::enable_if < (8 < N && N < 17) > ::type* = nullptr>
    size_t calculate_serialized_size(
            const std::bitset<N>& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {2 + alignment(current_alignment, 2)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a bitset of 32bits.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<size_t N, typename std::enable_if < (16 < N && N < 33) > ::type* = nullptr>
    size_t calculate_serialized_size(
            const std::bitset<N>& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {4 + alignment(current_alignment, 4)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a bitset of 64bits.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<size_t N, typename std::enable_if < (32 < N && N < 65) > ::type* = nullptr>
    size_t calculate_serialized_size(
            const std::bitset<N>& data,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {8 + alignment(current_alignment, align64_)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an optional type.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T>
    size_t calculate_serialized_size(
            const optional<_T>& data,
            size_t& current_alignment)
    {
        size_t initial_alignment = current_alignment;

        if (CdrVersion::XCDRv2 == cdr_version_ &&
                EncodingAlgorithmFlag::PL_CDR2 != current_encoding_)
        {
            // Take into account the boolean is_present;
            ++current_alignment;
        }

        size_t calculated_size {current_alignment - initial_alignment};

        if (data.has_value())
        {
            calculated_size += calculate_serialized_size(data.value(), current_alignment);
        }

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an external type.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @exception exception::BadParamException This exception is thrown when the external is null.
     * @return Encoded size of the instance.
     */
    template<class _T>
    size_t calculate_serialized_size(
            const external<_T>& data,
            size_t& current_alignment)
    {
        if (!data)
        {
            throw exception::BadParamException("External member is null");
        }

        return calculate_serialized_size(*data, current_alignment);
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of unknown type.
     * @tparam _T Array's type.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T>
    size_t calculate_array_serialized_size(
            const _T* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        size_t calculated_size {0};

        for (size_t count = 0; count < num_elements; ++count)
        {
            calculated_size += calculate_serialized_size(data[count], current_alignment);
        }

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of int8_t.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const int8_t* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        current_alignment += num_elements;
        return num_elements;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of uint8_t.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const uint8_t* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        current_alignment += num_elements;
        return num_elements;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of char.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const char* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        current_alignment += num_elements;
        return num_elements;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of wchar.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const wchar_t* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 2 + alignment(current_alignment, 2)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of int16_t.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const int16_t* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 2 + alignment(current_alignment, 2)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of uint16_t.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const uint16_t* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 2 + alignment(current_alignment, 2)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of int32_t.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const int32_t* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 4 + alignment(current_alignment, 4)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of uint32_t.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const uint32_t* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 4 + alignment(current_alignment, 4)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of int64_t.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const int64_t* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 8 + alignment(current_alignment, align64_)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of uint64_t.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const uint64_t* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 8 + alignment(current_alignment, align64_)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of float.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const float* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 4 + alignment(current_alignment, 4)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of double.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const double* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 8 + alignment(current_alignment, align64_)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of an array of long double.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const long double* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        static_cast<void>(data);
        size_t calculated_size {num_elements* 16 + alignment(current_alignment, align64_)};
        current_alignment += calculated_size;
        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an instance of a multi-dimensional array.
     * @param[in] data Reference to the array's instance.
     * @param[in] num_elements Number of elements in the array.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T, size_t _N>
    size_t calculate_array_serialized_size(
            const std::array<_T, _N>* data,
            size_t num_elements,
            size_t& current_alignment)
    {
        return calculate_array_serialized_size(data->data(), num_elements * data->size(), current_alignment);
    }

    /*!
     * @brief Specific template which calculates the encoded size of an std::vector of primitives as an array.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T, class _Alloc, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    size_t calculate_array_serialized_size(
            const std::vector<_T, _Alloc>& data,
            size_t& current_alignment)
    {
        return calculate_array_serialized_size(data.data(), data.size(), current_alignment);
    }

    /*!
     * @brief Specific template which calculates the encoded size of an std::vector of non-primitives as an array.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    template<class _T, class _Alloc, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    size_t calculate_array_serialized_size(
            const std::vector<_T, _Alloc>& data,
            size_t& current_alignment)
    {
        size_t initial_alignment {current_alignment};

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            // DHEADER
            current_alignment += 4 + alignment(current_alignment, 4);
        }

        size_t calculated_size {current_alignment - initial_alignment};
        calculated_size += calculate_array_serialized_size(data.data(), data.size(), current_alignment);

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            // Inform DHEADER can be joined with NEXTINT
            serialized_member_size_ = SERIALIZED_MEMBER_SIZE;
        }

        return calculated_size;
    }

    /*!
     * @brief Specific template which calculates the encoded size of an std::vector of bool as an array.
     * @param[in] data Reference to the instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the instance.
     */
    TEMPLATE_SPEC
    size_t calculate_array_serialized_size(
            const std::vector<bool>& data,
            size_t& current_alignment)
    {
        current_alignment += data.size();
        return data.size();
    }

    /*!
     * @brief Generic template which calculates the encoded size of the constructed type's member of a unknown type.
     * @tparam _T Member's type.
     * @param[in] id Member's identifier.
     * @param[in] data Reference to the member's instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the member's instance.
     */
    template<class _T>
    size_t calculate_member_serialized_size(
            const MemberId& id,
            const _T& data,
            size_t& current_alignment)
    {
        size_t initial_alignment {current_alignment};

        if (EncodingAlgorithmFlag::PL_CDR == current_encoding_ ||
                EncodingAlgorithmFlag::PL_CDR2 == current_encoding_)
        {
            // Align to 4 for the XCDR header before calculating the data serialized size.
            current_alignment += alignment(current_alignment, 4);
        }

        size_t prev_size {current_alignment - initial_alignment};
        size_t extra_size {0};

        if (EncodingAlgorithmFlag::PL_CDR == current_encoding_)
        {
            current_alignment = 0;
        }

        size_t calculated_size {calculate_serialized_size(data, current_alignment)};

        if (CdrVersion::XCDRv2 == cdr_version_ && EncodingAlgorithmFlag::PL_CDR2 == current_encoding_ &&
                0 < calculated_size)
        {

            if (8 < calculated_size ||
                    (1 != calculated_size && 2 != calculated_size && 4 != calculated_size &&
                    8 != calculated_size))
            {
                extra_size = 8; // Long EMHEADER.
                if (NO_SERIALIZED_MEMBER_SIZE != serialized_member_size_)
                {
                    calculated_size -= 4; // Join NEXTINT and DHEADER.
                }
            }
            else
            {
                extra_size = 4; // EMHEADER;
            }
        }
        else if (CdrVersion::XCDRv1 == cdr_version_ && EncodingAlgorithmFlag::PL_CDR == current_encoding_ &&
                0 < calculated_size)
        {
            extra_size = 4; // ShortMemberHeader

            if (0x3F00 < id.id || calculated_size > std::numeric_limits<uint16_t>::max())
            {
                extra_size += 8; // LongMemberHeader
            }

        }

        calculated_size += prev_size + extra_size;
        if (EncodingAlgorithmFlag::PL_CDR != current_encoding_)
        {
            current_alignment += extra_size;
        }

        serialized_member_size_ = NO_SERIALIZED_MEMBER_SIZE;

        return calculated_size;
    }

    /*!
     * @brief Generic template which calculates the encoded size of the constructed type's member of type optional.
     * @tparam _T Member's optional type.
     * @param[in] id Member's identifier.
     * @param[in] data Reference to the member's instance.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return Encoded size of the member's instance.
     */
    template<class _T>
    size_t calculate_member_serialized_size(
            const MemberId& id,
            const optional<_T>& data,
            size_t& current_alignment)
    {
        size_t initial_alignment = current_alignment;

        if (CdrVersion::XCDRv2 != cdr_version_ ||
                EncodingAlgorithmFlag::PL_CDR2 == current_encoding_)
        {
            if (data.has_value() || EncodingAlgorithmFlag::PLAIN_CDR == current_encoding_)
            {
                // Align to 4 for the XCDR header before calculating the data serialized size.
                current_alignment += alignment(current_alignment, 4);
            }
        }

        size_t prev_size = {current_alignment - initial_alignment};
        size_t extra_size {0};

        if (CdrVersion::XCDRv1 == cdr_version_ &&
                (data.has_value() || EncodingAlgorithmFlag::PLAIN_CDR == current_encoding_))
        {
            current_alignment = 0;
        }

        size_t calculated_size {calculate_serialized_size(data, current_alignment)};

        if (CdrVersion::XCDRv2 == cdr_version_ && EncodingAlgorithmFlag::PL_CDR2 == current_encoding_ &&
                0 < calculated_size)
        {
            if (8 < calculated_size ||
                    (1 != calculated_size && 2 != calculated_size && 4 != calculated_size &&
                    8 != calculated_size))
            {
                extra_size = 8; // Long EMHEADER.
                if (NO_SERIALIZED_MEMBER_SIZE != serialized_member_size_)
                {
                    calculated_size -= 4; // Join NEXTINT and DHEADER.
                }
            }
            else
            {
                extra_size = 4; // EMHEADER;
            }
        }
        else if (CdrVersion::XCDRv1 == cdr_version_ &&
                (0 < calculated_size || EncodingAlgorithmFlag::PLAIN_CDR == current_encoding_))
        {
            extra_size = 4; // ShortMemberHeader

            if (0x3F00 < id.id || calculated_size > std::numeric_limits<uint16_t>::max())
            {
                extra_size += 8; // LongMemberHeader
            }

        }

        calculated_size += prev_size + extra_size;
        if (CdrVersion::XCDRv1 != cdr_version_)
        {
            current_alignment += extra_size;
        }


        return calculated_size;
    }

    /*!
     * @brief Indicates a new constructed type will be calculated.
     * @param[in] new_encoding New encoding algorithm used for the constructed type.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return If new encoding algorithm encodes a header, return the encoded size of it.
     */
    Cdr_DllAPI size_t begin_calculate_type_serialized_size(
            EncodingAlgorithmFlag new_encoding,
            size_t& current_alignment);

    /*!
     * @brief Indicates the ending of a constructed type.
     * @param[in] new_encoding New encoding algorithm used after the constructed type.
     * @param[inout] current_alignment Current alignment in the encoding.
     * @return If current encoding algorithm encodes a final mark, return the encoded size of it.
     */
    Cdr_DllAPI size_t end_calculate_type_serialized_size(
            EncodingAlgorithmFlag new_encoding,
            size_t& current_alignment);

private:

    CdrSizeCalculator() = delete;

    CdrVersion cdr_version_ {CdrVersion::XCDRv2};

    EncodingAlgorithmFlag current_encoding_ {EncodingAlgorithmFlag::PLAIN_CDR2};

    enum SerializedMemberSizeForNextInt
    {
        NO_SERIALIZED_MEMBER_SIZE,
        SERIALIZED_MEMBER_SIZE,
        SERIALIZED_MEMBER_SIZE_4,
        SERIALIZED_MEMBER_SIZE_8
    }
    //! Specifies if a DHEADER was serialized. Used to calculate XCDRv2 member headers.
    serialized_member_size_ {NO_SERIALIZED_MEMBER_SIZE};

    //! Align for types equal or greater than 64bits.
    size_t align64_ {4};

    inline size_t alignment(
            size_t current_alignment,
            size_t data_size) const
    {
        return (data_size - (current_alignment % data_size)) & (data_size - 1);
    }

    template<class _T, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    constexpr SerializedMemberSizeForNextInt get_serialized_member_size() const
    {
        return (1 == sizeof(_T) ? SERIALIZED_MEMBER_SIZE :
               (4 == sizeof(_T) ? SERIALIZED_MEMBER_SIZE_4 :
               (8 == sizeof(_T) ? SERIALIZED_MEMBER_SIZE_8 :  NO_SERIALIZED_MEMBER_SIZE)));
    }

};

}        // namespace fastcdr
}        // namespace eprosima

#endif // _FASTCDR_CDRSIZECALCULATOR_HPP_
