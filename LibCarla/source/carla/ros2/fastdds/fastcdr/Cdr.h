// Copyright 2016 Proyectos y Sistemas de Mantenimiento SL (eProsima).
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

#ifndef _FASTCDR_CDR_H_
#define _FASTCDR_CDR_H_

#include <array>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "fastcdr/fastcdr_dll.h"

#include "fastcdr/CdrEncoding.hpp"
#include "fastcdr/cdr/fixed_size_string.hpp"
#include "fastcdr/detail/container_recursive_inspector.hpp"
#include "fastcdr/exceptions/BadParamException.h"
#include "fastcdr/exceptions/Exception.h"
#include "fastcdr/exceptions/NotEnoughMemoryException.h"
#include "fastcdr/FastBuffer.h"
#include "fastcdr/xcdr/external.hpp"
#include "fastcdr/xcdr/MemberId.hpp"
#include "fastcdr/xcdr/optional.hpp"

#if !__APPLE__ && !__FreeBSD__ && !__VXWORKS__
#include <malloc.h>
#else
#include <stdlib.h>
#endif // if !__APPLE__ && !__FreeBSD__ && !__VXWORKS__

namespace eprosima {
namespace fastcdr {

class Cdr;

template<class _T>
extern void serialize(
        Cdr&,
        const _T&);

template<class _T>
extern void deserialize(
        Cdr&,
        _T&);

/*!
 * @brief This class offers an interface to serialize/deserialize some basic types using CDR protocol inside an eprosima::fastcdr::FastBuffer.
 * @ingroup FASTCDRAPIREFERENCE
 */
class Cdr
{
public:

    /*!
     * @brief This enumeration represents endianness types.
     */
    typedef enum : uint8_t
    {
        //! @brief Big endianness.
        BIG_ENDIANNESS = 0x0,
        //! @brief Little endianness.
        LITTLE_ENDIANNESS = 0x1
    } Endianness;

    //! Default endianess in the system.
    Cdr_DllAPI static const Endianness DEFAULT_ENDIAN;

    /*!
     * Used to decide, in encoding algorithms where member headers support a short header version and a long header
     * version, which one will be used.
     */
    typedef enum
    {
        //! Initially a short member header is allocated and cannot be changed. This option may cause an exception.
        SHORT_HEADER,
        //! Initially a long member header is allocated and cannot be changed.
        LONG_HEADER,
        //! Initially a short member header is allocated but can be changed to the longer version.
        AUTO_WITH_SHORT_HEADER_BY_DEFAULT,
        //! Initially a long member header is allocated but can be changed to the shorter version.
        AUTO_WITH_LONG_HEADER_BY_DEFAULT
    } XCdrHeaderSelection;

    /*!
     * @brief This class stores the current state of a CDR serialization.
     */
    class state
    {
        friend class Cdr;

    public:

        //! Default constructor.
        Cdr_DllAPI state(
                const Cdr& cdr);

        //! Copy constructor.
        Cdr_DllAPI state(
                const state& state);


        //! Compares two states.
        Cdr_DllAPI bool operator ==(
                const state& other_state) const;

    private:

        state& operator =(
                const state& state) = delete;

        //! The position in the buffer when the state was created.
        const FastBuffer::iterator offset_;

        //! The position from the alignment is calculated, when the state was created.
        const FastBuffer::iterator origin_;

        //! This attribute specifies if it is needed to swap the bytes when the state is created.
        bool swap_bytes_ {false};

        //! Stores the last datasize serialized/deserialized when the state was created.
        size_t last_data_size_ {0};

        //! Not related with the state. Next member id which will be encoded.
        MemberId next_member_id_;

        //! Not related with the state. Used by encoding algorithms to set the encoded member size.
        uint32_t member_size_ {0};

        //! Not related with the state. Used by encoding algorithms to store the selected member header version.
        XCdrHeaderSelection header_selection_ {XCdrHeaderSelection::AUTO_WITH_SHORT_HEADER_BY_DEFAULT};

        //! Not related with the state. Used by encoding algorithms to store the allocated member header version.
        XCdrHeaderSelection header_serialized_ {XCdrHeaderSelection::SHORT_HEADER};

        //! Not related with the state. Used by encoding algorithms to store the previous encoding algorithm.
        EncodingAlgorithmFlag previous_encoding_ {EncodingAlgorithmFlag::PLAIN_CDR2};
    };

    /*!
     * @brief This constructor creates an eprosima::fastcdr::Cdr object that can serialize/deserialize
     * the assigned buffer.
     * @param cdr_buffer A reference to the buffer that contains (or will contain) the CDR representation.
     * @param endianness The initial endianness that will be used. The default value is the endianness of the system.
     * @param cdr_version Represents the type of encoding algorithm that will be used for the encoding.
     * The default value is CdrVersion::XCDRv2.
     */
    Cdr_DllAPI Cdr(
            FastBuffer& cdr_buffer,
            const Endianness endianness = DEFAULT_ENDIAN,
            const CdrVersion cdr_version = XCDRv2);

    /*!
     * @brief This function reads the encapsulation of the CDR stream.
     *        If the CDR stream contains an encapsulation, then this function should be called before starting to deserialize.
     *        CdrVersion and EncodingAlgorithmFlag internal values will be changed to the ones specified by the
     *        encapsulation.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to deserialize an invalid value.
     */
    Cdr_DllAPI Cdr& read_encapsulation();

    /*!
     * @brief This function writes the encapsulation of the CDR stream.
     *        If the CDR stream should contain an encapsulation, then this function should be called before starting to serialize.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize_encapsulation();

    /*!
     * @brief Retrieves the CdrVersion used by the instance.
     * @return Configured CdrVersion.
     */
    Cdr_DllAPI CdrVersion get_cdr_version() const;

    /*!
     * @brief Returns the EncodingAlgorithmFlag set in the encapsulation when the CDR type is
     * CdrVersion::DDS_CDR, CdrVersion::XCDRv1 or CdrVersion::XCDRv2.
     * @return The specified flag in the encapsulation.
     */
    Cdr_DllAPI EncodingAlgorithmFlag get_encoding_flag() const;

    /*!
     * @brief Sets the EncodingAlgorithmFlag for the encapsulation when the CDR type is
     * CdrVersion::DDS_CDR, CdrVersion::XCDRv1 or CdrVersion::XCDRv2.
     * This function only works when is called before starting the encoding/decoding.
     * @param[in] encoding_flag Value to be used in the encapsulation.
     * @return Indicates whether the setting was successful.
     */
    Cdr_DllAPI bool set_encoding_flag(
            EncodingAlgorithmFlag encoding_flag);

    /*!
     * @brief This function returns the option flags when the CDR type is eprosima::fastcdr::DDS_CDR.
     * @return The option flags.
     */
    Cdr_DllAPI std::array<uint8_t, 2> get_dds_cdr_options() const;

    /*!
     * @brief This function sets the option flags when the CDR type is eprosima::fastcdr::DDS_CDR.
     * @param options New value for the option flags.
     */
    Cdr_DllAPI void set_dds_cdr_options(
            const std::array<uint8_t, 2>& options);

    /*!
     * @brief This function sets the current endianness used by the CDR type.
     * @param endianness The new endianness value.
     */
    Cdr_DllAPI void change_endianness(
            Endianness endianness);

    /*!
     * @brief This function returns the current endianness used by the CDR type.
     * @return The endianness.
     */
    Cdr_DllAPI Endianness endianness() const;

    /*!
     * @brief This function skips a number of bytes in the CDR stream buffer.
     * @param num_bytes The number of bytes that will be jumped.
     * @return True is returned when it works successfully. Otherwise, false is returned.
     */
    Cdr_DllAPI bool jump(
            size_t num_bytes);

    /*!
     * @brief This function resets the current position in the buffer to the beginning.
     */
    Cdr_DllAPI void reset();

    /*!
     * @brief This function returns the pointer to the current used buffer.
     * @return Pointer to the starting position of the buffer.
     */
    Cdr_DllAPI char* get_buffer_pointer();

    /*!
     * @brief This function returns the current position in the CDR stream.
     * @return Pointer to the current position in the buffer.
     */
    Cdr_DllAPI char* get_current_position();

    /*!
     * @brief This function returns the length of the serialized data inside the stream.
     * @return The length of the serialized data.
     */
    Cdr_DllAPI size_t get_serialized_data_length() const;

    /*!
     * @brief Returns the number of bytes needed to align a position to certain data size.
     * @param current_alignment Position to be aligned.
     * @param data_size Size of next data to process (should be power of two).
     * @return Number of required alignment bytes.
     */
    inline static size_t alignment(
            size_t current_alignment,
            size_t data_size)
    {
        return (data_size - (current_alignment % data_size)) & (data_size - 1);
    }

    /*!
     * @brief Returns the current state of the CDR serialization process.
     * @return The current state of the CDR serialization process.
     */
    Cdr_DllAPI state get_state() const;

    /*!
     * @brief Sets a previous state of the CDR serialization process;
     * @param state Previous state that will be set.
     */
    Cdr_DllAPI void set_state(
            const state& state);

    /*!
     * @brief This function moves the alignment forward.
     * @param num_bytes The number of bytes the alignment should advance.
     * @return True If alignment was moved successfully.
     */
    Cdr_DllAPI bool move_alignment_forward(
            size_t num_bytes);

    /*!
     * @brief This function resets the alignment to the current position in the buffer.
     */
    inline void reset_alignment()
    {
        origin_ = offset_;
        last_data_size_ = 0;
    }

    /*!
     * @brief Encodes the value into the buffer.
     *
     * If previously a MemberId was set using operator<<, this operator will encode the value as a member of a type
     * consistent with the set member identifier and according to the encoding algorithm used.
     *
     * In other case, the operator will simply encode the value.
     *
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    inline Cdr& operator <<(
            const _T& value)
    {
        if (MEMBER_ID_INVALID == next_member_id_)
        {
            serialize(value);
        }
        else
        {
            serialize_member(next_member_id_, value);

        }

        return *this;
    }

    /*!
     * @brief Decodes the value from the buffer.
     *
     * If this operator is called while decoding members of a type, this operator will decode the value as a member
     * according to the encoding algorithm used.
     *
     * In other case, the operator will simply decode the value.
     *
     * @param[out] value Reference to the variable where the value will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a position
     * that exceeds the internal memory size.
     */
    template<class _T>
    inline Cdr& operator >>(
            _T& value)
    {
        if (MEMBER_ID_INVALID == next_member_id_)
        {
            deserialize(value);
        }
        else
        {
            deserialize_member(value);
        }
        return *this;
    }

