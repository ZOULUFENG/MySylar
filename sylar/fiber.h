#pragma once

#include "thread.h"
#include "ucontext.h"
#include <functional>
#include <memory>

namespace sylar {
class Fiber : public std::enable_shared_from_this<Fiber> {
public:
    typedef std::shared_ptr<Fiber> ptr;

    enum State {
        INIT,
        HOLD,
        EXEC,
        TERM,
        READY,
        EXCEPT
    };

private:
    Fiber();

public:
    Fiber(std::function<void()> cb, size_t stacksize = 0);
    ~Fiber();

    void reset(std::function<void()> cb);

    void swapIn();
    void swapOut();

    uint64_t getId() const { return m_id; }

public:
    static void SetThis(Fiber* f);
    static Fiber::ptr GetThis();
    static void YieldToReady();
    static void YieldToHold();

    static uint64_t TotalFibers();
    static void MainFunc();
    static uint64_t GetFiberId();

private:
    uint64_t m_id = 0;
    uint32_t m_stacksize = 0;
    State m_state = INIT;

    // 上下文
    ucontext_t m_ctx;
    // 堆栈
    void* m_stack = nullptr;
    // 回调函数
    std::function<void()> m_cb;
};
} // namespace sylar
