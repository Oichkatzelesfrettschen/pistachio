#include <unsupported/Eigen/CXX23/SpinLock>
#include <thread>
#include <vector>
#include <fstream>

int main() {
    Eigen::cxx23::SpinLock lock;
    int counter = 0;
    constexpr int loops = 1000;
    std::vector<std::thread> threads;
    for(int i = 0; i < 4; ++i) {
        threads.emplace_back([&]() {
            for(int j = 0; j < loops; ++j) {
                Eigen::cxx23::SpinLockGuard g(lock);
                ++counter;
            }
        });
    }
    for(auto &t : threads) t.join();
    std::ofstream("/tmp/spinlock_out.txt") << counter << "\n";
    return 0;
}
