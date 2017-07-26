// This file is a modification of https://stackoverflow.com/a/23713793

#include "carla/server/DoubleBuffer.h"

// The bottom (lowest) bit of the state will be the active cell (the one for
// writing). The active cell can only be switched if there's at most one
// concurrent user. The next two bits of state will be the number of concurrent
// users. The fourth bit indicates if there's a value available for reading in
// _buffer[0u], and the fifth bit has the same meaning but for _buffer[1u].

namespace carla {
namespace server {
namespace detail {

  DoubleBufferState::DoubleBufferState() :
    _state(0),
    _readState() {}

  // Never returns nullptr
  DoubleBufferState::ActiveBuffer DoubleBufferState::StartWriting() {
      // Increment active users; once we do this, no one can swap the active
      // cell on us until we're done
      auto state = _state.fetch_add(0x2, std::memory_order_relaxed);
      return static_cast<ActiveBuffer>(state & 1);
  }

  void DoubleBufferState::EndWriting() {
      // We want to swap the active cell, but only if we were the last ones
      // concurrently accessing the data (otherwise the consumer will do it
      // for us when *it's* done accessing the data).

      auto state = _state.load(std::memory_order_relaxed);
      std::uint32_t flag = (8 << (state & 1)) ^ (state & (8 << (state & 1)));
      state = _state.fetch_add(flag - 0x2, std::memory_order_release) + flag - 0x2;
      if ((state & 0x6) == 0) {
          // The consumer wasn't in the middle of a read, we should swap
          // (unless the consumer has since started a read or already
          // swapped or read a value and is about to swap). If we swap, we
          // also want to clear the full flag on what will become the active
          // cell, otherwise the consumer could eventually read two values
          // out of order (it reads a new value, then swaps and reads the
          // old value while the producer is idle).
          _state.compare_exchange_strong(state, (state ^ 0x1) & ~(0x10 >> (state & 1)), std::memory_order_release);
      }
  }

  // Returns nullptr if there appears to be no more data to read yet.
  DoubleBufferState::ActiveBuffer DoubleBufferState::StartReading() {
      _readState = _state.load(std::memory_order_relaxed);
      if ((_readState & (0x10 >> (_readState & 1))) == 0) {
          // Nothing to read here!
          return NUMBER_OF_BUFFERS;
      }

      // At this point, there is guaranteed to be something to read, because
      // the full flag is never turned off by the producer thread once it's
      // on; the only thing that could happen is that the active cell
      // changes, but that can only happen after the producer wrote a value
      // into it, in which case there's still a value to read, just in a
      // different cell.

      _readState = _state.fetch_add(0x2, std::memory_order_acquire) + 0x2;

      // Now that we've incremented the user count, nobody can swap until we
      // decrement it.
      return static_cast<ActiveBuffer>((_readState & 1) ^ 1);
  }

  void DoubleBufferState::EndReading() {
      if ((_readState & (0x10 >> (_readState & 1))) == 0) {
          // There was nothing to read; shame to repeat this check, but if
          // these functions are inlined it might not matter. Otherwise the
          // API could be changed. Or just don't call this method if
          // start_reading() returns nullptr -- then you could also get rid
          // of _readState.
          return;
      }

      // Alright, at this point the active cell cannot change on us, but the
      // active cell's flag could change and the user count could change. We
      // want to release our user count and remove the flag on the value we
      // read.

      auto state = _state.load(std::memory_order_relaxed);
      std::uint32_t sub = (0x10 >> (state & 1)) | 0x2;
      state = _state.fetch_sub(sub, std::memory_order_relaxed) - sub;
      if ((state & 0x6) == 0 && (state & (0x8 << (state & 1))) == 1) {
          // Oi, we were the last ones accessing the data when we released
          // our cell. That means we should swap, but only if the producer
          // isn't in the middle of producing something, and hasn't already
          // swapped, and hasn't already set the flag we just reset (which
          // would mean they swapped an even number of times).  Note that we
          // don't bother swapping if there's nothing to read in the other
          // cell.
          _state.compare_exchange_strong(state, state ^ 0x1, std::memory_order_relaxed);
      }
  }

} // namespace detail
} // namespace carla
} // namespace server
