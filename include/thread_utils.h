#pragma once
#include <atomic>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <pthread.h>
#include <sched.h>
#include <sys/syscall.h>
#include <thread>
#include <unistd.h>
#include <utility>

namespace Common {
inline auto setThreadCore(int core_id) noexcept {
  cpu_set_t cpuset;  // represent a set of cpus
  CPU_ZERO(&cpuset); // clear cpuset variable
  CPU_SET(core_id, &cpuset);
  return (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) ==
          0);
}

template <typename T, typename... A>
inline auto creatAndStartThread(int core_id, const std::string &name, T &&func,
                                A &&...args) noexcept {
  std::atomic<bool> running(false), failed(false);
  auto thread_body = [&] {
    if (core_id >= 0 && !setThreadCore(core_id)) {
      std::cerr << "Failed to set core affinity for " << name << " "
                << pthread_self() << " to " << core_id << std::endl;
      failed = true;
      return;
    }
    std::cout << "Set core affinity for " << name << " " << pthread_self()
              << " to " << core_id << std::endl;
    running = true;
    std::forward<T>(func)((std::forward<A>(args))...);
  };
  auto t = new std::thread(thread_body);
  while (!running &&
         !failed) { // until thread started running or has failed to start
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(1s);
  }
  if (failed) {
    t->join();
    delete t;
    t = nullptr;
  }
  return t;
};

} // namespace Common
