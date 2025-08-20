#include "radix_sort.h"

#include <random>
#include <chrono>

#include <benchmark/benchmark.h>

void MeasureRadix(benchmark::State& state) {
  std::vector<int64_t> data(state.range(0));
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int64_t> dist{0, std::numeric_limits<int64_t>::max()};
  std::generate(data.begin(), data.end(), [&]() { return dist(rng); });

  for (auto _ : state) {
    auto v = data;
    RadixSortAsc(v.begin(), v.end(), [](int64_t x) {
      return x;
    });
    benchmark::ClobberMemory(); // Prevents the compiler from optimizing away the copy and sort.
  }
}

void MeasureStd(benchmark::State& state) {
  std::vector<int64_t> data(state.range(0));
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int64_t> dist{0, std::numeric_limits<int64_t>::max()};
  std::generate(data.begin(), data.end(), [&]() { return dist(rng); });

  // The main benchmark loop. Timing starts automatically here.
  for (auto _ : state) {
    // --- TIMED CODE ---
    // Create a copy to avoid timing the setup of already sorted data in subsequent iterations.
    auto v = data;
    std::sort(v.begin(), v.end(), [](auto& x, auto& y) {
      return x < y;
    });
    benchmark::ClobberMemory(); // Prevents the compiler from optimizing away the copy and sort.
  }
}

BENCHMARK(MeasureRadix)->Arg(1 << 11);
BENCHMARK(MeasureStd)->Arg(1 << 11);
BENCHMARK(MeasureRadix)->Arg(1 << 12);
BENCHMARK(MeasureStd)->Arg(1 << 12);
BENCHMARK(MeasureRadix)->Arg(1 << 13);
BENCHMARK(MeasureStd)->Arg(1 << 13);
BENCHMARK(MeasureRadix)->Arg(1 << 14);
BENCHMARK(MeasureStd)->Arg(1 << 14);
BENCHMARK(MeasureRadix)->Arg(1 << 15);
BENCHMARK(MeasureStd)->Arg(1 << 15);
BENCHMARK(MeasureRadix)->Arg(1 << 16);
BENCHMARK(MeasureStd)->Arg(1 << 16);
BENCHMARK(MeasureRadix)->Arg(1 << 17);
BENCHMARK(MeasureStd)->Arg(1 << 17);
BENCHMARK(MeasureRadix)->Arg(1 << 18);
BENCHMARK(MeasureStd)->Arg(1 << 18);
BENCHMARK(MeasureRadix)->Arg(1 << 19);
BENCHMARK(MeasureStd)->Arg(1 << 19);
BENCHMARK(MeasureRadix)->Arg(1 << 20);
BENCHMARK(MeasureStd)->Arg(1 << 20);
BENCHMARK(MeasureRadix)->Arg(1 << 21);
BENCHMARK(MeasureStd)->Arg(1 << 21);
BENCHMARK(MeasureRadix)->Arg(1 << 22);
BENCHMARK(MeasureStd)->Arg(1 << 22);
BENCHMARK(MeasureRadix)->Arg(1 << 23);
BENCHMARK(MeasureStd)->Arg(1 << 23);


BENCHMARK_MAIN();
