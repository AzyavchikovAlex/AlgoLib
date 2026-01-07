#include <benchmark/benchmark.h>
#include <set>
#include <vector>
#include <random>
#include <algorithm>
#include <memory>

#include "van_emde_boas_set.h"

using namespace veb;

constexpr int SEED = 42;

std::vector<uint32_t> GenerateRandomData(size_t size) {
  std::vector<uint32_t> data(size);
  std::mt19937 gen(SEED);
  std::uniform_int_distribution<uint32_t> dist(0, std::numeric_limits<uint32_t>::max());
  for (auto& x : data) {
    x = dist(gen);
  }
  return data;
}

template<class Container>
static void BM_Insert(benchmark::State& state) {
  const size_t size = state.range(0);
  auto data = GenerateRandomData(size);

  for (auto _ : state) {
    state.PauseTiming();
    Container c;
    state.ResumeTiming();

    for (auto x : data) {
      c.Insert(x);
    }
  }
  state.SetItemsProcessed(state.iterations() * size);
}

template<>
void BM_Insert<std::set<uint32_t>>(benchmark::State& state) {
  const size_t size = state.range(0);
  auto data = GenerateRandomData(size);

  for (auto _ : state) {
    state.PauseTiming();
    std::set<uint32_t> c;
    state.ResumeTiming();

    for (auto x : data) {
      c.insert(x);
    }
  }
  state.SetItemsProcessed(state.iterations() * size);
}

template<class Container>
static void BM_Contains(benchmark::State& state) {
  const size_t size = state.range(0);
  auto data = GenerateRandomData(size);
  auto queries =
      GenerateRandomData(size);

  Container c;
  for (auto x : data) {
    if constexpr (std::is_same_v<Container, std::set<uint32_t>>) c.insert(x);
    else c.Insert(x);
  }

  for (auto _ : state) {
    for (auto q : queries) {
      if constexpr (std::is_same_v<Container,
                                   std::set<uint32_t>>)
        benchmark::DoNotOptimize(c.count(q));
      else benchmark::DoNotOptimize(c.Contains(q));
    }
  }
  state.SetItemsProcessed(state.iterations() * size);
}

template<class Container>
static void BM_Next(benchmark::State& state) {
  const size_t size = state.range(0);
  auto data = GenerateRandomData(size);
  auto queries = GenerateRandomData(size);

  Container c;
  for (auto x : data) {
    if constexpr (std::is_same_v<Container, std::set<uint32_t>>) c.insert(x);
    else c.Insert(x);
  }

  for (auto _ : state) {
    for (auto q : queries) {
      if constexpr (std::is_same_v<Container, std::set<uint32_t>>) {
        auto it = c.upper_bound(q);
        if (it != c.end()) benchmark::DoNotOptimize(*it);
      } else {
        benchmark::DoNotOptimize(c.Next(q));
      }
    }
  }
  state.SetItemsProcessed(state.iterations() * size);
}

#define REGISTER_BM(Func, Name, Type) \
    BENCHMARK_TEMPLATE(Func, Type)->Name(Name)->RangeMultiplier(4)->Range(1<<10, 1<<18);
using StdSet = std::set<uint32_t>;
using VEB32 = TVEBSet<32>;

// 1. Insert
BENCHMARK_TEMPLATE(BM_Insert, StdSet)
    ->Name("StdSet_Insert")
    ->RangeMultiplier(2)->Range(1 << 12, 1 << 21);

BENCHMARK_TEMPLATE(BM_Insert, VEB32)
    ->Name("TVEBSet_Insert")
    ->RangeMultiplier(2)->Range(1 << 12, 1 << 21);

// 2. Contains
BENCHMARK_TEMPLATE(BM_Contains, StdSet)
    ->Name("StdSet_Contains")
    ->RangeMultiplier(2)->Range(1 << 12, 1 << 21);

BENCHMARK_TEMPLATE(BM_Contains, VEB32)
    ->Name("TVEBSet_Contains")
    ->RangeMultiplier(2)->Range(1 << 12, 1 << 21);

// 3. Next (Upper Bound)
BENCHMARK_TEMPLATE(BM_Next, StdSet)
    ->Name("StdSet_Next")
    ->RangeMultiplier(2)->Range(1 << 10, 1 << 21);

BENCHMARK_TEMPLATE(BM_Next, VEB32)
    ->Name("TVEBSet_Next")
    ->RangeMultiplier(2)->Range(1 << 10, 1 << 21);

BENCHMARK_MAIN();