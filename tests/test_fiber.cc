#include "sylar/sylar.h"

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void run_in_fiber()
{
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber begin";
    // After swapOut, there was no value set for the status, which resulted in this error
    // sylar::Fiber::GetThis()->swapOut();
    sylar::Fiber::YieldToHold();
    SYLAR_LOG_INFO(g_logger) << "run_in_fiber end";
    // The result before the execution of the following statement is:
    // main begin->run_in_fiber begin->main after swapIN->run_in_fiber end
    // sylar::Fiber::YieldToHold();
    // The result after executing the following statement is:
    // main begin->run_in_fiber begin->main after swapIN->run_in_fiber end->main end
    // why?
}

void test_fiber()
{
    SYLAR_LOG_INFO(g_logger) << "main begin -1";
    {
        sylar::Fiber::GetThis();
        SYLAR_LOG_INFO(g_logger) << "main begin";
        sylar::Fiber::ptr fiber(new sylar::Fiber(run_in_fiber));
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main after swapIN";
        fiber->swapIn();
        SYLAR_LOG_INFO(g_logger) << "main end";
        fiber->swapIn();
    }
    SYLAR_LOG_INFO(g_logger) << "main end 2";
}

int main(int argc, char* argv[])
{
    sylar::Thread::SetName("main");
    std::vector<sylar::Thread::ptr> thrs;
    for (int i = 0; i < 3; ++i) {
        thrs.push_back(sylar::Thread::ptr(new sylar::Thread(&test_fiber, "name_" + std::to_string(i))));
    }
    for (auto i : thrs) {
        i->join();
    }
    return 0;
}