    /*!
     * @brief Encodes the value of a type into the buffer.
     *
     * To do that, the encoder expects a function `serialize` to be provided by the type.
     *
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T, typename std::enable_if<!std::is_enum<_T>::value>::type* = nullptr, typename = void>
    Cdr& serialize(
            const _T& value)
    {
        eprosima::fastcdr::serialize(*this, value);
        return *this;
    }

    /*!
     * @brief Encodes the value of a type with a different endianness.
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @param endianness Endianness that will be used in the serialization of this value.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& serialize(
            const _T& value,
            Endianness endianness)
    {
        bool aux_swap = swap_bytes_;
        swap_bytes_ = (swap_bytes_ && (static_cast<Endianness>(endianness_) == endianness)) ||
                (!swap_bytes_ && (static_cast<Endianness>(endianness_) != endianness));

        try
        {
            serialize(value);
            swap_bytes_ = aux_swap;
        }
        catch (exception::Exception& ex)
        {
            swap_bytes_ = aux_swap;
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief Encodes the value of a enumerator into the buffer.
     *
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            int32_t>::value>::type* = nullptr>
    Cdr& serialize(
            const _T& value)
    {
        return serialize(static_cast<int32_t>(value));
    }

    /*!
     * @brief Encodes the value of a enumerator into the buffer.
     *
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            uint32_t>::value>::type* = nullptr>
    Cdr& serialize(
            const _T& value)
    {
        return serialize(static_cast<uint32_t>(value));
    }

    /*!
     * @brief Encodes the value of a enumerator into the buffer.
     *
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            int16_t>::value>::type* = nullptr>
    Cdr& serialize(
            const _T& value)
    {
        return serialize(static_cast<int16_t>(value));
    }

    /*!
     * @brief Encodes the value of a enumerator into the buffer.
     *
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            uint16_t>::value>::type* = nullptr>
    Cdr& serialize(
            const _T& value)
    {
        return serialize(static_cast<uint16_t>(value));
    }

    /*!
     * @brief Encodes the value of a enumerator into the buffer.
     *
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            int8_t>::value>::type* = nullptr>
    Cdr& serialize(
            const _T& value)
    {
        return serialize(static_cast<int8_t>(value));
    }

    /*!
     * @brief Encodes the value of a enumerator into the buffer.
     *
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            uint8_t>::value>::type* = nullptr>
    Cdr& serialize(
            const _T& value)
    {
        return serialize(static_cast<uint8_t>(value));
    }

    /*!
     * @brief This function serializes an octet.
     * @param octet_t The value of the octet that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const uint8_t& octet_t);

    /*!
     * @brief This function serializes a character.
     * @param char_t The value of the character that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const char char_t);

    /*!
     * @brief This function serializes an int8_t.
     * @param int8 The value of the int8_t that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const int8_t int8);

    /*!
     * @brief This function serializes an unsigned short.
     * @param ushort_t The value of the unsigned short that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const uint16_t ushort_t);

    /*!
     * @brief This function serializes a short.
     * @param short_t The value of the short that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const int16_t short_t);

    /*!
     * @brief This function serializes an unsigned long.
     * @param ulong_t The value of the unsigned long that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const uint32_t ulong_t);

    /*!
     * @brief This function serializes a long.
     * @param long_t The value of the long that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const int32_t long_t);

    /*!
     * @brief This function serializes a wide-char.
     * @param wchar The value of the wide-char that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const wchar_t wchar);

    /*!
     * @brief This function serializes an unsigned long long.
     * @param ulonglong_t The value of the unsigned long long that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const uint64_t ulonglong_t);

    /*!
     * @brief This function serializes a long long.
     * @param longlong_t The value of the long long that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const int64_t longlong_t);

    /*!
     * @brief This function serializes a float.
     * @param float_t The value of the float that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const float float_t);

    /*!
     * @brief This function serializes a double.
     * @param double_t The value of the double that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const double double_t);

    /*!
     * @brief This function serializes a long double.
     * @param ldouble_t The value of the long double that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     * @note Due to internal representation differences, WIN32 and *NIX like systems are not compatible.
     */
    Cdr_DllAPI Cdr& serialize(
            const long double ldouble_t);

    /*!
     * @brief This function serializes a boolean.
     * @param bool_t The value of the boolean that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const bool bool_t);

    /*!
     * @brief This function serializes a string.
     * @param string_t The pointer to the string that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            char* string_t);

    /*!
     * @brief This function serializes a string.
     * @param string_t The pointer to the string that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const char* string_t);

    /*!
     * @brief This function serializes a wstring.
     * @param string_t The pointer to the wstring that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize(
            const wchar_t* string_t);

    /*!
     * @brief This function serializes a std::string.
     * @param string_t The string that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to serialize a string with null characters.
     */
    TEMPLATE_SPEC
    Cdr& serialize(
            const std::string& string_t)
    {
        // Check there are no null characters in the string.
        const char* c_str = string_t.c_str();
        const auto str_len = strlen(c_str);
        if (string_t.size() > str_len)
        {
            throw exception::BadParamException("The string contains null characters");
        }

        return serialize_sequence(c_str, str_len + 1);
    }

    /*!
     * @brief This function serializes a std::wstring.
     * @param string_t The wstring that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize(
            const std::wstring& string_t)
    {
        return serialize(string_t.c_str());
    }

    /*!
     * @brief Encodes a eprosima::fastcdr::fixed_string in the buffer.
     * @param[in] value A reference to the fixed string which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template <size_t MAX_CHARS>
    Cdr& serialize(
            const fixed_string<MAX_CHARS>& value)
    {
        return serialize(value.c_str());
    }

    /*!
     * @brief This function template serializes an array.
     * @param array_t The array that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _T, size_t _Size>
    Cdr& serialize(
            const std::array<_T, _Size>& array_t)
    {
        if (!is_multi_array_primitive(&array_t))
        {
            Cdr::state dheader_state {allocate_xcdrv2_dheader()};

            serialize_array(array_t.data(), array_t.size());

            set_xcdrv2_dheader(dheader_state);
        }
        else
        {
            serialize_array(array_t.data(), array_t.size());
        }

        return *this;
    }

    /*!
     * @brief This function template serializes a sequence of non-primitive.
     * @param vector_t The sequence that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& serialize(
            const std::vector<_T, _Alloc>& vector_t)
    {
        Cdr::state dheader_state {allocate_xcdrv2_dheader()};

        serialize(static_cast<int32_t>(vector_t.size()));

        try
        {
            serialize_array(vector_t.data(), vector_t.size());
        }
        catch (exception::Exception& ex)
        {
            set_state(dheader_state);
            ex.raise();
        }

        set_xcdrv2_dheader(dheader_state);

        return *this;
    }

    /*!
     * @brief This function template serializes a sequence of primitive.
     * @param vector_t The sequence that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& serialize(
            const std::vector<_T, _Alloc>& vector_t)
    {
        state state_before_error(*this);

        serialize(static_cast<int32_t>(vector_t.size()));

        try
        {
            serialize_array(vector_t.data(), vector_t.size());
        }
        catch (exception::Exception& ex)
        {
            set_state(state_before_error);
            ex.raise();
        }

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            serialized_member_size_ = get_serialized_member_size<_T>();
        }

        return *this;
    }

    /*!
     * @brief This function template serializes a sequence of booleans.
     * @param vector_t The sequence that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize(
            const std::vector<bool>& vector_t)
    {
        return serialize_bool_sequence(vector_t);
    }

    /*!
     * @brief This function template serializes a map of non-primitive.
     * @param map_t The map that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _K, class _T, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& serialize(
            const std::map<_K, _T>& map_t)
    {
        Cdr::state dheader_state {allocate_xcdrv2_dheader()};

        serialize(static_cast<int32_t>(map_t.size()));

        try
        {
            for (auto it_pair = map_t.begin(); it_pair != map_t.end(); ++it_pair)
            {
                serialize(it_pair->first);
                serialize(it_pair->second);
            }
        }
        catch (exception::Exception& ex)
        {
            set_state(dheader_state);
            ex.raise();
        }

        set_xcdrv2_dheader(dheader_state);

        return *this;
    }

    /*!
     * @brief This function template serializes a map of primitive.
     * @param map_t The map that will be serialized in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _K, class _T, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& serialize(
            const std::map<_K, _T>& map_t)
    {
        state state_(*this);

        serialize(static_cast<int32_t>(map_t.size()));

        try
        {
            for (auto it_pair = map_t.begin(); it_pair != map_t.end(); ++it_pair)
            {
                serialize(it_pair->first);
                serialize(it_pair->second);
            }
        }
        catch (exception::Exception& ex)
        {
            set_state(state_);
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief Encodes the value of a bitset into the buffer.
     *
     * @param[in] value A reference to the value which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<size_t N, typename std::enable_if < (N < 9) > ::type* = nullptr>
    Cdr& serialize(
            const std::bitset<N>& value)
    {
        return serialize(static_cast<uint8_t>(value.to_ulong()));
    }

    template<size_t N, typename std::enable_if < (8 < N && N < 17) > ::type* = nullptr>
    Cdr& serialize(
            const std::bitset<N>& value)
    {
        return serialize(static_cast<uint16_t>(value.to_ulong()));
    }

    template<size_t N, typename std::enable_if < (16 < N && N < 33) > ::type* = nullptr>
    Cdr& serialize(
            const std::bitset<N>& value)
    {
        return serialize(static_cast<uint32_t>(value.to_ulong()));
    }

    template<size_t N, typename std::enable_if < (32 < N && N < 65) > ::type* = nullptr>
    Cdr& serialize(
            const std::bitset<N>& value)
    {
        return serialize(static_cast<uint64_t>(value.to_ullong()));
    }

    /*!
     * @brief Encodes an array of a type not managed by this encoder into the buffer.
     *
     * To do that, the encoder expects a function `serialize` to be provided by the type.
     *
     * @param[in] value Array which will be encoded in the buffer.
     * @param[in] num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& serialize_array(
            const _T* value,
            size_t num_elements)
    {
        for (size_t count = 0; count < num_elements; ++count)
        {
            serialize(value[count]);
        }
        return *this;
    }

    /*!
     * @brief This function template serializes an array of non-basic objects with a different endianness.
     * @param type_t The array of objects that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @param endianness Endianness that will be used in the serialization of this value.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& serialize_array(
            const _T* type_t,
            size_t num_elements,
            Endianness endianness)
    {
        bool aux_swap = swap_bytes_;
        swap_bytes_ = (swap_bytes_ && (static_cast<Endianness>(endianness_) == endianness)) ||
                (!swap_bytes_ && (static_cast<Endianness>(endianness_) != endianness));

        try
        {
            serialize_array(type_t, num_elements);
            swap_bytes_ = aux_swap;
        }
        catch (exception::Exception& ex)
        {
            swap_bytes_ = aux_swap;
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief This function serializes an array of octets.
     * @param octet_t The sequence of octets that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize_array(
            const uint8_t* octet_t,
            size_t num_elements)
    {
        return serialize_array(reinterpret_cast<const char*>(octet_t), num_elements);
    }

    /*!
     * @brief This function serializes an array of characters.
     * @param char_t The array of characters that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize_array(
            const char* char_t,
            size_t num_elements);

    /*!
     * @brief This function serializes an array of int8_t.
     * @param int8 The sequence of int8_t that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize_array(
            const int8_t* int8,
            size_t num_elements)
    {
        return serialize_array(reinterpret_cast<const char*>(int8), num_elements);
    }

    /*!
     * @brief This function serializes an array of unsigned shorts.
     * @param ushort_t The array of unsigned shorts that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize_array(
            const uint16_t* ushort_t,
            size_t num_elements)
    {
        return serialize_array(reinterpret_cast<const int16_t*>(ushort_t), num_elements);
    }

    /*!
     * @brief This function serializes an array of shorts.
     * @param short_t The array of shorts that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize_array(
            const int16_t* short_t,
            size_t num_elements);

    /*!
     * @brief This function serializes an array of unsigned longs.
     * @param ulong_t The array of unsigned longs that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize_array(
            const uint32_t* ulong_t,
            size_t num_elements)
    {
        return serialize_array(reinterpret_cast<const int32_t*>(ulong_t), num_elements);
    }

    /*!
     * @brief This function serializes an array of longs.
     * @param long_t The array of longs that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize_array(
            const int32_t* long_t,
            size_t num_elements);

    /*!
     * @brief This function serializes an array of wide-chars.
     * @param wchar The array of wide-chars that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize_array(
            const wchar_t* wchar,
            size_t num_elements);

    /*!
     * @brief This function serializes an array of unsigned long longs.
     * @param ulonglong_t The array of unsigned long longs that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize_array(
            const uint64_t* ulonglong_t,
            size_t num_elements)
    {
        return serialize_array(reinterpret_cast<const int64_t*>(ulonglong_t), num_elements);
    }

    /*!
     * @brief This function serializes an array of long longs.
     * @param longlong_t The array of long longs that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize_array(
            const int64_t* longlong_t,
            size_t num_elements);

    /*!
     * @brief This function serializes an array of floats.
     * @param float_t The array of floats that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize_array(
            const float* float_t,
            size_t num_elements);

    /*!
     * @brief This function serializes an array of doubles.
     * @param double_t The array of doubles that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize_array(
            const double* double_t,
            size_t num_elements);

    /*!
     * @brief This function serializes an array of long doubles.
     * @param ldouble_t The array of long doubles that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     * @note Due to internal representation differences, WIN32 and *NIX like systems are not compatible.
     */
    Cdr_DllAPI Cdr& serialize_array(
            const long double* ldouble_t,
            size_t num_elements);

