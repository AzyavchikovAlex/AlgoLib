#include "radix_sort.h"

#include <random>
#include <chrono>
#include <vector>
#include <algorithm>
#include <numeric>

#include <benchmark/benchmark.h>

class RandomDataFixture : public benchmark::Fixture {
 public:
  std::vector<std::vector<uint64_t>> DataPool;
  size_t CurrentIndex = 0;
  static constexpr size_t POOL_SIZE = 10;

  void SetUp(const ::benchmark::State& state) override {
    size_t size = state.range(0);

    std::random_device rd;
    std::mt19937 g(rd());

    DataPool.reserve(POOL_SIZE);

    for (size_t i = 0; i < POOL_SIZE; ++i) {
      std::vector<uint64_t> temp_vec(size);
      std::iota(temp_vec.begin(), temp_vec.end(), 0);
      std::shuffle(temp_vec.begin(), temp_vec.end(), g);
      DataPool.push_back(std::move(temp_vec));
    }
  }

  void TearDown(const ::benchmark::State& state) override {
    DataPool.clear();
  }

  template<typename Sorter>
  void RunSortBenchmark(benchmark::State& state, Sorter sorter) {
    for (auto _ : state) {
      state.PauseTiming();
      // Get the next dataset from our pre-generated pool
      const auto& unsortedData = DataPool[CurrentIndex++ % POOL_SIZE];

      // Make a copy so the original remains unsorted
      auto v = unsortedData;
      state.ResumeTiming();

      // Call the specific sort function that was passed in
      sorter(v.begin(), v.end());

      // Prevent the compiler from optimizing the sort away
      benchmark::ClobberMemory();
    }
  }
};

BENCHMARK_DEFINE_F(RandomDataFixture, StdSort)(benchmark::State& state) {
  RunSortBenchmark(state, [](auto begin, auto end) {
    std::sort(begin, end);
  });
}

BENCHMARK_REGISTER_F(RandomDataFixture, StdSort)
    ->Range(2 << 14, 2 << 20)
    ->ArgNames({"ArraySize"});

BENCHMARK_DEFINE_F(RandomDataFixture, RadixSort)(benchmark::State& state) {
  RunSortBenchmark(state, [](auto begin, auto end) {
    RadixSortAsc(begin, end, [](uint64_t x) {
      return static_cast<uint64_t>(x);
    });
  });
}

BENCHMARK_REGISTER_F(RandomDataFixture, RadixSort)
    ->Range(2 << 14, 2 << 20)
    ->ArgNames({"ArraySize"});


class AlmostSortedDataFixture : public RandomDataFixture {
 public:
  void SetUp(const ::benchmark::State& state) override {
    size_t size = state.range(0);
    size_t notSortedPositionsCount = static_cast<size_t>(0.15 * size);

    std::random_device rd;
    std::mt19937 g(rd());
    assert(size > 0);
    std::uniform_int_distribution<size_t> indexesDist{0, size - 1};
    std::uniform_int_distribution<uint64_t> valuesDist{0, std::numeric_limits<uint64_t>::max()};

    DataPool.reserve(POOL_SIZE);

    for (size_t i = 0; i < POOL_SIZE; ++i) {
      std::vector<uint64_t> temp_vec(size);
      std::iota(temp_vec.begin(), temp_vec.end(), 0);

      // set to some random positions random values
      for (size_t j = 0; j < notSortedPositionsCount; ++j) {
        size_t pos = indexesDist(g);
        temp_vec[pos] = valuesDist(g);
      }
      DataPool.push_back(std::move(temp_vec));
    }
  }
};

BENCHMARK_DEFINE_F(AlmostSortedDataFixture, StdSort)(benchmark::State& state) {
  RunSortBenchmark(state, [](auto begin, auto end) {
    std::sort(begin, end);
  });
}

BENCHMARK_REGISTER_F(AlmostSortedDataFixture, StdSort)
    ->Range(2 << 14, 2 << 20)
    ->ArgNames({"ArraySize"});

BENCHMARK_DEFINE_F(AlmostSortedDataFixture, RadixSort)(benchmark::State& state) {
  RunSortBenchmark(state, [](auto begin, auto end) {
    RadixSortAsc(begin, end, [](uint64_t x) {
      return static_cast<uint64_t>(x);
    });
  });
}

BENCHMARK_REGISTER_F(AlmostSortedDataFixture, RadixSort)
    ->Range(2 << 14, 2 << 20)
    ->ArgNames({"ArraySize"});

BENCHMARK_MAIN();
