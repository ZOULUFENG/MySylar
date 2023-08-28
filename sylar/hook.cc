#include "hook.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_NAME("system");

namespace sylar {

static thread_local bool t_hook_enabled = false;

#define HOOK_FUN(XX) \ 
    XX(sleep)  \
    XX(usleep) \
        XX(nanosleep)

void hook_init()
{
    static bool is_inited = false;
    if (is_inited)
        return;

#define XX(name) name##_f = (name##_fun)dlsym(RTLD_NEXT, #name);
    HOOK_FUN(XX)
#undef XX
}

//
struct _HookIniter {
    HookIniter()
    {
        hook_init();
    }
};
// Let hook start before main
static _HookIniter s_hook_initer;

bool is_hook_enabled()
{
    return t_hook_enabled;
}

void set_hook_enable(bool flag)
{
    return t_hook_enabled = flag;
}
}

extern "C" {
#define XX(name) name##_fun name##_f = nullptr;
HOOK_FUN(XX)
#undef XX
unsigned int sleep(unsigned int seconds)
{
    if (!sylar::t_hook_enabled) {
        return sleep_f(seconds);
    }

    syalr::Fiber::ptr fiber = sylar::Fiber::GetThis();
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    iom->addTimer(seconds * 1000, std::bind(&sylar::IOManager::scheduler, iom, fiber));
    sylar::Fiber::YieldToHold();
}
int usleep(useconds_t usec)
{
    if (!sylar::t_hook_enable) {
        return usleep_f(usec);
    }
    sylar::Fiber::ptr fiber = sylar::Fiber::GetThis();
    sylar::IOManager* iom = sylar::IOManager::GetThis();
    iom->addTimer(usec / 1000, std::bind((void(sylar::Scheduler::*)(sylar::Fiber::ptr, int thread)) & sylar::IOManager::schedule, iom, fiber, -1));
    sylar::Fiber::YieldToHold();
    return 0;
}
}