    /*!
     * @brief This function serializes an array of booleans.
     * @param bool_t The array of booleans that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& serialize_array(
            const bool* bool_t,
            size_t num_elements);

    /*!
     * @brief This function serializes an array of strings.
     * @param string_t The array of strings that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize_array(
            const std::string* string_t,
            size_t num_elements)
    {
        for (size_t count = 0; count < num_elements; ++count)
        {
            serialize(string_t[count].c_str());
        }
        return *this;
    }

    /*!
     * @brief This function serializes an array of wide-strings.
     * @param string_t The array of wide-strings that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize_array(
            const std::wstring* string_t,
            size_t num_elements)
    {
        for (size_t count = 0; count < num_elements; ++count)
        {
            serialize(string_t[count].c_str());
        }
        return *this;
    }

    /*!
     * @brief Encodes an array of fixed strings.
     * @param[in] value Array of fixed strings which will be encoded in the buffer.
     * @param[in] num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<size_t MAX_CHARS>
    Cdr& serialize_array(
            const fixed_string<MAX_CHARS>* value,
            size_t num_elements)
    {
        for (size_t count = 0; count < num_elements; ++count)
        {
            serialize(value[count].c_str());
        }
        return *this;
    }

    /*!
     * @brief Encodes an std::vector of primitives as an array.
     * @param[in] value Reference to a std::vector.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& serialize_array(
            const std::vector<_T, _Alloc>& value)
    {
        serialize_array(value.data(), value.size());

        return *this;
    }

    /*!
     * @brief Encodes an std::vector of non-primitives as an array.
     * @param[in] value Reference to a std::vector.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& serialize_array(
            const std::vector<_T, _Alloc>& value)
    {
        Cdr::state dheader_state {allocate_xcdrv2_dheader()};

        serialize_array(value.data(), value.size());

        set_xcdrv2_dheader(dheader_state);

        return *this;
    }

    /*!
     * @brief Encodes an std::vector as an array with a different endianness.
     * @param[in] value Reference to a std::vector.
     * @param[in] endianness Endianness that will be used in the serialization of this value.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc>
    Cdr& serialize_array(
            const std::vector<_T, _Alloc>& value,
            Endianness endianness)
    {
        bool aux_swap = swap_bytes_;
        swap_bytes_ = (swap_bytes_ && (static_cast<Endianness>(endianness_) == endianness)) ||
                (!swap_bytes_ && (static_cast<Endianness>(endianness_) != endianness));

        try
        {
            serialize_array(value);
            swap_bytes_ = aux_swap;
        }
        catch (exception::Exception& ex)
        {
            swap_bytes_ = aux_swap;
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief Encodes an std::vector of booleans as an array.
     * @param[in] value Reference to a std::vector.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& serialize_array(
            const std::vector<bool>& value)
    {
        serialize_bool_array(value);

        return *this;
    }

    /*!
     * @brief This function template serializes a raw sequence of non-primitives
     * @param sequence_t Pointer to the sequence that will be serialized in the buffer.
     * @param num_elements The number of elements contained in the sequence.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _T, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& serialize_sequence(
            const _T* sequence_t,
            size_t num_elements)
    {
        Cdr::state dheader_state {allocate_xcdrv2_dheader()};

        serialize(static_cast<int32_t>(num_elements));

        try
        {
            serialize_array(sequence_t, num_elements);
        }
        catch (exception::Exception& ex)
        {
            set_state(dheader_state);
            ex.raise();
        }

        set_xcdrv2_dheader(dheader_state);

        return *this;
    }

    /*!
     * @brief This function template serializes a raw sequence of primitives
     * @param sequence_t Pointer to the sequence that will be serialized in the buffer.
     * @param num_elements The number of elements contained in the sequence.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _T, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& serialize_sequence(
            const _T* sequence_t,
            size_t num_elements)
    {
        state state_before_error(*this);

        serialize(static_cast<int32_t>(num_elements));

        try
        {
            serialize_array(sequence_t, num_elements);
        }
        catch (exception::Exception& ex)
        {
            set_state(state_before_error);
            ex.raise();
        }

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            serialized_member_size_ = get_serialized_member_size<_T>();
        }

        return *this;
    }

    /*!
     * @brief This function template serializes a raw sequence with a different endianness.
     * @param sequence_t Pointer to the sequence that will be serialized in the buffer.
     * @param num_elements The number of elements contained in the sequence.
     * @param endianness Endianness that will be used in the serialization of this value.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& serialize_sequence(
            const _T* sequence_t,
            size_t num_elements,
            Endianness endianness)
    {
        bool aux_swap = swap_bytes_;
        swap_bytes_ = (swap_bytes_ && (static_cast<Endianness>(endianness_) == endianness)) ||
                (!swap_bytes_ && (static_cast<Endianness>(endianness_) != endianness));

        try
        {
            serialize_sequence(sequence_t, num_elements);
            swap_bytes_ = aux_swap;
        }
        catch (exception::Exception& ex)
        {
            swap_bytes_ = aux_swap;
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief Decodes the value of a type from the buffer.
     *
     * To do that, the encoder expects a function `deserialize` to be provided by the type.
     *
     * @param[out] value Reference to the variable where the value will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T, typename std::enable_if<!std::is_enum<_T>::value>::type* = nullptr, typename = void>
    Cdr& deserialize(
            _T& value)
    {
        eprosima::fastcdr::deserialize(*this, value);
        return *this;
    }

    /*!
     * @brief Decodes the value of a type with a different endianness.
     * @param[out] value Reference to the variable where the value will be stored after decoding from the buffer.
     * @param endianness Endianness that will be used in the deserialization of this value.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& deserialize(
            _T& value,
            Endianness endianness)
    {
        bool aux_swap = swap_bytes_;
        swap_bytes_ = (swap_bytes_ && (static_cast<Endianness>(endianness_) == endianness)) ||
                (!swap_bytes_ && (static_cast<Endianness>(endianness_) != endianness));

        try
        {
            deserialize(value);
            swap_bytes_ = aux_swap;
        }
        catch (exception::Exception& ex)
        {
            swap_bytes_ = aux_swap;
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief Decodes an enumeration from the buffer.
     * @param[out] value Reference to the variable where the enumeration will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            int32_t>::value>::type* = nullptr>
    Cdr& deserialize(
            _T& value)
    {
        int32_t decode_value {0};
        deserialize(decode_value);
        value = static_cast<_T>(decode_value);
        return *this;
    }

    /*!
     * @brief Decodes an enumeration from the buffer.
     * @param[out] value Reference to the variable where the enumeration will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            uint32_t>::value>::type* = nullptr>
    Cdr& deserialize(
            _T& value)
    {
        uint32_t decode_value {0};
        deserialize(decode_value);
        value = static_cast<_T>(decode_value);
        return *this;
    }

    /*!
     * @brief Decodes an enumeration from the buffer.
     * @param[out] value Reference to the variable where the enumeration will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            int16_t>::value>::type* = nullptr>
    Cdr& deserialize(
            _T& value)
    {
        int16_t decode_value {0};
        deserialize(decode_value);
        value = static_cast<_T>(decode_value);
        return *this;
    }

    /*!
     * @brief Decodes an enumeration from the buffer.
     * @param[out] value Reference to the variable where the enumeration will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            uint16_t>::value>::type* = nullptr>
    Cdr& deserialize(
            _T& value)
    {
        uint16_t decode_value {0};
        deserialize(decode_value);
        value = static_cast<_T>(decode_value);
        return *this;
    }

    /*!
     * @brief Decodes an enumeration from the buffer.
     * @param[out] value Reference to the variable where the enumeration will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            int8_t>::value>::type* = nullptr>
    Cdr& deserialize(
            _T& value)
    {
        int8_t decode_value {0};
        deserialize(decode_value);
        value = static_cast<_T>(decode_value);
        return *this;
    }

    /*!
     * @brief Decodes an enumeration from the buffer.
     * @param[out] value Reference to the variable where the enumeration will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T,
            typename std::enable_if<std::is_enum<_T>::value>::type* = nullptr,
            typename std::enable_if<std::is_same<typename std::underlying_type<_T>::type,
            uint8_t>::value>::type* = nullptr>
    Cdr& deserialize(
            _T& value)
    {
        uint8_t decode_value {0};
        deserialize(decode_value);
        value = static_cast<_T>(decode_value);
        return *this;
    }

    /*!
     * @brief This function deserializes an octet.
     * @param octet_t The variable that will store the octet read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize(
            uint8_t& octet_t)
    {
        return deserialize(reinterpret_cast<char&>(octet_t));
    }

    /*!
     * @brief This function deserializes a character.
     * @param char_t The variable that will store the character read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize(
            char& char_t);

    /*!
     * @brief This function deserializes an int8_t.
     * @param int8 The variable that will store the int8_t read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize(
            int8_t& int8)
    {
        return deserialize(reinterpret_cast<char&>(int8));
    }

    /*!
     * @brief This function deserializes an unsigned short.
     * @param ushort_t The variable that will store the unsigned short read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize(
            uint16_t& ushort_t)
    {
        return deserialize(reinterpret_cast<int16_t&>(ushort_t));
    }

    /*!
     * @brief This function deserializes a short.
     * @param short_t The variable that will store the short read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize(
            int16_t& short_t);

    /*!
     * @brief This function deserializes an unsigned long.
     * @param ulong_t The variable that will store the unsigned long read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize(
            uint32_t& ulong_t)
    {
        return deserialize(reinterpret_cast<int32_t&>(ulong_t));
    }

    /*!
     * @brief This function deserializes a long.
     * @param long_t The variable that will store the long read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize(
            int32_t& long_t);

    /*!
     * @brief This function deserializes a wide-char.
     * @param wchar The variable that will store the wide-char read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize(
            wchar_t& wchar)
    {
        uint16_t ret;
        deserialize(ret);
        wchar = static_cast<wchar_t>(ret);
        return *this;
    }

    /*!
     * @brief This function deserializes an unsigned long long.
     * @param ulonglong_t The variable that will store the unsigned long long read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize(
            uint64_t& ulonglong_t)
    {
        return deserialize(reinterpret_cast<int64_t&>(ulonglong_t));
    }

    /*!
     * @brief This function deserializes a long long.
     * @param longlong_t The variable that will store the long long read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize(
            int64_t& longlong_t);

    /*!
     * @brief This function deserializes a float.
     * @param float_t The variable that will store the float read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize(
            float& float_t);

    /*!
     * @brief This function deserializes a double.
     * @param double_t The variable that will store the double read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize(
            double& double_t);

    /*!
     * @brief This function deserializes a long double.
     * @param ldouble_t The variable that will store the long double read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     * @note Due to internal representation differences, WIN32 and *NIX like systems are not compatible.
     */
    Cdr_DllAPI Cdr& deserialize(
            long double& ldouble_t);

