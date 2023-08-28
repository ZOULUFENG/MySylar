#pragma once

namespace sylar {
bool is_hook_enabled();
void set_hook_enable(bool flag);

} // namespace sylar

extern "C" {

// sleep
typedef unsigned int (*sleep_fun)(unsigned int seconds);
extern sleep_fun sleep_f;

typedef int (*usleep_fun)(useconds_t usec);
extern usleep_fun usleep_f;

typedef int (*nanosleep_fun)(const struct timespec* req, struct timespec* rem);
extern nanosleep_fun nanosleep_f;

}
