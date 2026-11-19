// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "carla/Buffer.h"

#include <algorithm>
#include <cstring>
#include <ostream>
#include <streambuf>

namespace carla {
namespace multigpu {

  /// A std::streambuf that writes into a carla::Buffer, so a frame can be
  /// written once and sent as it is, without the copies an std::ostringstream
  /// and its std::string take. It supports tellp/seekp on the bytes already
  /// written, which the recorder packets use to back-patch their sizes.
  class BufferStreamBuf : public std::streambuf {
  public:

    explicit BufferStreamBuf(size_t capacity = 64u * 1024u) {
      Grow(std::max<size_t>(capacity, 256u));
    }

    /// Bytes written so far.
    size_t size() const {
      return std::max(_end, static_cast<size_t>(pptr() - pbase()));
    }

    /// Hands the bytes over. The stream buffer is empty afterwards.
    Buffer Release() {
      const size_t written = size();
      _buffer.reset(static_cast<Buffer::size_type>(written));
      Buffer result = std::move(_buffer);
      _end = 0u;
      setp(nullptr, nullptr);
      return result;
    }

  protected:

    int_type overflow(int_type ch) override {
      if (traits_type::eq_int_type(ch, traits_type::eof())) {
        return traits_type::not_eof(ch);
      }
      Reserve(1u);
      *pptr() = traits_type::to_char_type(ch);
      pbump(1);
      return ch;
    }

    std::streamsize xsputn(const char_type *data, std::streamsize count) override {
      if (count <= 0) {
        return 0;
      }
      Reserve(static_cast<size_t>(count));
      std::memcpy(pptr(), data, static_cast<size_t>(count));
      Advance(static_cast<size_t>(count));
      return count;
    }

    pos_type seekoff(off_type offset, std::ios_base::seekdir dir, std::ios_base::openmode which) override {
      if (!(which & std::ios_base::out)) {
        return pos_type(off_type(-1));
      }
      const off_type current = static_cast<off_type>(pptr() - pbase());
      off_type base = 0;
      if (dir == std::ios_base::cur) {
        base = current;
      } else if (dir == std::ios_base::end) {
        base = static_cast<off_type>(size());
      }
      return seekpos(pos_type(base + offset), which);
    }

    pos_type seekpos(pos_type position, std::ios_base::openmode which) override {
      const off_type target = static_cast<off_type>(position);
      if (!(which & std::ios_base::out) || (target < 0) ||
          (static_cast<size_t>(target) > size())) {
        return pos_type(off_type(-1));
      }
      _end = size();
      SetPut(static_cast<size_t>(target));
      return position;
    }

  private:

    /// Moves the put position by @a count bytes inside the current block.
    void Advance(size_t count) {
      size_t remaining = count;
      while (remaining > 0u) {
        const int step = static_cast<int>(std::min<size_t>(remaining, 1u << 30));
        pbump(step);
        remaining -= static_cast<size_t>(step);
      }
    }

    void SetPut(size_t offset) {
      char *begin = reinterpret_cast<char *>(_buffer.data());
      setp(begin, begin + _buffer.capacity());
      Advance(offset);
    }

    /// Makes room for @a count more bytes at the put position.
    void Reserve(size_t count) {
      const size_t position = static_cast<size_t>(pptr() - pbase());
      if (position + count <= _buffer.capacity()) {
        return;
      }
      _end = size();
      Grow(std::max(position + count, 2u * static_cast<size_t>(_buffer.capacity())));
      SetPut(position);
    }

    void Grow(size_t capacity) {
      Buffer bigger(static_cast<Buffer::size_type>(capacity));
      if (_end > 0u) {
        std::memcpy(bigger.data(), _buffer.data(), _end);
      }
      _buffer = std::move(bigger);
      if (pbase() == nullptr) {
        SetPut(0u);
      }
    }

    Buffer _buffer;

    /// The furthest byte written before the last seek or growth.
    size_t _end = 0u;
  };

} // namespace multigpu
} // namespace carla