    /*!
     * @brief This function deserializes a boolean.
     * @param bool_t The variable that will store the boolean read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to deserialize an invalid value.
     */
    Cdr_DllAPI Cdr& deserialize(
            bool& bool_t);

    /*!
     * @brief This function deserializes a string.
     * This function allocates memory to store the string. The user pointer will be set to point this allocated memory.
     * The user will have to free this allocated memory using free()
     * @param string_t The pointer that will point to the string read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize(
            char*& string_t);

    /*!
     * @brief This function deserializes a wide-string.
     * This function allocates memory to store the wide string. The user pointer will be set to point this allocated memory.
     * The user will have to free this allocated memory using free()
     * @param string_t The pointer that will point to the wide string read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize(
            wchar_t*& string_t);

    /*!
     * @brief This function deserializes a std::string.
     * @param string_t The variable that will store the string read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize(
            std::string& string_t)
    {
        uint32_t length = 0;
        const char* str = read_string(length);
        string_t.assign(str, length);
        return *this;
    }

    /*!
     * @brief This function deserializes a std::wstring.
     * @param string_t The variable that will store the string read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize(
            std::wstring& string_t)
    {
        uint32_t length = 0;
        string_t = read_wstring(length);
        return *this;
    }

    /*!
     * @brief Decodes a fixed string.
     * @param[out] value Reference to the variable where the fixed string will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template <size_t MAX_CHARS>
    Cdr& deserialize(
            fixed_string<MAX_CHARS>& value)
    {
        uint32_t length = 0;
        const char* str = read_string(length);
        value.assign(str, length);
        return *this;
    }

    /*!
     * @brief This function template deserializes an array.
     * @param array_t The variable that will store the array read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _T, size_t _Size>
    Cdr& deserialize(
            std::array<_T, _Size>& array_t)
    {
        if (CdrVersion::XCDRv2 == cdr_version_ && !is_multi_array_primitive(&array_t))
        {
            uint32_t dheader {0};
            deserialize(dheader);

            uint32_t count {0};
            auto offset = offset_;
            while (offset_ - offset < dheader && count < _Size)
            {
                deserialize_array(&array_t.data()[count], 1);
                ++count;
            }

            if (offset_ - offset != dheader)
            {
                throw exception::BadParamException("Member size greater than size specified by DHEADER");
            }
        }
        else
        {
            return deserialize_array(array_t.data(), array_t.size());
        }

        return *this;
    }

    /*!
     * @brief This function template deserializes a sequence of non-primitive.
     * @param vector_t The variable that will store the sequence read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& deserialize(
            std::vector<_T, _Alloc>& vector_t)
    {
        uint32_t sequence_length {0};

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            uint32_t dheader {0};
            deserialize(dheader);

            auto offset = offset_;

            deserialize(sequence_length);

            if (0 == sequence_length)
            {
                vector_t.clear();
                return *this;
            }
            else
            {
                vector_t.resize(sequence_length);
            }

            uint32_t count {0};
            while (offset_ - offset < dheader && count < sequence_length)
            {
                deserialize(vector_t.data()[count]);
                ++count;
            }

            if (offset_ - offset != dheader)
            {
                throw exception::BadParamException("Member size differs from the size specified by DHEADER");
            }
        }
        else
        {
            state state_before_error(*this);

            deserialize(sequence_length);

            if (sequence_length == 0)
            {
                vector_t.clear();
                return *this;
            }

            if ((end_ - offset_) < sequence_length)
            {
                set_state(state_before_error);
                throw exception::NotEnoughMemoryException(
                          exception::NotEnoughMemoryException::NOT_ENOUGH_MEMORY_MESSAGE_DEFAULT);
            }

            try
            {
                vector_t.resize(sequence_length);
                return deserialize_array(vector_t.data(), vector_t.size());
            }
            catch (exception::Exception& ex)
            {
                set_state(state_before_error);
                ex.raise();
            }
        }

        return *this;
    }

    /*!
     * @brief This function template deserializes a sequence of primitive.
     * @param vector_t The variable that will store the sequence read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& deserialize(
            std::vector<_T, _Alloc>& vector_t)
    {
        uint32_t sequence_length = 0;
        state state_before_error(*this);

        deserialize(sequence_length);

        if (sequence_length == 0)
        {
            vector_t.clear();
            return *this;
        }

        if ((end_ - offset_) < sequence_length)
        {
            set_state(state_before_error);
            throw exception::NotEnoughMemoryException(
                      exception::NotEnoughMemoryException::NOT_ENOUGH_MEMORY_MESSAGE_DEFAULT);
        }

        try
        {
            vector_t.resize(sequence_length);
            return deserialize_array(vector_t.data(), vector_t.size());
        }
        catch (exception::Exception& ex)
        {
            set_state(state_before_error);
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief This function template deserializes a sequence.
     * @param vector_t The variable that will store the sequence read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize(
            std::vector<bool>& vector_t)
    {
        return deserialize_bool_sequence(vector_t);
    }

    /*!
     * @brief This function template deserializes a map of non-primitive.
     * @param map_t The variable that will store the map read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _K, class _T, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& deserialize(
            std::map<_K, _T>& map_t)
    {
        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            uint32_t dheader {0};
            deserialize(dheader);

            auto offset = offset_;

            uint32_t map_length {0};
            deserialize(map_length);

            map_t.clear();

            uint32_t count {0};
            while (offset_ - offset < dheader && count < map_length)
            {
                _K key;
                _T val;
                deserialize(key);
                deserialize(val);
                map_t.emplace(std::pair<_K, _T>(std::move(key), std::move(val)));
                ++count;
            }

            if (offset_ - offset != dheader)
            {
                throw exception::BadParamException("Member size greater than size specified by DHEADER");
            }
        }
        else
        {
            uint32_t sequence_length = 0;
            state state_(*this);

            deserialize(sequence_length);

            map_t.clear();

            try
            {
                for (uint32_t i = 0; i < sequence_length; ++i)
                {
                    _K key;
                    _T value;
                    deserialize(key);
                    deserialize(value);
                    map_t.emplace(std::pair<_K, _T>(std::move(key), std::move(value)));
                }
            }
            catch (exception::Exception& ex)
            {
                set_state(state_);
                ex.raise();
            }
        }

        return *this;
    }

    /*!
     * @brief This function template deserializes a map of primitive.
     * @param map_t The variable that will store the map read from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _K, class _T, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& deserialize(
            std::map<_K, _T>& map_t)
    {
        uint32_t sequence_length = 0;
        state state_(*this);

        deserialize(sequence_length);

        try
        {
            for (uint32_t i = 0; i < sequence_length; ++i)
            {
                _K key;
                _T value;
                deserialize(key);
                deserialize(value);
                map_t.emplace(std::pair<_K, _T>(std::move(key), std::move(value)));
            }
        }
        catch (exception::Exception& ex)
        {
            set_state(state_);
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief Decodes a bitset from the buffer.
     * @param[out] value Reference to the variable where the bitset will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<size_t N, typename std::enable_if < (N < 9) > ::type* = nullptr>
    Cdr& deserialize(
            std::bitset<N>& value)
    {
        uint8_t decode_value {0};
        deserialize(decode_value);
        value = decode_value;
        return *this;
    }

    template<size_t N, typename std::enable_if < (8 < N && N < 17) > ::type* = nullptr>
    Cdr& deserialize(
            std::bitset<N>& value)
    {
        uint16_t decode_value {0};
        deserialize(decode_value);
        value = decode_value;
        return *this;
    }

    template<size_t N, typename std::enable_if < (16 < N && N < 33) > ::type* = nullptr>
    Cdr& deserialize(
            std::bitset<N>& value)
    {
        uint32_t decode_value {0};
        deserialize(decode_value);
        value = decode_value;
        return *this;
    }

    template<size_t N, typename std::enable_if < (32 < N && N < 65) > ::type* = nullptr>
    Cdr& deserialize(
            std::bitset<N>& value)
    {
        uint64_t decode_value {0};
        deserialize(decode_value);
        value = decode_value;
        return *this;
    }

    /*!
     * @brief Decodes an array of a type not managed by this encoder from the buffer.
     *
     * To do that, the encoder expects a function `deserialize` to be provided by the type.
     *
     * @param[out] value Reference to the variable where the array will be stored after decoding from the buffer.
     * @param[in] num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& deserialize_array(
            _T* value,
            size_t num_elements)
    {
        for (size_t count = 0; count < num_elements; ++count)
        {
            deserialize(value[count]);
        }
        return *this;
    }

    /*!
     * @brief This function template deserializes an array of non-basic objects with a different endianness.
     * @param type_t The variable that will store the array of objects read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @param endianness Endianness that will be used in the deserialization of this value.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& deserialize_array(
            _T* type_t,
            size_t num_elements,
            Endianness endianness)
    {
        bool aux_swap = swap_bytes_;
        swap_bytes_ = (swap_bytes_ && (static_cast<Endianness>(endianness_) == endianness)) ||
                (!swap_bytes_ && (static_cast<Endianness>(endianness_) != endianness));

        try
        {
            deserialize_array(type_t, num_elements);
            swap_bytes_ = aux_swap;
        }
        catch (exception::Exception& ex)
        {
            swap_bytes_ = aux_swap;
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief This function deserializes an array of octets.
     * @param octet_t The variable that will store the array of octets read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize_array(
            uint8_t* octet_t,
            size_t num_elements)
    {
        return deserialize_array(reinterpret_cast<char*>(octet_t), num_elements);
    }

    /*!
     * @brief This function deserializes an array of characters.
     * @param char_t The variable that will store the array of characters read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize_array(
            char* char_t,
            size_t num_elements);

    /*!
     * @brief This function deserializes an array of int8_t.
     * @param int8 The variable that will store the array of int8_t read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize_array(
            int8_t* int8,
            size_t num_elements)
    {
        return deserialize_array(reinterpret_cast<char*>(int8), num_elements);
    }

    /*!
     * @brief This function deserializes an array of unsigned shorts.
     * @param ushort_t The variable that will store the array of unsigned shorts read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize_array(
            uint16_t* ushort_t,
            size_t num_elements)
    {
        return deserialize_array(reinterpret_cast<int16_t*>(ushort_t), num_elements);
    }

    /*!
     * @brief This function deserializes an array of shorts.
     * @param short_t The variable that will store the array of shorts read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize_array(
            int16_t* short_t,
            size_t num_elements);

    /*!
     * @brief This function deserializes an array of unsigned longs.
     * @param ulong_t The variable that will store the array of unsigned longs read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize_array(
            uint32_t* ulong_t,
            size_t num_elements)
    {
        return deserialize_array(reinterpret_cast<int32_t*>(ulong_t), num_elements);
    }

    /*!
     * @brief This function deserializes an array of longs.
     * @param long_t The variable that will store the array of longs read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize_array(
            int32_t* long_t,
            size_t num_elements);

    /*!
     * @brief This function deserializes an array of wide-chars.
     * @param wchar The variable that will store the array of wide-chars read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize_array(
            wchar_t* wchar,
            size_t num_elements);

    /*!
     * @brief This function deserializes an array of unsigned long longs.
     * @param ulonglong_t The variable that will store the array of unsigned long longs read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize_array(
            uint64_t* ulonglong_t,
            size_t num_elements)
    {
        return deserialize_array(reinterpret_cast<int64_t*>(ulonglong_t), num_elements);
    }

    /*!
     * @brief This function deserializes an array of long longs.
     * @param longlong_t The variable that will store the array of long longs read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize_array(
            int64_t* longlong_t,
            size_t num_elements);

    /*!
     * @brief This function deserializes an array of floats.
     * @param float_t The variable that will store the array of floats read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize_array(
            float* float_t,
            size_t num_elements);

    /*!
     * @brief This function deserializes an array of doubles.
     * @param double_t The variable that will store the array of doubles read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize_array(
            double* double_t,
            size_t num_elements);

    /*!
     * @brief This function deserializes an array of long doubles.
     * @param ldouble_t The variable that will store the array of long doubles read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     * @note Due to internal representation differences, WIN32 and *NIX like systems are not compatible.
     */
    Cdr_DllAPI Cdr& deserialize_array(
            long double* ldouble_t,
            size_t num_elements);

