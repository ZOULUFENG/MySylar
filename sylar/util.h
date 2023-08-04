#pragma once
// #include <sys/types.h>
#include <unistd.h>

namespace sylar {

pid_t GetThreadId();
unsigned int GetFiberId();

}  // namespace sylar
