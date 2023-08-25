#include "util.h"
#include "fiber.h"
#include "log.h"
#include <execinfo.h>
#include <sys/syscall.h>

namespace sylar {

Logger::ptr g_logger = SYLAR_LOG_NAME("system");

pid_t GetThreadId() { return syscall(SYS_gettid); }
uint64_t GetFiberId() { return Fiber::GetFiberId(); }

void Backtrace(std::vector<std::string>& bt, int size, int skip)
{
    // 分配一个大小为size的数组
    void** array = (void**)malloc((sizeof(void*) * size));
    // 获取backtrace函数的返回值
    size_t s = ::backtrace(array, size);

    // 获取backtrace_symbols函数的返回值
    char** strings = ::backtrace_symbols(array, s);
    if (strings == NULL) {
        // 若backtrace_symbols函数的返回值为NULL，则输出错误信息
        // 在这里可以直接返回，不需要释放array内存，因为它没有被传递给其他地方
        SYLAR_LOG_ERROR(g_logger) << "backtrace_symbols error";
        return;
    }
    // 将backtrace_symbols函数的返回值添加到bt中
    for (size_t i = skip; i < s; ++i) {
        bt.push_back(strings[i]);
    }
    // 释放backtrace_symbols函数的返回值
    free(strings);
    // 释放backtrace函数的返回值
    free(array);
}

std::string BacktraceToString(int size, int skip, const std::string& prefix)
{
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for (size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

uint64_t GetCurrentMS()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

uint64_t GetCurrentUS()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

} // namespace sylar