    /*!
     * @brief This function deserializes an array of booleans.
     * @param bool_t The variable that will store the array of booleans read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize_array(
            bool* bool_t,
            size_t num_elements);

    /*!
     * @brief Decodes an array of primitives on a std::vector.
     *
     * std::vector must have allocated the number of element of the array.
     *
     * @param[out] value Reference to the std::vector where the array will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& deserialize_array(
            std::vector<_T, _Alloc>& value)
    {
        deserialize_array(value.data(), value.size());

        return *this;
    }

    /*!
     * @brief Decodes an array of non-primitives on a std::vector.
     *
     * std::vector must have allocated the number of element of the array.
     *
     * @param[out] value Reference to the std::vector where the array will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& deserialize_array(
            std::vector<_T, _Alloc>& value)
    {
        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            uint32_t dheader {0};
            deserialize(dheader);

            uint32_t count {0};
            auto offset = offset_;
            while (offset_ - offset < dheader && count < value.size())
            {
                deserialize_array(&value.data()[count], 1);
                ++count;
            }

            if (offset_ - offset != dheader)
            {
                throw exception::BadParamException("Member size greater than size specified by DHEADER");
            }
        }
        else
        {
            return deserialize_array(value.data(), value.size());
        }

        return *this;
    }

    /*!
     * @brief Decodes an array of non-primitives on a std::vector with a different endianness.
     *
     * std::vector must have allocated the number of element of the array.
     *
     * @param[out] value Reference to the std::vector where the array will be stored after decoding from the buffer.
     * @param[in] endianness Endianness that will be used in the serialization of this value.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T, class _Alloc>
    Cdr& deserialize_array(
            std::vector<_T, _Alloc>& value,
            Endianness endianness)
    {
        bool aux_swap = swap_bytes_;
        swap_bytes_ = (swap_bytes_ && (static_cast<Endianness>(endianness_) == endianness)) ||
                (!swap_bytes_ && (static_cast<Endianness>(endianness_) != endianness));

        try
        {
            deserialize_array(value);
            swap_bytes_ = aux_swap;
        }
        catch (exception::Exception& ex)
        {
            swap_bytes_ = aux_swap;
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief Decodes an array of booleans on a std::vector.
     *
     * std::vector must have allocated the number of element of the array.
     *
     * @param[out] value Reference to the std::vector where the array will be stored after decoding from the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize_array(
            std::vector<bool>& value)
    {
        deserialize_bool_array(value);

        return *this;
    }

    /*!
     * @brief This function template deserializes a raw sequence of non-primitives.
     * This function allocates memory to store the sequence. The user pointer will be set to point this allocated memory.
     * The user will have to free this allocated memory using free()
     * @param sequence_t The pointer that will store the sequence read from the buffer.
     * @param num_elements This variable return the number of elements of the sequence.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _T, typename std::enable_if<!std::is_enum<_T>::value &&
            !std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& deserialize_sequence(
            _T*& sequence_t,
            size_t& num_elements)
    {
        uint32_t sequence_length {0};

        if (CdrVersion::XCDRv2 == cdr_version_)
        {
            uint32_t dheader {0};
            deserialize(dheader);

            auto offset = offset_;

            deserialize(sequence_length);

            try
            {
                sequence_t = reinterpret_cast<_T*>(calloc(sequence_length, sizeof(_T)));

                uint32_t count {0};
                while (offset_ - offset < dheader && count < sequence_length)
                {
                    deserialize(sequence_t[count]);
                    ++count;
                }

                if (offset_ - offset != dheader)
                {
                    throw exception::BadParamException("Member size greater than size specified by DHEADER");
                }
            }
            catch (exception::Exception& ex)
            {
                free(sequence_t);
                sequence_t = NULL;
                ex.raise();
            }
        }
        else
        {
            state state_before_error(*this);

            deserialize(sequence_length);

            if ((end_ - offset_) < sequence_length)
            {
                set_state(state_before_error);
                throw exception::NotEnoughMemoryException(
                          exception::NotEnoughMemoryException::NOT_ENOUGH_MEMORY_MESSAGE_DEFAULT);
            }

            try
            {
                sequence_t = reinterpret_cast<_T*>(calloc(sequence_length, sizeof(_T)));
                deserialize_array(sequence_t, sequence_length);
            }
            catch (exception::Exception& ex)
            {
                free(sequence_t);
                sequence_t = NULL;
                set_state(state_before_error);
                ex.raise();
            }
        }

        num_elements = sequence_length;
        return *this;
    }

    /*!
     * @brief This function template deserializes a raw sequence of primitives.
     * This function allocates memory to store the sequence. The user pointer will be set to point this allocated memory.
     * The user will have to free this allocated memory using free()
     * @param sequence_t The pointer that will store the sequence read from the buffer.
     * @param num_elements This variable return the number of elements of the sequence.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _T, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    Cdr& deserialize_sequence(
            _T*& sequence_t,
            size_t& num_elements)
    {
        uint32_t sequence_length = 0;
        state state_before_error(*this);

        deserialize(sequence_length);

        try
        {
            sequence_t = reinterpret_cast<_T*>(calloc(sequence_length, sizeof(_T)));
            deserialize_array(sequence_t, sequence_length);
        }
        catch (exception::Exception& ex)
        {
            free(sequence_t);
            sequence_t = NULL;
            set_state(state_before_error);
            ex.raise();
        }

        num_elements = sequence_length;
        return *this;
    }

    /*!
     * @brief This function template deserializes a raw sequence with a different endianness.
     * This function allocates memory to store the sequence. The user pointer will be set to point this allocated memory.
     * The user will have to free this allocated memory using free()
     * @param sequence_t The pointer that will store the sequence read from the buffer.
     * @param num_elements This variable return the number of elements of the sequence.
     * @param endianness Endianness that will be used in the deserialization of this value.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& deserialize_sequence(
            _T*& sequence_t,
            size_t& num_elements,
            Endianness endianness)
    {
        bool aux_swap = swap_bytes_;
        swap_bytes_ = (swap_bytes_ && (static_cast<Endianness>(endianness_) == endianness)) ||
                (!swap_bytes_ && (static_cast<Endianness>(endianness_) != endianness));

        try
        {
            deserialize_sequence(sequence_t, num_elements);
            swap_bytes_ = aux_swap;
        }
        catch (exception::Exception& ex)
        {
            swap_bytes_ = aux_swap;
            ex.raise();
        }

        return *this;
    }

    /*!
     * @brief This function template deserializes a string sequence.
     * This function allocates memory to store the sequence. The user pointer will be set to point this allocated memory.
     * The user will have to free this allocated memory using free()
     * @param sequence_t The pointer that will store the sequence read from the buffer.
     * @param num_elements This variable return the number of elements of the sequence.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize_sequence(
            std::string*& sequence_t,
            size_t& num_elements)
    {
        return deserialize_string_sequence(sequence_t, num_elements);
    }

    /*!
     * @brief This function template deserializes a wide-string sequence.
     * This function allocates memory to store the sequence. The user pointer will be set to point this allocated memory.
     * The user will have to free this allocated memory using free()
     * @param sequence_t The pointer that will store the sequence read from the buffer.
     * @param num_elements This variable return the number of elements of the sequence.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    TEMPLATE_SPEC
    Cdr& deserialize_sequence(
            std::wstring*& sequence_t,
            size_t& num_elements)
    {
        return deserialize_wstring_sequence(sequence_t, num_elements);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// XCDR extensions
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /*!
     * @brief Encodes a member of a type according to the encoding algorithm used.
     * @param[in] member_id Member identifier.
     * @param[in] member_value Member value.
     * @param[in] header_selection Selects which member header will be used to allocate space.
     * Default: XCdrHeaderSelection::AUTO_WITH_SHORT_HEADER_BY_DEFAULT.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& serialize_member(
            const MemberId& member_id,
            const _T& member_value,
            XCdrHeaderSelection header_selection = XCdrHeaderSelection::AUTO_WITH_SHORT_HEADER_BY_DEFAULT)
    {
        Cdr::state current_state(*this);
        (this->*begin_serialize_member_)(member_id, true, current_state, header_selection);
        serialize(member_value);
        return (this->*end_serialize_member_)(current_state);
    }

    /*!
     * @brief Encodes an optional member of a type according to the encoding algorithm used.
     * @param[in] member_id Member identifier.
     * @param[in] member_value Optional member value.
     * @param[in] header_selection Selects which member header will be used to allocate space.
     * Default: XCdrHeaderSelection::AUTO_WITH_SHORT_HEADER_BY_DEFAULT.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& serialize_member(
            const MemberId& member_id,
            const optional<_T>& member_value,
            XCdrHeaderSelection header_selection = XCdrHeaderSelection::AUTO_WITH_SHORT_HEADER_BY_DEFAULT)
    {
        Cdr::state current_state(*this);
        (this->*begin_serialize_opt_member_)(member_id, member_value.has_value(), current_state, header_selection);
        serialize(member_value);
        return (this->*end_serialize_opt_member_)(current_state);
    }

    /*!
     * @brief Decodes a member of a type according to the encoding algorithm used.
     * @param[out] member_value A reference of the variable where the member value will be stored.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& deserialize_member(
            _T& member_value)
    {
        return deserialize(member_value);
    }

    /*!
     * @brief Decodes an optional member of a type according to the encoding algorithm used.
     * @param[out] member_value A reference of the variable where the optional member value will be stored.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& deserialize_member(
            optional<_T>& member_value)
    {
        if (EncodingAlgorithmFlag::PLAIN_CDR == current_encoding_)
        {
            Cdr::state current_state(*this);
            MemberId member_id;
            xcdr1_deserialize_member_header(member_id, current_state);
            auto prev_offset = offset_;
            if (0 < current_state.member_size_)
            {
                deserialize(member_value);
            }
            size_t member_size {current_state.member_size_};
            size_t diff {offset_ - prev_offset};
            if (member_size < diff)
            {
                throw exception::BadParamException(
                          "Member size provided by member header is lower than real decoded member size");
            }

            // Skip unused bytes
            offset_ += (member_size - diff);
        }
        else
        {
            deserialize(member_value);
        }
        return *this;
    }

    /*!
     * @brief Tells to the encoder a new type and its members starts to be encoded.
     * @param[in,out] current_state State of the encoder previous of calling this function.
     * @param[in] type_encoding The encoding algorithm used to encode the type and its members.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& begin_serialize_type(
            Cdr::state& current_state,
            EncodingAlgorithmFlag type_encoding);

    /*!
     * @brief Tells to the encoder the encoding of the type finishes.
     * @param[in] current_state State of the encoder previous of calling the function begin_serialize_type.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& end_serialize_type(
            Cdr::state& current_state);

    /*!
     * @brief Tells to the encoder a new type and its members starts to be decoded.
     * @param[in] type_encoding The encoding algorithm used to decode the type and its members.
     * @param[in] functor Functor called each time a member has to be decoded.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    Cdr_DllAPI Cdr& deserialize_type(
            EncodingAlgorithmFlag type_encoding,
            std::function<bool (Cdr&, const MemberId&)> functor);

    /*!
     * @brief Encodes an optional in the buffer.
     * @param[in] value A reference to the optional which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& serialize(
            const optional<_T>& value)
    {
        if (CdrVersion::XCDRv2 == cdr_version_ && EncodingAlgorithmFlag::PL_CDR2 != current_encoding_)
        {
            serialize(value.has_value());
        }

        if (value.has_value())
        {
            serialize(*value);
        }
        return *this;
    }

    /*!
     * @brief Encodes an external in the buffer.
     * @param[in] value A reference to the external which will be encoded in the buffer.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::BadParamException This exception is thrown when external is null.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& serialize(
            const external<_T>& value)
    {
        if (!value)
        {
            throw exception::BadParamException("External member is null");
        }

        serialize(*value);
        return *this;
    }

    /*!
     * @brief Tells the encoder the member identifier for the next member to be encoded.
     * @param[in] member_id Member identifier.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::BadParamException This exception is thrown when a member id is already set without being
     * encoded.
     */
    Cdr_DllAPI Cdr& operator <<(
            const MemberId& member_id);

