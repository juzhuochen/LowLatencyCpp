#include "thread_utils.h"
#include "lf_queue.h"
// #include <iostream>
struct Mystruct {
    int m_data[3];
};

using namespace Common;

auto consumeFunc(LFQueue<Mystruct> *lfq) {
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(1s);
    while (lfq->size()) { //read from lfq
        const auto *elem = lfq->getNextToread();
        lfq->updateReadIndex();
        std::cout << "consumeFunc read elememt: " << elem->m_data[0] << ", "
                  << elem->m_data[1] << ", " << elem->m_data[2]
                  << " queue size is: " << lfq->size() << "\n";
        std::this_thread::sleep_for(2s);
    }
    std::cout << "consumeFunc exiting"
              << "\n";
}

int main() {
    LFQueue<Mystruct> lfq(20);
    auto *testThread = creatAndStartThread(-1, "", consumeFunc, &lfq);
    for (auto i = 0; i < 100; ++i) {
        const Mystruct test{i, i * 5, i * 10};
        *(lfq.getNextToWriteTo()) = test;
        lfq.updateWriteIndex();
        std::cout << "main constructed elements:" << test.m_data[0] << ", "
                  << test.m_data[1] << ", " << test.m_data[2]
                  << " queue size: " << lfq.size() << "\n";
        using namespace std::literals::chrono_literals;
        std::this_thread::sleep_for(1s);
    }

    testThread->join();
    std::cout << "\n\n main exiting\n\n";
    return 0;
}