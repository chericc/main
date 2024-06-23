#include <Public/DPProducerConsumer.h>

#include <chrono>
#include <condition_variable>
#include <future>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

/*
 * Producer and Consumer model.
 */

PRIVATE_BEGIN

class Container {
   public:
    void Add(const std::string& s) {
        std::unique_lock<std::mutex> ul(m_mutex);
        while (m_vecStrs.size() >= m_nlimit) {
            m_condnotfull.wait(ul);
        }
        m_vecStrs.push_back(s);
        m_condnotempty.notify_all();
    }
    std::string Pop() {
        std::unique_lock<std::mutex> ul(m_mutex);
        while (m_vecStrs.size() == 0) {
            m_condnotempty.wait(ul);
        }
        std::string s = m_vecStrs.back();
        m_vecStrs.pop_back();
        m_condnotfull.notify_all();
        return s;
    }

   private:
    static constexpr int m_nlimit = 5;
    std::vector<std::string> m_vecStrs;
    std::mutex m_mutex;
    std::condition_variable m_condnotfull;
    std::condition_variable m_condnotempty;
};

Container g_c;

int producer() {
    std::cout << "producer thread start!" << std::endl;
    while (true) {
        std::string s;
        std::cin >> s;
        std::cout << "[producer]prepare add " << s << std::endl;
        g_c.Add(s);
        std::cout << "[producer]add " << s << " finished" << std::endl;
    }
}

int consumer() {
    std::cout << "consumer thread start!" << std::endl;
    while (true) {
        std::string s;
        std::cout << "[consumer]prepare pop" << std::endl;
        s = g_c.Pop();
        std::cout << "[consumer]pop " << s << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    }
}

void Example() {
    std::future<int> futureconsumer(std::async(std::launch::async, consumer));
    std::future<int> futureproducer(std::async(std::launch::async, producer));

    futureconsumer.get();
    futureproducer.get();

    return;
}

PRIVATE_END

DP_BEGIN

void DPProducerConsumer::Play() { Example(); }

DP_END