    /*!
     * @brief Decodes an optional from the buffer.
     * @param[out] value A reference to the variable where the optional will be stored.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& deserialize(
            optional<_T>& value)
    {
        bool is_present = true;
        if (CdrVersion::XCDRv2 == cdr_version_ && EncodingAlgorithmFlag::PL_CDR2 != current_encoding_)
        {
            deserialize(is_present);
        }
        value.reset(is_present);
        if (is_present)
        {
            deserialize(*value);
        }
        return *this;
    }

    /*!
     * @brief Decodes an external from the buffer.
     * @param[out] value A reference to the variable where the external will be stored.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::BadParamException This exception is thrown when the external is locked.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& deserialize(
            external<_T>& value)
    {
        if (value.is_locked())
        {
            throw exception::BadParamException("External member is locked");
        }

        if (!value)
        {
            value = external<_T>{new typename external<_T>::type()};
        }

        deserialize(*value);
        return *this;
    }

    /*!
     * @brief Decodes an optional of an external from the buffer.
     * @param[out] value A reference to the variable where the optional will be stored.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::BadParamException This exception is thrown when the external is locked.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     */
    template<class _T>
    Cdr& deserialize(
            optional<external<_T>>& value)
    {
        if (value.has_value() && value.value().is_locked())
        {
            throw exception::BadParamException("External member is locked");
        }

        bool is_present = true;
        if (CdrVersion::XCDRv2 == cdr_version_ && EncodingAlgorithmFlag::PL_CDR2 != current_encoding_)
        {
            deserialize(is_present);
        }
        value.reset(is_present);
        if (is_present)
        {
            deserialize(*value);
        }
        return *this;
    }

    /*!
     * @brief Encodes an empty DHEADER if the encoding version is XCDRv2.
     * After serializing the members's type, @ref set_xcdrv2_dheader must be called to set the correct DHEADER value
     * using the @ref state returned by this function.
     */
    Cdr_DllAPI state allocate_xcdrv2_dheader();

    /*!
     * @brief Uses the @ref state to calculate the member's type size and serialize the value in the previous allocated
     * DHEADER.
     *
     * @param[in] state @ref state used to calculate the member's type size.
     */
    Cdr_DllAPI void set_xcdrv2_dheader(
            const state& state);

private:

