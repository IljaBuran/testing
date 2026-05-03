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
using u32 = uint32_t;
using u64 = uint64_t;

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

static i64 sequential_check(const std::vector<i32>& v)
{
    return (i64)std::accumulate(v.begin(), v.end(), (i64)0);
}


static void worker_thread(const std::vector<i32>& vec, u64 begin, u64 end, i64& result)
{
    result = std::accumulate(vec.begin() + begin, vec.begin() + end, (i64)0);
}

constexpr i32 numberOfThreads = 16;

int main()
{
    std::vector<i32> v;

    get_time(start);
    remove_file("random.bin");
    generate_random_data_file("random.bin", 1'000, 100'000'000);
    get_time(end);
    std::cout << "generate_random_data: " << get_duration() << " ms\n";

    get_time(start);

    feed_queue_from_data_file("random.bin", v);
    get_time(end);
    std::cout << "feed_queue_from_file: " << get_duration() << " ms\n";
    do
    {
        // threaded
        get_time(start);
        u64 chunkSize = v.size() / numberOfThreads;

        i64 res_threaded = 0;
        std::vector<i64> partials(numberOfThreads, 0);
        std::vector<std::thread> threads;

        for (u32 i = 0; i < numberOfThreads; i++)
        {
            u64 begin = i * chunkSize;
            u64 end = i == (numberOfThreads - 1) ? v.size() : begin + chunkSize;

            threads.emplace_back(worker_thread, std::cref(v), begin, end, std::ref(partials[i]));
        }

        for (auto& thread : threads)
            thread.join();

        res_threaded = std::accumulate(partials.begin(), partials.end(), (i64)0);

        get_time(end);
        std::cout << "threading_check: " << get_duration() << " ms\n";
        std::cout << "THREADING RESULT = " << res_threaded << '\n';

        // sequential
        get_time(start);
        i64 res_seq = sequential_check(v);
        get_time(end);
        std::cout << "sequential_check: " << get_duration() << " ms\n";
        std::cout << "SEQUENTIAL RESULT = " << res_seq << '\n';

        if (res_seq != res_threaded)
            throw std::runtime_error("Failed");
    } while(0);
    return 0;
}
