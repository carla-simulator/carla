/****************************************************************************/
// Eclipse SUMO, Simulation of Urban MObility; see https://eclipse.org/sumo
// Copyright (C) 2004-2020 German Aerospace Center (DLR) and others.
// This program and the accompanying materials are made available under the
// terms of the Eclipse Public License 2.0 which is available at
// https://www.eclipse.org/legal/epl-2.0/
// This Source Code may also be made available under the following Secondary
// Licenses when the conditions for such availability set forth in the Eclipse
// Public License 2.0 are satisfied: GNU General Public License, version 2
// or later which is available at
// https://www.gnu.org/licenses/old-licenses/gpl-2.0-standalone.html
// SPDX-License-Identifier: EPL-2.0 OR GPL-2.0-or-later
/****************************************************************************/
/// @file    FXWorkerThread.h
/// @author  Michael Behrisch
/// @date    2014-07-13
///
// A thread class together with a pool and a task for parallelized computation
/****************************************************************************/

#ifndef FXWorkerThread_h
#define FXWorkerThread_h

// #define WORKLOAD_PROFILING
// at which interval report maximum workload of the threads, needs WORKLOAD_PROFILING
// undefine to use summary report only
#define WORKLOAD_INTERVAL 100
#include <config.h>

#include <list>
#include <vector>
#include <fx.h>
#ifdef WORKLOAD_PROFILING
#include <chrono>
#include <utils/common/MsgHandler.h>
#include <utils/common/ToString.h>
#endif
#include <utils/common/UtilExceptions.h>


// ===========================================================================
// class definitions
// ===========================================================================
/**
 * @class FXWorkerThread
 * @brief A thread repeatingly calculating incoming tasks
 */
class FXWorkerThread : public FXThread {

public:
    /**
     * @class FXWorkerThread::Task
     * @brief Abstract superclass of a task to be run with an index to keep track of pending tasks
     */
    class Task {
    public:
        /// @brief Desctructor
        virtual ~Task() {};

        /** @brief Abstract method which in subclasses should contain the computations to be performed.
         *
         * If there is data to be shared among several tasks (but not among several threads) it can be put in the
         *  a thread class subclassing the FXWorkerThread. the instance of the thread is then made available
         *  via the context parameter.
         *
         * @param[in] context The thread which runs the task
         */
        virtual void run(FXWorkerThread* context) = 0;

        /** @brief Sets the running index of this task.
         *
         * Every task receive an index which is unique among all pending tasks of the same thread pool.
         *
         * @param[in] newIndex the index to assign
         */
        void setIndex(const int newIndex) {
            myIndex = newIndex;
        }
    private:
        /// @brief the index of the task, valid only after the task has been added to the pool
        int myIndex;
    };

    /**
     * @class FXWorkerThread::Pool
     * @brief A pool of worker threads which distributes the tasks and collects the results
     */
    class Pool {
    public:
        /** @brief Constructor
         *
         * May initialize the pool with a given number of workers.
         *
         * @param[in] numThreads the number of threads to create
         */
        Pool(int numThreads = 0) : myPoolMutex(true), myRunningIndex(0), myException(nullptr)
#ifdef WORKLOAD_PROFILING
            , myNumBatches(0), myTotalMaxLoad(0.), myTotalSpread(0.)
#endif
        {
#ifdef WORKLOAD_PROFILING
            long long int timeDiff = 0;
            for (int i = 0; i < 100; i++) {
                const auto begin = std::chrono::high_resolution_clock::now();
                const auto end = std::chrono::high_resolution_clock::now();
                timeDiff += std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
            }
            //std::cout << ("Average cost of a timing call (in ns): " + toString(timeDiff / 100.)) << std::endl;
#endif
            while (numThreads > 0) {
                new FXWorkerThread(*this);
                numThreads--;
            }
        }

        /** @brief Destructor
         *
         * Stopping and deleting all workers by calling clear.
         */
        virtual ~Pool() {
            clear();
        }

        /** @brief Stops and deletes all worker threads.
         */
        void clear() {
            for (FXWorkerThread* const worker : myWorkers) {
                delete worker;
            }
            myWorkers.clear();
        }

        /** @brief Adds the given thread to the pool.
         *
         * @param[in] w the thread to add
         */
        void addWorker(FXWorkerThread* const w) {
            myWorkers.push_back(w);
        }

