#include "radix_sort.h"

#include <random>
#include <chrono>
#include <algorithm>

#include <gtest/gtest.h>

std::mt19937_64 gen(time(nullptr));
std::uniform_int_distribution<uint64_t>
    distrib(0, std::numeric_limits<uint64_t>::max());

std::vector<uint64_t> GenerateArray(size_t size,
                                    uint64_t max = std::numeric_limits<int>::max()) {
  std::vector<uint64_t> answer;
  for (int i = 0; i < size; ++i) {
    uint64_t value = distrib(gen) % max;
    answer.push_back(value);
  }
  return answer;
}

void TestAscSortCorrectness(size_t arraySize,
                            uint64_t maxValue,
                            size_t iterationsCount) {
  for (size_t i = 0; i < iterationsCount; ++i) {
    auto array = GenerateArray(arraySize, maxValue);
    auto sortedArray = array;
    std::sort(sortedArray.begin(), sortedArray.end());
    RadixSortAsc(array.begin(), array.end(), [](int64_t x) {
      return static_cast<uint64_t>(x);
    });
    ASSERT_EQ(array, sortedArray);
  }
}

void TestDescSortCorrectness(size_t arraySize,
                             uint64_t maxValue,
                             size_t iterationsCount) {
  for (size_t i = 0; i < iterationsCount; ++i) {
    auto array = GenerateArray(arraySize, maxValue);
    auto sortedArray = array;
    std::sort(sortedArray.begin(), sortedArray.end(), std::greater<uint64_t>{});
    RadixSortDesc(array.begin(), array.end(), [](int64_t x) {
      return static_cast<uint64_t>(x);
    });
    ASSERT_EQ(array, sortedArray);
  }
}


TEST(RadixSortCorrectness, Small) {
  const size_t n = 40;
  const uint64_t maxValue = uint64_t(1) << 63;
  const int iterationsCount = 5000;
  TestAscSortCorrectness(n, maxValue, iterationsCount);
}

TEST(RadixSortCorrectness, Medium1) {
  const size_t n = 1000;
  const uint64_t maxValue = uint64_t(1) << 63;
  const int iterationsCount = 200;
  TestAscSortCorrectness(n, maxValue, iterationsCount);
}

TEST(RadixSortCorrectness, Medium2) {
  const size_t n = 10000;
  const uint64_t maxValue = uint64_t(1) << 63;
  const int iterationsCount = 200;
  TestAscSortCorrectness(n, maxValue, iterationsCount);
}

TEST(RadixSortDescCorrectness, Medium) {
  const size_t n = 10000;
  const uint64_t maxValue = uint64_t(1) << 63;
  const int iterationsCount = 200;
  TestDescSortCorrectness(n, maxValue, iterationsCount);
}

TEST(RadixSortCorrectness, Large) {
  const size_t n = 200'000;
  const int64_t maxValue = std::numeric_limits<int64_t>::max();
  const int iterationsCount = 25;
  TestAscSortCorrectness(n, maxValue, iterationsCount);
}

TEST(RadixSortCorrectness, UserDataStructure) {
  const size_t n = 100;
  struct Point {
    uint32_t x, y;

    bool operator==(const Point& p) const {
      return x == p.x && y == p.y;
    }
  };

  auto getPointRadixIndex = [](const Point& p) {
    return (uint64_t(p.x) << 32) + p.y;
  };
  const int iterationsCount = 200;

  auto testSortCorrectness = [&getPointRadixIndex](size_t n) {
    auto x = GenerateArray(n, std::numeric_limits<uint32_t>::max());
    auto y = GenerateArray(n, std::numeric_limits<uint32_t>::max());
    std::vector<Point> data;
    for (int i = 0; i < n; ++i) {
      data.push_back(Point{
          .x = static_cast<uint32_t>(x[i]),
          .y = static_cast<uint32_t>(y[i]),
      });
    }
    auto expectedData = data;

    RadixSortAsc(data.begin(), data.end(), getPointRadixIndex);
    std::sort(expectedData.begin(), expectedData.end(), [](auto x, auto y) {
      if (x.x == y.y) {
        return x.y < y.y;
      }
      return x.x < y.x;
    });

    EXPECT_EQ(data, expectedData);
  };

  for (int test = 0; test < iterationsCount; ++test) {
    testSortCorrectness(n);
  }
}
