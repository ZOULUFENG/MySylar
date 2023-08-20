#pragma once
#include "fiber.h"
#include "thread.h"
#include <list>
#include <memory>
#include <vector>

namespace sylar {
class Scheduler {
public:
    typedef std::shared_ptr<Scheduler> ptr;
    typedef Mutex MutexType;

    // what's the mean and function about use_caller
    Scheduler(size_t threads = 1, bool use_caller = true, const std::string& name = "");
    // Why is the virtual type used here
    virtual ~Scheduler();

    const std::string& getName() const { return m_name; }

    static Scheduler* GetThis();
    // static Scheduler::ptr GetThis(); What is the difference from the above statement

    static Fiber* GetMainFiber();

    void start();
    void stop();

    template <class FiberOrCb>
    // Why is the default thread -1
    void schedule(FiberOrCb fc, int thread = -1)
    {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            need_tickle = scheduleNoLock(fc, thread);
        }
        if (need_tickle) {
            tickle();
        }
    }

    template <class InputIterator>
    void schedule(InputIterator begin, InputIterator end)
    {
        bool need_tickle = false;
        {
            MutexType::Lock lock(m_mutex);
            while (begin != end) {
                need_tickle = scheduleNoLock(&*begin) || need_tickle;
                ++begin;
            }
        }
        if (need_tickle) {
            tickle();
        }
    }

protected:
    virtual void tickle();
    void run();
    virtual bool stopping();
    virtual void idle();
    void setThis();

private:
    // What is the function of this function
    template <class FiberOrCb>
    bool scheduleNoLock(FiberOrCb fc, int thread)
    {
        bool need_tickle = m_fibers.empty();
        FiberAndThread ft(fc, thread);
        if (ft.fiber || ft.cb) {
            m_fibers.push_back(ft);
        }
        return need_tickle;
    }

private:
    struct FiberAndThread {
        Fiber::ptr fiber;
        std::function<void()> cb;
        int thread;

        FiberAndThread(Fiber::ptr f, int thr)
            : fiber(f)
            , thread(thr)
        {
        }
        // What is the function of this function
        FiberAndThread(Fiber::ptr* f, int thr)
            : thread(thr)
        {
            fiber.swap(*f);
        }

        FiberAndThread(std::function<void()> f, int thr)
            : cb(f)
            , thread(thr)
        {
        }

        FiberAndThread(std::function<void()>* f, int thr)
            : thread(thr)
        {
            cb.swap(*f);
        }

        // Why does it still need to provide a default constructor here
        FiberAndThread()
            : thread(-1)
        {
        }
        void reset()
        {
            fiber = nullptr;
            cb = nullptr;
            thread = -1;
        }
    };

private:
    MutexType m_mutex;
    std::vector<Thread::ptr> m_threads;
    // why is the list type used here
    // Because for the convenience of insertion and deletion operations
    std::list<FiberAndThread> m_fibers;
    Fiber::ptr m_rootFiber;
    std::string m_name;

protected:
    std::vector<int> m_threadIds;
    size_t m_threadCount = 0;
    std::atomic<size_t> m_activeThreadCount = { 0 };
    std::atomic<size_t> m_idleThreadCount = { 0 };
    bool m_stopping = true;
    bool m_autoStop = false;
    int m_rootThread = 0; // user_caller's Id
};

} // namespace sylar
