#include "radix_sort.h"

#include <random>
#include <chrono>

#include <benchmark/benchmark.h>

void MeasureRadix(benchmark::State& state) {
  std::vector<int64_t> data(state.range(0));
  std::mt19937 rng(std::random_device{}());
  std::uniform_int_distribution<int64_t> dist{0, std::numeric_limits<int64_t>::max()};
  std::generate(data.begin(), data.end(), [&]() { return dist(rng); });

  for (auto s_ : state) {
    auto v = data;
    RadixSortAsc(v, [](int64_t x) {
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
    std::sort(v.begin(), v.end());
    benchmark::ClobberMemory(); // Prevents the compiler from optimizing away the copy and sort.
  }
}

BENCHMARK(MeasureRadix)->Arg(1'000);
BENCHMARK(MeasureRadix)->Arg(10'000);
BENCHMARK(MeasureRadix)->Arg(100'000);
BENCHMARK(MeasureStd)->Arg(1000);
BENCHMARK(MeasureStd)->Arg(10'000);
BENCHMARK(MeasureStd)->Arg(100'000);

BENCHMARK_MAIN();
// TEST(BEnc, t1) {
//   ::benchmark::RunSpecifiedBenchmarks();
// }