    Cdr(
            const Cdr&) = delete;

    Cdr& operator =(
            const Cdr&) = delete;

    Cdr_DllAPI Cdr& serialize_bool_array(
            const std::vector<bool>& vector_t);

    Cdr_DllAPI Cdr& serialize_bool_sequence(
            const std::vector<bool>& vector_t);

    Cdr_DllAPI Cdr& deserialize_bool_array(
            std::vector<bool>& vector_t);

    Cdr_DllAPI Cdr& deserialize_bool_sequence(
            std::vector<bool>& vector_t);

    Cdr_DllAPI Cdr& deserialize_string_sequence(
            std::string*& sequence_t,
            size_t& num_elements);

    Cdr_DllAPI Cdr& deserialize_wstring_sequence(
            std::wstring*& sequence_t,
            size_t& num_elements);

    /*!
     * @brief This function template detects the content type of the STD container array and serializes the array.
     * @param array_t The array that will be serialized in the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to serialize a position that exceeds the internal memory size.
     */
    template<class _T, size_t _Size>
    Cdr& serialize_array(
            const std::array<_T, _Size>* array_t,
            size_t num_elements)
    {
        return serialize_array(array_t->data(), num_elements * array_t->size());
    }

    /*!
     * @brief This function template detects the content type of the STD container array and deserializes the array.
     * @param array_t The variable that will store the array read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _T, size_t _Size>
    Cdr& deserialize_array(
            std::array<_T, _Size>* array_t,
            size_t num_elements)
    {
        return deserialize_array(array_t->data(), num_elements * array_t->size());
    }

    /*!
     * @brief This function template detects the content type of STD container array and deserializes the array with a different endianness.
     * @param array_t The variable that will store the array read from the buffer.
     * @param num_elements Number of the elements in the array.
     * @param endianness Endianness that will be used in the deserialization of this value.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to deserialize a position that exceeds the internal memory size.
     */
    template<class _T, size_t _Size>
    Cdr& deserialize_array(
            std::array<_T, _Size>* array_t,
            size_t num_elements,
            Endianness endianness)
    {
        return deserialize_array(array_t->data(), num_elements * array_t->size(), endianness);
    }

    /*!
     * @brief Returns the number of bytes needed to align the current position (having as reference the origin) to
     * certain data size.
     * @param data_size The size of the data that will be serialized.
     * @return The size needed for the alignment.
     */
    inline size_t alignment(
            size_t data_size) const
    {
        return data_size > last_data_size_ ? (data_size - ((offset_ - origin_) % data_size)) & (data_size - 1) : 0;
    }

    /*!
     * @brief This function jumps the number of bytes of the alignment. These bytes should be calculated with the function eprosima::fastcdr::Cdr::alignment.
     * @param align The number of bytes to be skipped.
     */
    inline void make_alignment(
            size_t align)
    {
        offset_ += align;
        last_data_size_ = 0;
    }

    /*!
     * @brief This function resizes the internal buffer. It only applies if the FastBuffer object was created with the default constructor.
     * @param min_size_inc Minimun size increase for the internal buffer
     * @return True if the resize was succesful, false if it was not
     */
    bool resize(
            size_t min_size_inc);

    Cdr_DllAPI const char* read_string(
            uint32_t& length);
    Cdr_DllAPI const std::wstring read_wstring(
            uint32_t& length);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// XCDR extensions
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /*!
     * @brief Encodes a short member header of a member according to XCDRv1.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x3F00.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr1_serialize_short_member_header(
            const MemberId& member_id);

    /*!
     * @brief Finish the encoding of a short member header of a member according to XCDRv1.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x3F00.
     * @param[in] member_serialized_size Size of the serialized member.
     * @pre Serialized size equal or less than 0xFFFF.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr1_end_short_member_header(
            const MemberId& member_id,
            size_t member_serialized_size);

    /*!
     * @brief Encodes a long member header of a member according to XCDRv1.
     * @param[in] member_id Member identifier.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr1_serialize_long_member_header(
            const MemberId& member_id);

    /*!
     * @brief Finish the encoding of a long member header of a member according to XCDRv1.
     * @param[in] member_id Member identifier.
     * @param[in] member_serialized_size Size of the serialized member.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr1_end_long_member_header(
            const MemberId& member_id,
            size_t member_serialized_size);

    /*!
     * @brief Changes the previous encoded long header to a short header according to XCDRv1.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x3F00.
     * @param[in] member_serialized_size Size of the serialized member.
     * @pre Serialized size equal or less than 0xFFFF.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr1_change_to_short_member_header(
            const MemberId& member_id,
            size_t member_serialized_size);

    /*!
     * @brief Changes the previous encoded short header to a long header according to XCDRv1.
     * @param[in] member_id Member identifier.
     * @param[in] member_serialized_size Size of the serialized member.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr1_change_to_long_member_header(
            const MemberId& member_id,
            size_t member_serialized_size);

    /*!
     * @brief Decodes a member header according to XCDRv1.
     * @param[out] member_id Member identifier.
     * @param[in,out] current_state State of the encoder previous to call this function.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to decode an invalid value.
     */
    Cdr_DllAPI bool xcdr1_deserialize_member_header(
            MemberId& member_id,
            Cdr::state& current_state);

    /*!
     * @brief Encodes a short member header of a member according to XCDRv2.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x10000000.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr2_serialize_short_member_header(
            const MemberId& member_id);

    /*!
     * @brief Finish the encoding of a short member header of a member according to XCDRv2.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x10000000.
     * @param[in] member_serialized_size Size of the serialized member.
     * @pre Serialized size equal or less than 0x8.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr2_end_short_member_header(
            const MemberId& member_id,
            size_t member_serialized_size);

    /*!
     * @brief Encodes a long member header of a member according to XCDRv2.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x10000000.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr2_serialize_long_member_header(
            const MemberId& member_id);

    /*!
     * @brief Finish the encoding of a long member header of a member according to XCDRv2.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x10000000.
     * @param[in] member_serialized_size Size of the serialized member.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr2_end_long_member_header(
            const MemberId& member_id,
            size_t member_serialized_size);

    /*!
     * @brief Changes the previous encoded long header to a short header according to XCDRv2.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x10000000.
     * @param[in] member_serialized_size Size of the serialized member.
     * @pre Serialized size equal or less than 8.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr2_change_to_short_member_header(
            const MemberId& member_id,
            size_t member_serialized_size);

    /*!
     * @brief Changes the previous encoded long header to a short header according to XCDRv2.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x10000000.
     * @param[in] member_serialized_size Size of the serialized member.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr2_change_to_long_member_header(
            const MemberId& member_id,
            size_t member_serialized_size);

    /*!
     * @brief Join the previous encoded long header with the next DHEADER which was serialized after.
     * @param[in] member_id Member identifier.
     * @pre Member identifier less than 0x10000000.
     * @param[in] offset The last offset of the buffer previous to call this function.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    void xcdr2_shrink_to_long_member_header(
            const MemberId& member_id,
            const FastBuffer::iterator& offset);

    /*!
     * @brief Decodes a member header according to XCDRv2.
     * @param[out] member_id Member identifier.
     * @param[in,out] current_state State of the encoder previous to call this function.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to decode from a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to decode an invalid value.
     */
    void xcdr2_deserialize_member_header(
            MemberId& member_id,
            Cdr::state& current_state);

    /*!
     * @brief Tells to the encoder a member starts to be encoded according to XCDRv1.
     * @param[in] member_id Member identifier.
     * @pre Member identifier cannot be MEMBER_ID_INVALID and next_member_id_ must be equal to the member identifier or
     * MEMBER_ID_INVALID.
     * @param[in] is_present If the member is present.
     * @pre When XCDRv1, is_present must be always true.
     * @param[in,out] current_state State of the encoder previous to call this function.
     * @pre Current encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR or EncodingAlgorithmFlag::PL_CDR.
     * @param[in] header_selection Selects which member header will be used to allocate space.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to encode a long header when
     * header_selection is XCdrHeaderSelection::SHORT_HEADER.
     */
    Cdr& xcdr1_begin_serialize_member(
            const MemberId& member_id,
            bool is_present,
            Cdr::state& current_state,
            XCdrHeaderSelection header_selection);

    /*!
     * @brief Tells to the encoder to finish the encoding of the member.
     * @param[in] current_state State of the encoder previous to call xcdr1_begin_serialize_member function.
     * @pre next_member_id_ cannot be MEMBER_ID_INVALID.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to encode a long header when
     * header_selection is XCdrHeaderSelection::SHORT_HEADER.
     */
    Cdr& xcdr1_end_serialize_member(
            const Cdr::state& current_state);

    /*!
     * @brief Tells to the encoder a member starts to be encoded according to XCDRv1.
     * @param[in] member_id Member identifier.
     * @pre Member identifier cannot be MEMBER_ID_INVALID and next_member_id_ must be equal to the member identifier or
     * MEMBER_ID_INVALID.
     * @param[in] is_present If the member is present.
     * @param[in,out] current_state State of the encoder previous to call this function.
     * @pre Current encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR or EncodingAlgorithmFlag::PL_CDR.
     * @param[in] header_selection Selects which member header will be used to allocate space.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to encode a long header when
     * header_selection is XCdrHeaderSelection::SHORT_HEADER.
     */
    Cdr& xcdr1_begin_serialize_opt_member(
            const MemberId& member_id,
            bool is_present,
            Cdr::state& current_state,
            XCdrHeaderSelection header_selection);

    /*!
     * @brief Tells to the encoder to finish the encoding of the member.
     * @param[in] current_state State of the encoder previous to call xcdr1_begin_serialize_opt_member function.
     * @pre next_member_id_ cannot be MEMBER_ID_INVALID.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to encode a long header when
     * header_selection is XCdrHeaderSelection::SHORT_HEADER.
     */
    Cdr& xcdr1_end_serialize_opt_member(
            const Cdr::state& current_state);

