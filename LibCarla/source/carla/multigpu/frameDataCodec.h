// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB).
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <istream>
#include <optional>
#include <ostream>
#include <streambuf>
#include <string>
#include <type_traits>
#include <utility>

namespace carla {
namespace multigpu {

  /// Frame data (the SEND_FRAME payload) is a sequence of packets
  /// [uint8 id][uint32 payload size][payload] in host byte order. Every server
  /// runs the same build, so nothing is byte swapped.
  constexpr std::size_t kFrameDataHeaderSize = 5u;

  /// Id of the frame counter packet, the last packet of every complete frame.
  constexpr uint8_t kFrameCounterPacketId = 18u;

  /// Version of the whole Multi-GPU wire format, carried in every frame. It is
  /// only reported: readers skip what they do not know instead of rejecting it.
  constexpr uint8_t kMultiGpuProtocolVersion = 1u;

  enum class FrameDataStatus : uint8_t {
    Ok,
    TruncatedHeader,
    TruncatedPayload,
    ShortPacket,
    ReaderError,
    MissingFrameCounter,
    /// Packets that contradict each other (set by the reader's owner, e.g. a
    /// FrameTime for another frame than the FrameCounter).
    Inconsistent
  };

  inline const char *FrameDataStatusName(FrameDataStatus status) {
    switch (status) {
      case FrameDataStatus::Ok: return "complete";
      case FrameDataStatus::TruncatedHeader: return "truncated packet header";
      case FrameDataStatus::TruncatedPayload: return "truncated packet payload";
      case FrameDataStatus::ShortPacket: return "packet shorter than its fields";
      case FrameDataStatus::ReaderError: return "packet reader failed";
      case FrameDataStatus::MissingFrameCounter: return "no frame counter";
      case FrameDataStatus::Inconsistent: return "packets that contradict each other";
    }
    return "unknown status";
  }

  struct FrameDataReadResult {
    FrameDataStatus status = FrameDataStatus::Ok;
    /// Value of the frame counter packet, if it was read.
    std::optional<uint64_t> frame;
    /// Id, offset and declared size of the last packet looked at.
    uint8_t packet_id = 0u;
    std::size_t offset = 0u;
    uint32_t packet_size = 0u;
    /// Packets whose id this build does not know.
    uint32_t skipped = 0u;
    uint8_t first_skipped_id = 0u;
    /// Known packets with trailing bytes this build does not read (written by
    /// a newer build).
    uint32_t grown = 0u;

    /// Only a complete frame may be played.
    bool IsComplete() const {
      return status == FrameDataStatus::Ok;
    }
  };

namespace detail {

  /// Read-only stream buffer over one packet's payload. It cannot seek, so a
  /// reader can neither leave its packet nor make the packet loop lose its place.
  class PayloadBuffer : public std::streambuf {
  public:

    PayloadBuffer(const uint8_t *data, std::size_t size) {
      char *begin = const_cast<char *>(reinterpret_cast<const char *>(data));
      setg(begin, begin, begin + size);
    }

    std::size_t Remaining() const {
      return static_cast<std::size_t>(egptr() - gptr());
    }
  };

  template <typename T>
  void WritePod(std::ostream &out, const T &value) {
    static_assert(std::is_trivially_copyable_v<T>, "only plain values go on the wire");
    out.write(reinterpret_cast<const char *>(&value), sizeof(T));
  }

  template <typename T>
  bool ReadPod(std::istream &in, T &value) {
    static_assert(std::is_trivially_copyable_v<T>, "only plain values go on the wire");
    in.read(reinterpret_cast<char *>(&value), sizeof(T));
    return !in.fail();
  }

} // namespace detail

  inline void WriteFrameDataHeader(std::ostream &out, uint8_t id, uint32_t payload_size) {
    detail::WritePod(out, id);
    detail::WritePod(out, payload_size);
  }

  /// Writes a packet whose payload was built beforehand.
  inline void WriteFrameDataPacket(std::ostream &out, uint8_t id, const std::string &payload) {
    WriteFrameDataHeader(out, id, static_cast<uint32_t>(payload.size()));
    out.write(payload.data(), static_cast<std::streamsize>(payload.size()));
  }

  /// Walks the packets of one frame. @a read_packet(id, payload, size) reads a
  /// known packet from @a payload, a stream that ends where the packet ends,
  /// and returns true; for an id it does not know it returns false without
  /// reading. The loop always moves on by the declared size, so unknown
  /// packets, packets that grew in a newer build and empty packets are skipped,
  /// and it never reads outside [data, data + size).
  template <typename ReadPacket>
  FrameDataReadResult ReadFrameData(const uint8_t *data, std::size_t size, ReadPacket &&read_packet) {
    FrameDataReadResult result;
    std::size_t offset = 0u;
    while (offset < size) {
      result.offset = offset;
      if (size - offset < kFrameDataHeaderSize) {
        result.status = FrameDataStatus::TruncatedHeader;
        return result;
      }
      uint8_t id = 0u;
      uint32_t payload_size = 0u;
      std::memcpy(&id, data + offset, sizeof(id));
      std::memcpy(&payload_size, data + offset + sizeof(id), sizeof(payload_size));
      result.packet_id = id;
      result.packet_size = payload_size;
      const std::size_t payload = offset + kFrameDataHeaderSize;
      if (payload_size > size - payload) {
        result.status = FrameDataStatus::TruncatedPayload;
        return result;
      }

      detail::PayloadBuffer buffer(data + payload, payload_size);
      std::istream in(&buffer);
      bool known = true;
      if (id == kFrameCounterPacketId) {
        uint64_t frame = 0u;
        if (detail::ReadPod(in, frame)) {
          result.frame = frame;
        }
      } else {
#ifndef LIBCARLA_NO_EXCEPTIONS
        try {
#endif // LIBCARLA_NO_EXCEPTIONS
          known = read_packet(id, in, payload_size);
#ifndef LIBCARLA_NO_EXCEPTIONS
        } catch (const std::exception &) {
          result.status = FrameDataStatus::ReaderError;
          return result;
        }
#endif // LIBCARLA_NO_EXCEPTIONS
      }

      if (!known) {
        if (result.skipped++ == 0u) {
          result.first_skipped_id = id;
        }
      } else if (in.fail()) {
        result.status = FrameDataStatus::ShortPacket;
        return result;
      } else if (buffer.Remaining() > 0u) {
        ++result.grown;
      }
      offset = payload + payload_size;
    }
    if (!result.frame) {
      result.status = FrameDataStatus::MissingFrameCounter;
    }
    return result;
  }

