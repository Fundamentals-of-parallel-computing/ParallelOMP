#include <omp.h>

#include <cassert>
#include <chrono>
#include <functional>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

int THREADS = 8;
int N       = 10000000;

class Printer {
public:
    explicit Printer(std::chrono::duration<double> completionTime, long result)
        : _completionTime(completionTime), _result(result) {}

    void print() const {
        std::cout << "результат: " << _result << "\n";
        std::cout << "время: " << _completionTime.count() << " seconds\n";
    }

    long get() const {
        return _result;
    }

private:
    long _result;
    std::chrono::duration<double> _completionTime;
};

std::chrono::duration<double> measureTime(
    std::function<long()> func, long& result) {
    auto start = std::chrono::high_resolution_clock::now();
    result     = func();
    auto end   = std::chrono::high_resolution_clock::now();
    return end - start;
}

class Summation {
public:
    static std::vector<int> generate(size_t n) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> dist(-100, 100);

        std::vector<int> result(n);
        for (size_t i = 0; i < n; i++) {
            result[i] = dist(gen);
        }
        return result;
    }

    static long sum(const std::vector<int>& array) {
        return std::accumulate(array.begin(), array.end(), 0L);
    }

    static long sum_parallel(const std::vector<int>& array) {
        long r = 0;
#pragma omp parallel for reduction(+ : r)
        for (size_t i = 0; i < array.size(); i++) {
            r += array[i];
        }
        return r;
    }
};

int main() {
    omp_set_num_threads(THREADS);

    std::vector<int> vec = Summation::generate(N);
    long result_serial = 0, result_parallel = 0;

    Printer serialPrinter(
        measureTime([&]() { return Summation::sum(vec); }, result_serial),
        result_serial);
    Printer parallelPrinter(
        measureTime(
            [&]() { return Summation::sum_parallel(vec); }, result_parallel),
        result_parallel);

    assert(result_serial == result_parallel);

    std::cout << "последовательно: ";
    serialPrinter.print();
    std::cout << "Параллельно: ";
    parallelPrinter.print();

    return 0;
}
