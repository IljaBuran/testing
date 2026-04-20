#include <iostream>
#include <random>
#include <fstream>
#include <stdexcept>
#include <stdint.h>
#include <vector>
#include <filesystem>
#include <thread>
#include <chrono>
#include <mutex>
#include <numeric>

using i64 = int64_t;
using i32 = int32_t;

std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

void get_time(std::chrono::time_point<std::chrono::high_resolution_clock>& hrc)
{
    hrc = std::chrono::high_resolution_clock::now();
}

i64 get_duration()
{
    auto dur = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    return dur.count();
}

static i32 generate_random_i32(i32 upperBoundary = 10'000)
{
    static std::random_device dev;
    static std::mt19937 rng(dev());
    std::uniform_int_distribution<i32> dist(1, upperBoundary);

    return (i32)dist(rng);
}

static void remove_file(const char* fileName)
{
    if (std::filesystem::exists(fileName))
        std::filesystem::remove(fileName);
}

static void generate_random_data_file(const char* fileName, i32 upperBoundary = 10'000, i32 dataCount = 10'000)
{
    std::ofstream f(fileName, std::ios::binary);

    if (!f.is_open())
        throw std::runtime_error("The file could not be opened");

    for (i32 i = 0; i < dataCount; i++)
    {
        i32 val = generate_random_i32(upperBoundary);
        f.write((const char*)&val, sizeof(val));
    }
}

static void feed_queue_from_data_file(const char* fileName, std::vector<i32>& v)
{
    std::ifstream f(fileName, std::ios::binary);

    if (!f.is_open())
        throw std::runtime_error("The file could not be opened");

    i32 num;

    while (f.read((char*)&num, sizeof(num)))
        v.emplace_back(num);
}

static i64 sequential_check(std::vector<i32>& v)
{
    return (i64)std::accumulate(v.begin(), v.end(), (i64)0);
}


std::mutex m;
std::vector<i32> v1;
std::vector<i32> v2;

void multithreaded_fn()
{
    return std::accumulate(v)
}

constexpr i32 numberOfThreads = 2;

int main()
{
    std::vector<std::thread> threads;
    i64 res_threaded = 0;

    get_time(start);
    remove_file("random.bin");
    generate_random_data_file("random.bin", 1'000, 100'000'000);
    get_time(end);
    std::cout << "generate_random_data: " << get_duration() << " ms\n";

    get_time(start);

    feed_queue_from_data_file("random.bin", v1);
    get_time(end);
    std::cout << "feed_queue_from_file: " << get_duration() << " ms\n";

    // threaded

    get_time(start);

    get_time(end);
    std::cout << "feed_queue_from_file: " << get_duration() << " ms\n";

    // sequential
    get_time(start);
    i64 res_seq = sequential_check(v1);
    get_time(end);
    std::cout << "sequential_check: " << get_duration() << " ms\n";
    std::cout << "SEQUENTIAL RESULT = " << res_seq << '\n';
    return 0;
}
