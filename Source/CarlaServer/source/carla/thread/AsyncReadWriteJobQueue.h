// CARLA, Copyright (C) 2017 Computer Vision Center (CVC)

#pragma once

#include <carla/thread/ThreadSafeQueue.h>
#include <carla/thread/ThreadUniquePointer.h>

#include <atomic>
#include <functional>

namespace carla {
namespace thread {

  /// Executes the given job asynchronously. Every item that the job returns is
  /// added to the queue.
  template <typename W, typename R>
  class CARLA_API AsyncReadWriteJobQueue {
  public:

    using WritingJob = std::function<W()>;
	using ReadingJob = std::function<void(R)>;

    explicit AsyncReadWriteJobQueue(WritingJob &&writingJob, ReadingJob &&readingJob) :
        _done(false),
		_writeJob(std::move(writingJob)),
		_readJob(std::move(readingJob)),
		_writeQueue(),
        _thread(new std::thread(&AsyncReadWriteJobQueue::workerThread, this)) {}

    ~AsyncReadWriteJobQueue() {
      _done = true;
    }

    bool tryPop(W &value) {
      return _writeQueue.try_pop(value);
    }

	void push(R item) {
		_readQueue.push(item);
	}

  private:

    void workerThread() {
      while (!_done) {
		  R value;
		  _readQueue.wait_and_pop(value);
		  _readJob(value);
		  _writeQueue.push(_writeJob());
      }
    }

    std::atomic_bool _done;

	WritingJob _writeJob;
	ReadingJob _readJob;

    ThreadSafeQueue<W> _writeQueue;
	ThreadSafeQueue<R> _readQueue;

    const ThreadUniquePointer _thread;
  };

} // namespace thread
} // namespace carla
