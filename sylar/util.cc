#include "util.h"

#include <sys/syscall.h>
namespace sylar {
pid_t GetThreadId() { return syscall(SYS_gettid); }
unsigned int GetFiberId() { return 0; }
}  // namespace sylar