    /*!
     * @brief Tells to the encoder a member starts to be encoded according to XCDRv2.
     * @param[in] member_id Member identifier.
     * @pre Member identifier cannot be MEMBER_ID_INVALID and next_member_id_ must be equal to the member identifier or
     * MEMBER_ID_INVALID.
     * @param[in] is_present If the member is present.
     * @param[in,out] current_state State of the encoder previous to call this function.
     * @pre Current encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR2, EncodingAlgorithmFlag::DELIMIT_CDR2 or
     * EncodingAlgorithmFlag::PL_CDR2.
     * @param[in] header_selection Selects which member header will be used to allocate space.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to encode member identifier equal or
     * greater than 0x10000000.
     */
    Cdr& xcdr2_begin_serialize_member(
            const MemberId& member_id,
            bool is_present,
            Cdr::state& current_state,
            XCdrHeaderSelection header_selection);

    /*!
     * @brief Tells to the encoder to finish the encoding of the member.
     * @param[in] current_state State of the encoder previous to call xcdr2_begin_serialize_member function.
     * @pre next_member_id_ cannot be MEMBER_ID_INVALID.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when trying to encode a long header when
     * header_selection is XCdrHeaderSelection::SHORT_HEADER.
     */
    Cdr& xcdr2_end_serialize_member(
            const Cdr::state& current_state);

    /*!
     * @brief Tells to the encoder a new type and its members start to be encoded according to XCDRv1.
     * @param[in,out] current_state State of the encoder previous to call this function.
     * @pre Current encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR or EncodingAlgorithmFlag::PL_CDR.
     * @param[in] type_encoding Encoding algorithm used to encode the type and its members.
     * @pre Type encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR or EncodingAlgorithmFlag::PL_CDR.
     * @pre If it is the beginning of the whole encoding, current encoding must be equal to type encoding.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     */
    Cdr& xcdr1_begin_serialize_type(
            Cdr::state& current_state,
            EncodingAlgorithmFlag type_encoding) noexcept;

    /*!
     * @brief Tells to the encoder to finish the encoding of the type.
     * @param[in] current_state State of the encoder previous to call xcdr1_begin_serialize_type function.
     * @pre Current encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR or EncodingAlgorithmFlag::PL_CDR.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    Cdr& xcdr1_end_serialize_type(
            const Cdr::state& current_state);

    /*!
     * @brief Tells to the encoder a new type and its members start to be encoded according to XCDRv2.
     * @param[in,out] current_state State of the encoder previous to call this function.
     * @pre Current encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR2, EncodingAlgorithmFlag::DELIMIT_CDR2 or
     * EncodingAlgorithmFlag::PL_CDR2.
     * @param[in] type_encoding Encoding algorithm used to encode the type and its members.
     * @pre Type encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR2, EncodingAlgorithmFlag::DELIMIT_CDR2 or
     * EncodingAlgorithmFlag::PL_CDR2.
     * @pre If it is the beginning of the whole encoding, current encoding must be equal to type encoding.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    Cdr& xcdr2_begin_serialize_type(
            Cdr::state& current_state,
            EncodingAlgorithmFlag type_encoding);

    /*!
     * @brief Tells to the encoder to finish the encoding of the type.
     * @param[in] current_state State of the encoder previous to call xcdr2_begin_serialize_type function.
     * @pre Current encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR2, EncodingAlgorithmFlag::DELIMIT_CDR2 or
     * EncodingAlgorithmFlag::PL_CDR2.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     */
    Cdr& xcdr2_end_serialize_type(
            const Cdr::state& current_state);

    /*!
     * @brief Tells to the encoder a new type and its members start to be decoded according to XCDRv1.
     * @param[in] type_encoding Encoding algorithm used to encode the type and its members.
     * @pre Type encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR or EncodingAlgorithmFlag::PL_CDR.
     * @pre If it is the beginning of the whole encoding, current encoding must be equal to type encoding.
     * @param[in] functor Functor called each time a member has to be decoded.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when an incorrect behaviour happens when
     * trying to decode.
     */
    Cdr& xcdr1_deserialize_type(
            EncodingAlgorithmFlag type_encoding,
            std::function<bool (Cdr&, const MemberId&)> functor);

    /*!
     * @brief Tells to the encoder a new type and its members start to be decoded according to XCDRv2.
     * @param[in] type_encoding Encoding algorithm used to encode the type and its members.
     * @pre Type encoding algorithm must be EncodingAlgorithmFlag::PLAIN_CDR2, EncodingAlgorithmFlag::DELIMIT_CDR2 or
     * EncodingAlgorithmFlag::PL_CDR2.
     * @pre If it is the beginning of the whole encoding, current encoding must be equal to type encoding.
     * @param[in] functor Functor called each time a member has to be decoded.
     * @return Reference to the eprosima::fastcdr::Cdr object.
     * @exception exception::NotEnoughMemoryException This exception is thrown when trying to encode into a buffer
     * position that exceeds the internal memory size.
     * @exception exception::BadParamException This exception is thrown when an incorrect behaviour happens when
     * trying to decode.
     */
    Cdr& xcdr2_deserialize_type(
            EncodingAlgorithmFlag type_encoding,
            std::function<bool (Cdr&, const MemberId&)> functor);

    Cdr& cdr_begin_serialize_member(
            const MemberId& member_id,
            bool is_present,
            Cdr::state& current_state,
            XCdrHeaderSelection header_selection);

    Cdr& cdr_end_serialize_member(
            const Cdr::state& current_state);

    Cdr& cdr_begin_serialize_type(
            Cdr::state& current_state,
            EncodingAlgorithmFlag type_encoding);

    Cdr& cdr_end_serialize_type(
            const Cdr::state& current_state);

    Cdr& cdr_deserialize_type(
            EncodingAlgorithmFlag type_encoding,
            std::function<bool (Cdr&, const MemberId&)> functor);

    /*!
     * @brief Resets the internal callbacks depending on the current selected Cdr version.
     */
    void reset_callbacks();

    using begin_serialize_member_functor = Cdr& (Cdr::*)(
        const MemberId&,
        bool,
        Cdr::state&,
        XCdrHeaderSelection);
    begin_serialize_member_functor begin_serialize_member_ { nullptr };

    using end_serialize_member_functor = Cdr& (Cdr::*)(
        const Cdr::state&);
    end_serialize_member_functor end_serialize_member_ { nullptr };

    using begin_serialize_opt_member_functor = Cdr& (Cdr::*)(
        const MemberId&,
        bool,
        Cdr::state&,
        XCdrHeaderSelection);
    begin_serialize_opt_member_functor begin_serialize_opt_member_ { nullptr };

    using end_serialize_memberopt__functor = Cdr& (Cdr::*)(
        const Cdr::state&);
    end_serialize_member_functor end_serialize_opt_member_ { nullptr };

    using begin_serialize_type_functor = Cdr& (Cdr::*)(
        Cdr::state&,
        EncodingAlgorithmFlag);
    begin_serialize_type_functor begin_serialize_type_ { nullptr };

    using end_serialize_type_functor = Cdr& (Cdr::*)(
        const Cdr::state&);
    end_serialize_type_functor end_serialize_type_ { nullptr };

    using deserialize_type_functor = Cdr& (Cdr::*)(
        EncodingAlgorithmFlag,
        std::function<bool (Cdr&, const MemberId&)>);
    deserialize_type_functor deserialize_type_ { nullptr };

    //! @brief Reference to the buffer that will be serialized/deserialized.
    FastBuffer& cdr_buffer_;

    //! @brief The type of CDR that will be use in serialization/deserialization.
    CdrVersion cdr_version_ {CdrVersion::XCDRv2};

    //! @brief Stores the main encoding algorithm.
    EncodingAlgorithmFlag encoding_flag_ {EncodingAlgorithmFlag::PLAIN_CDR2};

    //! @brief Stores the current encoding algorithm.
    EncodingAlgorithmFlag current_encoding_ {EncodingAlgorithmFlag::PLAIN_CDR2};

    //! @brief This attribute stores the option flags when the CDR type is DDS_CDR;
    std::array<uint8_t, 2> options_{{0}};

    //! @brief The endianness that will be applied over the buffer.
    uint8_t endianness_ {Endianness::LITTLE_ENDIANNESS};

    //! @brief This attribute specifies if it is needed to swap the bytes.
    bool swap_bytes_ {false};

    //! @brief Stores the last datasize serialized/deserialized. It's used to optimize.
    size_t last_data_size_ {0};

    //! @brief The current position in the serialization/deserialization process.
    FastBuffer::iterator offset_;

    //! @brief The position from where the alignment is calculated.
    FastBuffer::iterator origin_;

    //! @brief The last position in the buffer;
    FastBuffer::iterator end_;

    //! Next member identifier to be processed.
    MemberId next_member_id_;

    //! Align for types equal or greater than 64bits.
    size_t align64_ {4};

    /*!
     * When serializing a member's type using XCDRv2, this enumerator is used to inform the type was serialized with a
     * DHEADER and the algorithm could optimize the XCDRv2 member header.
     */
    enum SerializedMemberSizeForNextInt
    {
        NO_SERIALIZED_MEMBER_SIZE,     //! Default. No serialized member size in a DHEADER.
        SERIALIZED_MEMBER_SIZE,        //! Serialized member size in a DHEADER.
        SERIALIZED_MEMBER_SIZE_4,      //! Serialized member size (which is a multiple of 4) in a DHEADER.
        SERIALIZED_MEMBER_SIZE_8       //! Serialized member size (which is a multiple of 8) in a DHEADER.
    }
    //! Specifies if a DHEADER was serialized. Used to optimize XCDRv2 member headers.
    serialized_member_size_ {NO_SERIALIZED_MEMBER_SIZE};

    //! Stores the initial state.
    state initial_state_;

    //! Whether the encapsulation was serialized.
    bool encapsulation_serialized_ {false};


    uint32_t get_long_lc(
            SerializedMemberSizeForNextInt serialized_member_size);

    uint32_t get_short_lc(
            size_t member_serialized_size);

    template<class _T, typename std::enable_if<std::is_enum<_T>::value ||
            std::is_arithmetic<_T>::value>::type* = nullptr>
    constexpr SerializedMemberSizeForNextInt get_serialized_member_size() const
    {
        return (1 == sizeof(_T) ? SERIALIZED_MEMBER_SIZE :
               (4 == sizeof(_T) ? SERIALIZED_MEMBER_SIZE_4 :
               (8 == sizeof(_T) ? SERIALIZED_MEMBER_SIZE_8 :  NO_SERIALIZED_MEMBER_SIZE)));
    }

};

}            //namespace fastcdr
}        //namespace eprosima

#endif // _CDR_CDR_H_