  /// Reads the fields of one packet (or one record inside it) without ever
  /// consuming more than its declared size. A read past the end fails, and a
  /// failed reader fails its stream, so the packet loop reports a short packet.
  ///
  /// Payload layouts of the Multi-GPU packets:
  ///   - fixed-size records:    [u32 count][u16 record_size]{record}
  ///   - variable-size records: [u32 count]{[u32 record_size][record]}
  ///   - optional fields (TLV): {[u8 kind][u32 size][value]}
  /// A record or field only ever grows at its end; readers read the fields
  /// they know and skip the rest.
  class BoundedReader {
  public:

    BoundedReader(std::istream &in, uint32_t size)
      : _in(in),
        _remaining(size) {}

    template <typename T>
    bool Read(T &value) {
      if (_failed || (_remaining < sizeof(T)) || !detail::ReadPod(_in, value)) {
        return Fail();
      }
      _remaining -= static_cast<uint32_t>(sizeof(T));
      return true;
    }

    bool ReadBytes(void *out, uint32_t bytes) {
      if (_failed || (_remaining < bytes)) {
        return Fail();
      }
      _in.read(static_cast<char *>(out), static_cast<std::streamsize>(bytes));
      if (_in.fail()) {
        return Fail();
      }
      _remaining -= bytes;
      return true;
    }

    bool Skip(uint32_t bytes) {
      if (_failed || (_remaining < bytes)) {
        return Fail();
      }
      _in.ignore(static_cast<std::streamsize>(bytes));
      if (static_cast<uint32_t>(_in.gcount()) != bytes) {
        return Fail();
      }
      _remaining -= bytes;
      return true;
    }

    /// Runs @a read(record) on the next @a bytes as a reader of their own, then
    /// skips what it left unread (a record that grew in a newer build).
    template <typename F>
    bool ReadRecord(uint32_t bytes, F &&read) {
      if (_failed || (_remaining < bytes)) {
        return Fail();
      }
      BoundedReader record(_in, bytes);
      if (!read(record) || record.Failed() || !record.Skip(record.Remaining())) {
        return Fail();
      }
      _remaining -= bytes;
      return true;
    }

    /// Header of a list of fixed-size records, checked against the bytes left.
    bool ReadFixedListHeader(uint16_t min_record_size, uint32_t &count, uint16_t &record_size) {
      count = 0u;
      record_size = 0u;
      if (!Read(count) || !Read(record_size)) {
        return false;
      }
      if ((record_size < min_record_size) ||
          (static_cast<uint64_t>(count) * record_size > _remaining)) {
        return Fail();
      }
      return true;
    }

    /// Count of a list of variable-size records, checked against the bytes left.
    bool ReadVariableListCount(uint32_t &count) {
      count = 0u;
      if (!Read(count)) {
        return false;
      }
      if (count > _remaining / sizeof(uint32_t)) {
        return Fail();
      }
      return true;
    }

    /// Reads one [u32 record_size][record] entry of a variable-size list.
    template <typename F>
    bool ReadSizedRecord(F &&read) {
      uint32_t bytes = 0u;
      return Read(bytes) && ReadRecord(bytes, std::forward<F>(read));
    }

    /// Reads TLV fields until the reader is exhausted. @a read(kind, value)
    /// reads a known kind and returns true, or returns true without reading
    /// to skip an unknown one; false aborts the packet.
    template <typename F>
    bool ReadFields(F &&read) {
      while (_remaining > 0u) {
        uint8_t kind = 0u;
        uint32_t bytes = 0u;
        if (!Read(kind) || !Read(bytes)) {
          return false;
        }
        if (!ReadRecord(bytes, [&](BoundedReader &value) { return read(kind, value); })) {
          return false;
        }
      }
      return !_failed;
    }

    uint32_t Remaining() const {
      return _remaining;
    }

    bool Failed() const {
      return _failed;
    }

  private:

    bool Fail() {
      _failed = true;
      _in.setstate(std::ios_base::failbit);
      return false;
    }

    std::istream &_in;

    uint32_t _remaining;

    bool _failed = false;
  };

  /// Header of a list of fixed-size records.
  inline void WriteFixedListHeader(std::ostream &out, uint32_t count, uint16_t record_size) {
    detail::WritePod(out, count);
    detail::WritePod(out, record_size);
  }

  /// One [u32 size][bytes] entry: a variable-size record or, after a kind byte,
  /// a TLV value.
  inline void WriteSizedBytes(std::ostream &out, const std::string &bytes) {
    detail::WritePod(out, static_cast<uint32_t>(bytes.size()));
    out.write(bytes.data(), static_cast<std::streamsize>(bytes.size()));
  }

  inline void WriteField(std::ostream &out, uint8_t kind, const std::string &value) {
    detail::WritePod(out, kind);
    WriteSizedBytes(out, value);
  }

} // namespace multigpu
} // namespace carla
