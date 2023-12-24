#include "thread_utils.h"
#include <thread>

auto dummyFunction(int a, int b, bool sleep) {
  std::cout << "dummyFunction( " << a << ", " << b << ")" << std::endl;
  std::cout << "output:=" << a + b << std::endl;
  if (sleep) {
    std::cout << "dummyFunction sleeping..." << std::endl;
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(20s);
  }
  std::cout << "dummyFunction done" << std::endl;
}

int main() {

  using namespace Common;
  auto t1 =
      creatAndStartThread(-1, "dummyFunction1", dummyFunction, 12, 21, false);
  auto t2 =
      creatAndStartThread(1, "dummyFunction2", dummyFunction, 1, 21, true);
  t1->join();
  t2->join();

  return 0;
}