#include "radix_sort.h"

#include <random>
#include <chrono>
#include <algorithm>

#include <gtest/gtest.h>

std::mt19937_64 gen(time(nullptr));
std::uniform_int_distribution<int64_t>
    distrib(0, std::numeric_limits<int64_t>::max());

std::vector<int64_t> GenerateArray(int size,
                                   int64_t max = std::numeric_limits<int>::max()) {
  std::vector<int64_t> answer;
  for (int i = 0; i < size; ++i) {
    int64_t value = distrib(gen) % max;
    answer.push_back(value);
  }
  return answer;
}

void TestSortCorrectness(size_t arraySize,
                         int64_t maxValue,
                         size_t iterationsCount) {
  for (size_t i = 0; i < iterationsCount; ++i) {
    auto array = GenerateArray(arraySize, maxValue);
    auto sortedArray = array;
    std::sort(sortedArray.begin(), sortedArray.end());
    RadixSortAsc(array, [](int64_t x) {
      return static_cast<uint64_t>(x);
    });
    ASSERT_EQ(array, sortedArray);
  }
}

TEST(RadixSortCorrectness, Small) {
  const size_t n = 20;
  const int64_t maxValue = 100;
  const int iterationsCount = 1000;
  TestSortCorrectness(n, maxValue, iterationsCount);
}

TEST(RadixSortCorrectness, Medium) {
  const size_t n = 100;
  const int64_t maxValue = 1000;
  const int iterationsCount = 200;
  TestSortCorrectness(n, maxValue, iterationsCount);
}

TEST(RadixSortCorrectness, Large) {
  const size_t n = 100'000;
  const int64_t maxValue = std::numeric_limits<int64_t>::max();
  const int iterationsCount = 20;
  TestSortCorrectness(n, maxValue, iterationsCount);
}