        /** @brief Gives a number to the given task and assigns it to the worker with the given index.
         * If the index is negative, assign to the next (round robin) one.
         *
         * @param[in] t the task to add
         * @param[in] index index of the worker thread to use or -1 for an arbitrary one
         */
        void add(Task* const t, int index = -1) {
            if (index < 0) {
                index = myRunningIndex % myWorkers.size();
            }
#ifdef WORKLOAD_PROFILING
            if (myRunningIndex == 0) {
                for (FXWorkerThread* const worker : myWorkers) {
                    worker->startProfile();
                }
                myProfileStart = std::chrono::high_resolution_clock::now();
            }
#endif
            t->setIndex(myRunningIndex++);
            myWorkers[index]->add(t);
        }

        /** @brief Adds the given tasks to the list of finished tasks.
         *
         * Locks the internal mutex and appends the finished tasks. This is to be called by the worker thread only.
         *
         * @param[in] tasks the tasks to add
         */
        void addFinished(std::list<Task*>& tasks) {
            myMutex.lock();
            myFinishedTasks.splice(myFinishedTasks.end(), tasks);
            myCondition.signal();
            myMutex.unlock();
        }

        void setException(ProcessError& e) {
            myMutex.lock();
            if (myException == nullptr) {
                myException = new ProcessError(e);
            }
            myMutex.unlock();
        }

        /// @brief waits for all tasks to be finished
        void waitAll(const bool deleteFinished = true) {
            myMutex.lock();
            while ((int)myFinishedTasks.size() < myRunningIndex) {
                myCondition.wait(myMutex);
            }
#ifdef WORKLOAD_PROFILING
            if (myRunningIndex > 0) {
                const auto end = std::chrono::high_resolution_clock::now();
                const long long int elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - myProfileStart).count();
                double minLoad = std::numeric_limits<double>::max();
                double maxLoad = 0.;
                for (FXWorkerThread* const worker : myWorkers) {
                    const double load = worker->endProfile(elapsed);
                    minLoad = MIN2(minLoad, load);
                    maxLoad = MAX2(maxLoad, load);
                }
#ifdef WORKLOAD_INTERVAL
                myTotalMaxLoad += maxLoad;
                myTotalSpread += maxLoad / minLoad;
                myNumBatches++;
                if (myNumBatches % WORKLOAD_INTERVAL == 0) {
                    WRITE_MESSAGE(toString(myFinishedTasks.size()) + " tasks, average maximum load: " + toString(myTotalMaxLoad / WORKLOAD_INTERVAL) + ", average spread: " + toString(myTotalSpread / WORKLOAD_INTERVAL));
                    myTotalMaxLoad = 0.;
                    myTotalSpread = 0.;
                }
#endif
            }
#endif
            if (deleteFinished) {
                for (Task* task : myFinishedTasks) {
                    delete task;
                }
            }
            ProcessError* toRaise = myException;
            myException = nullptr;
            myFinishedTasks.clear();
            myRunningIndex = 0;
            myMutex.unlock();
            if (toRaise != nullptr) {
                throw* toRaise;
            }
        }

        /** @brief Checks whether there are currently more pending tasks than threads.
         *
         * This is only a rough estimate because the tasks are already assigned and there could be an idle thread even though the
         *  number of tasks is large.
         *
         * @return whether there are enough tasks to let all threads work
         */
        bool isFull() const {
            return myRunningIndex - (int)myFinishedTasks.size() >= size();
        }

        /** @brief Returns the number of threads in the pool.
         *
         * @return the number of threads
         */
        int size() const {
            return (int)myWorkers.size();
        }

        /// @brief locks the pool mutex
        void lock() {
            myPoolMutex.lock();
        }

        /// @brief unlocks the pool mutex
        void unlock() {
            myPoolMutex.unlock();
        }

