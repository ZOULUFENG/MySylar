#include "fiber.h"
#include "config.h"
#include "log.h"
#include "macro.h"
#include "scheduler.h"
#include <atomic>
#include <iostream>

namespace sylar {
// Why is here the type of g_logger is static
static Logger::ptr g_logger = SYLAR_LOG_NAME("system");

static std::atomic<uint64_t> s_fiber_id { 0 };
static std::atomic<uint64_t> s_fiber_count { 0 };

static thread_local Fiber* t_fiber = nullptr;
static thread_local Fiber::ptr t_threadFiber = nullptr;

static ConfigVar<uint32_t>::ptr g_fiber_stack_size = Config::Lookup<uint32_t>("fiber.stack_size", 1024 * 1024, "fiber stack size");

class MallocStackAllocator {
public:
    static void* Alloc(size_t size)
    {
        return malloc(size);
    }
    static void Dealloc(void* vp, size_t size)
    {
        return free(vp);
    }
};

using StackAllcator = MallocStackAllocator;

Fiber::Fiber()
{
    m_state = EXEC;
    SetThis(this);

    if (getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }
    ++s_fiber_count;
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber() ";
}

Fiber::Fiber(std::function<void()> cb, size_t stacksize, bool use_caller)
    : m_id(++s_fiber_id)
    , m_cb(cb)
{
    ++s_fiber_count;
    m_stacksize = stacksize ? stacksize : g_fiber_stack_size->getValue();

    m_stack = StackAllcator::Alloc(m_stacksize);
    if (getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }
    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    if (!use_caller) {
        makecontext(&m_ctx, &Fiber::MainFunc, 0);
    } else {
        makecontext(&m_ctx, &Fiber::CallerMainFunc, 0);
    }

    SYLAR_LOG_DEBUG(g_logger) << "Fiber::Fiber() id=" << m_id;
}

Fiber::~Fiber()
{
    --s_fiber_count;
    if (m_stack) {
        SYLAR_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);

        StackAllcator::Dealloc(m_stack, m_stacksize);
    } else {
        SYLAR_ASSERT(!m_cb)
        SYLAR_ASSERT(m_state == EXEC);

        Fiber* cur = t_fiber;
        if (cur == this) {
            SetThis(nullptr);
        }
    }
    SYLAR_LOG_DEBUG(g_logger) << "Fiber::~Fiber() id=" << m_id;
}

void Fiber::reset(std::function<void()> cb)
{
    SYLAR_ASSERT(m_stack);
    SYLAR_ASSERT(m_state == TERM || m_state == INIT || m_state == EXCEPT);
    m_cb = cb;
    if (getcontext(&m_ctx)) {
        SYLAR_ASSERT2(false, "getcontext");
    }

    m_ctx.uc_link = nullptr;
    m_ctx.uc_stack.ss_sp = m_stack;
    m_ctx.uc_stack.ss_size = m_stacksize;

    makecontext(&m_ctx, &Fiber::MainFunc, 0);
    m_state = INIT;
}

void Fiber::call()
{
    SetThis(this);
    // SYLAR_ASSERT(m_state != EXEC);
    SYLAR_LOG_DEBUG(g_logger) << "call_fiber_id=" << getId();
    m_state = EXEC;
    if (swapcontext(&t_threadFiber->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "call swapcontext");
    }
}

void Fiber::back()
{
    SetThis(t_threadFiber.get());
    if (swapcontext(&m_ctx, &t_threadFiber->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

void Fiber::swapIn()
{
    SetThis(this);
    SYLAR_ASSERT(m_state != EXEC);
    m_state = EXEC;
    // swapcontext(&old_ctx, &new_ctx)
    if (swapcontext(&Scheduler::GetMainFiber()->m_ctx, &m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

void Fiber::swapOut()
{
    SetThis(Scheduler::GetMainFiber());
    if (swapcontext(&m_ctx, &Scheduler::GetMainFiber()->m_ctx)) {
        SYLAR_ASSERT2(false, "swapcontext");
    }
}

void Fiber::SetThis(Fiber* f)
{
    t_fiber = f;
}

Fiber::ptr Fiber::GetThis()
{
    if (t_fiber) {
        return t_fiber->shared_from_this();
    }
    Fiber::ptr main_fiber(new Fiber);
    SYLAR_ASSERT(t_fiber == main_fiber.get());
    t_threadFiber = main_fiber;
    return t_fiber->shared_from_this();
}

// Don't care that it is the main_fiber?
void Fiber::YieldToReady()
{
    Fiber::ptr cur = GetThis();
    cur->m_state = READY;
    cur->swapOut();
}
void Fiber::YieldToHold()
{
    Fiber::ptr cur = GetThis();
    cur->m_state = HOLD;
    cur->swapOut();
}
uint64_t Fiber::TotalFibers()
{
    return s_fiber_count;
}

uint64_t Fiber::GetFiberId()
{
    if (t_fiber) {
        return t_fiber->getId();
    }
    return 0;
}

void Fiber::MainFunc()
{
    Fiber::ptr cur = GetThis();
    // Is it still possible that it doesn't exist?
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (const std::exception& ex) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what() << std::endl
                                  << "fiber_id=" << cur->getId()
                                  << sylar::BacktraceToString(64);
    } catch (...) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << std::endl
                                  << "fiber_id=" << cur->getId()
                                  << sylar::BacktraceToString(64);
    }
    // Use this method to perfectly solve the remaining problems below
    // std::cout << "111" << std::endl;
    auto raw_ptr = cur.get();
    // std::cout << "222" << std::endl;
    cur.reset();
    // Perfect solution to the problem of not being able to return main end.But there is still a problem:
    // The destructor cannot deconstruct this object
    // std::cout << "333" << std::endl;
    raw_ptr->swapOut();
    // std::cout << "444" << std::endl;

    SYLAR_ASSERT2(false, "never reach! fiber_id=" + std::to_string(raw_ptr->getId()));
}

void Fiber::CallerMainFunc()
{
    Fiber::ptr cur = GetThis();
    // Is it still possible that it doesn't exist?
    SYLAR_ASSERT(cur);
    try {
        cur->m_cb();
        cur->m_cb = nullptr;
        cur->m_state = TERM;
    } catch (const std::exception& ex) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << ex.what() << std::endl
                                  << "fiber_id=" << cur->getId()
                                  << sylar::BacktraceToString(64);
    } catch (...) {
        cur->m_state = EXCEPT;
        SYLAR_LOG_ERROR(g_logger) << "Fiber Except: " << std::endl
                                  << "fiber_id=" << cur->getId()
                                  << sylar::BacktraceToString(64);
    }
    // Use this method to perfectly solve the remaining problems below
    auto raw_ptr = cur.get();
    cur.reset();
    // Perfect solution to the problem of not being able to return main end.But there is still a problem:
    // The destructor cannot deconstruct this object
    raw_ptr->back();

    SYLAR_ASSERT2(false, "never reach! fiber_id=" + std::to_string(raw_ptr->getId()));
}

} // namespace sylar
