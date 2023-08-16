#include "sylar/sylar.h"
#include "sylar/util.h"
#include <assert.h>

sylar::Logger::ptr g_logger = SYLAR_LOG_ROOT();

void test_assert()
{
    SYLAR_LOG_INFO(g_logger) << sylar::BacktraceToString(10);
    SYLAR_ASSERT2(1 == 2, "hello");
}
int main(int argc, char* argv[])
{
    test_assert();
    return 0;
}