        const std::vector<FXWorkerThread*>& getWorkers() {
            return myWorkers;
        }
    private:
        /// @brief the current worker threads
        std::vector<FXWorkerThread*> myWorkers;
        /// @brief the internal mutex for the task list
        FXMutex myMutex;
        /// @brief the pool mutex for external sync
        FXMutex myPoolMutex;
        /// @brief the semaphore to wait on for finishing all tasks
        FXCondition myCondition;
        /// @brief list of finished tasks
        std::list<Task*> myFinishedTasks;
        /// @brief the running index for the next task
        int myRunningIndex;
        /// @brief the exception from a child thread
        ProcessError* myException;
#ifdef WORKLOAD_PROFILING
        /// @brief the number of finished batch runs
        int myNumBatches;
        /// @brief the sum over the maximum loads
        double myTotalMaxLoad;
        /// @brief the sum over the load spreads
        double myTotalSpread;
        /// @brief the time when profiling started
        std::chrono::high_resolution_clock::time_point myProfileStart;
#endif
    };

public:
    /** @brief Constructor
     *
     * Adds the thread to the given pool and starts it.
     *
     * @param[in] pool the pool for this thread
     */
    FXWorkerThread(Pool& pool): FXThread(), myPool(pool), myStopped(false)
#ifdef WORKLOAD_PROFILING
        , myCounter(0), myBusyTime(0), myTotalBusyTime(0), myTotalTime(0)
#endif
    {
        pool.addWorker(this);
        start();
    }

    /** @brief Destructor
     *
     * Stops the thread by calling stop.
     */
    virtual ~FXWorkerThread() {
        stop();
#ifdef WORKLOAD_PROFILING
        const double load = 100. * myTotalBusyTime / myTotalTime;
        WRITE_MESSAGE("Thread " + toString((long long int)this) + " ran " + toString(myCounter) +
                      " tasks and had a load of " + toString(load) + "% (" + toString(myTotalBusyTime) +
                      "us / " + toString(myTotalTime) + "us), " + toString(myTotalBusyTime / (double)myCounter) + " per task.");
#endif
    }

    /** @brief Adds the given task to this thread to be calculated
     *
     * @param[in] t the task to add
     */
    void add(Task* t) {
        myMutex.lock();
        myTasks.push_back(t);
        myCondition.signal();
        myMutex.unlock();
    }

    /** @brief Main execution method of this thread.
     *
     * Checks for new tasks, calculates them and puts them in the finished list of the pool until being stopped.
     *
     * @return always 0
     */
    FXint run() {
        while (!myStopped) {
            myMutex.lock();
            while (!myStopped && myTasks.empty()) {
                myCondition.wait(myMutex);
            }
            if (myStopped) {
                myMutex.unlock();
                break;
            }
            myCurrentTasks.splice(myCurrentTasks.end(), myTasks);
            myMutex.unlock();
            try {
                for (Task* const t : myCurrentTasks) {
#ifdef WORKLOAD_PROFILING
                    const auto before = std::chrono::high_resolution_clock::now();
#endif
                    t->run(this);
#ifdef WORKLOAD_PROFILING
                    const auto after = std::chrono::high_resolution_clock::now();
                    myBusyTime += std::chrono::duration_cast<std::chrono::microseconds>(after - before).count();
                    myCounter++;
#endif
                }
            } catch (ProcessError& e) {
                myPool.setException(e);
            }
            myPool.addFinished(myCurrentTasks);
        }
        return 0;
    }

    /** @brief Stops the thread
     *
     * The currently running task will be finished but all further tasks are discarded.
     */
    void stop() {
        myMutex.lock();
        myStopped = true;
        myCondition.signal();
        myMutex.unlock();
        join();
    }

#ifdef WORKLOAD_PROFILING
    void startProfile() {
        myBusyTime = 0;
    }

    double endProfile(const long long int time) {
        myTotalTime += time;
        myTotalBusyTime += myBusyTime;
        return time == 0 ? 100. : 100. * myBusyTime / time;
    }
#endif

private:
    /// @brief the pool for this thread
    Pool& myPool;
    /// @brief the mutex for the task list
    FXMutex myMutex;
    /// @brief the semaphore when waiting for new tasks
    FXCondition myCondition;
    /// @brief the list of pending tasks
    std::list<Task*> myTasks;
    /// @brief the list of tasks which are currently calculated
    std::list<Task*> myCurrentTasks;
    /// @brief whether we are still running
    bool myStopped;
#ifdef WORKLOAD_PROFILING
    /// @brief counting completed tasks
    int myCounter;
    /// @brief the time spent in calculations during the current batch
    long long int myBusyTime;
    /// @brief the total busy time
    long long int myTotalBusyTime;
    /// @brief the total time while anyone had tasks
    long long int myTotalTime;
#endif
};


#endif
