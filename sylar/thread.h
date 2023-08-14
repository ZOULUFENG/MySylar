#pragma once
#include <functional>
#include <memory>
#include <pthread.h>
#include <semaphore.h>
// #include <stdint.h>
#include <thread>

namespace sylar {

class Semaphore {
public:
    Semaphore(uint32_t count = 0);
    ~Semaphore();
    void wait();
    void notify();

private:
    Semaphore(const Semaphore&) = delete;
    Semaphore(const Semaphore&&) = delete;
    Semaphore& operator=(const Semaphore&) = delete;

private:
    sem_t m_semaphore;
};

template <typename T>
struct ScopeLockImpl {
public:
    ScopeLockImpl(T& mutex)
        : m_mutex(mutex)
    {
        m_mutex.lock();
        m_locked = true;
    }
    ~ScopeLockImpl() { unlock(); }
    void lock()
    {
        if (!m_locked) {
            m_mutex.lock();
            m_locked = true;
        }
    }
    void unlock()
    {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked = false;
};

template <typename T>
struct ReadScopeLockImpl {
public:
    ReadScopeLockImpl(T& mutex)
        : m_mutex(mutex)
    {
        m_mutex.rdlock();
        m_locked = true;
    }
    ~ReadScopeLockImpl() { unlock(); }
    void lock()
    {
        if (!m_locked) {
            m_mutex.rdlock();
            m_locked = true;
        }
    }
    void unlock()
    {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked = false;
};

template <typename T>
struct WriteScopeLockImpl {
public:
    WriteScopeLockImpl(T& mutex)
        : m_mutex(mutex)
    {
        m_mutex.wrlock();
        m_locked = true;
    }
    ~WriteScopeLockImpl() { unlock(); }
    void lock()
    {
        if (!m_locked) {
            m_mutex.wrlock();
            m_locked = true;
        }
    }
    void unlock()
    {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked = false;
};

class RWMutex {
public:
    typedef ReadScopeLockImpl<RWMutex> ReadLock;
    typedef WriteScopeLockImpl<RWMutex> WriteLock;
    RWMutex()
    {
        pthread_rwlock_init(&m_lock, nullptr);
    }
    ~RWMutex()
    {
        pthread_rwlock_destroy(&m_lock);
    }
    void rdlock()
    {
        pthread_rwlock_rdlock(&m_lock);
    }
    void wrlock()
    {
        pthread_rwlock_wrlock(&m_lock);
    }
    void unlock()
    {
        pthread_rwlock_unlock(&m_lock);
    }

private:
    RWMutex(const RWMutex&) = delete;
    RWMutex(const RWMutex&&) = delete;
    RWMutex& operator=(const RWMutex&) = delete;
    RWMutex& operator=(const RWMutex&&) = delete;
    pthread_rwlock_t m_lock;
};

class Thread {
public:
    typedef std::shared_ptr<Thread> ptr;
    Thread(std::function<void()> cb, const std::string& name = "");
    ~Thread();

    pid_t getId() const { return m_id; }
    const std::string& getName() const { return m_name; }

    void join();

    static Thread* GetThis(); // thread
    static const std::string& GetName(); // log
    static void SetName(const std::string& name);

private:
    Thread(const Thread&) = delete;
    Thread(const Thread&&) = delete;
    Thread& operator=(const Thread&) = delete;

    static void* run(void* arg);

private:
    pid_t m_id = -1;
    pthread_t m_thread = 0;
    std::function<void()> m_cb;
    std::string m_name;
    Semaphore m_semaphore;
};

}