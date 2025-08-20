#pragma once

#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <cstddef>
#include <limits>
#include <cstring>
#include <tuple>
#include <algorithm>

template<
    size_t WIDTH = 8,
    uint64_t REMINDER_GETTER = (uint64_t(1 << WIDTH) - 1)
>
void RadixSort(auto containerBegin,
               auto containerEnd,
               const auto& getRadixIndex) {
  static constexpr size_t radixIndexWidth = 64;
  static constexpr size_t bucketsCount = 1 << WIDTH;
  static constexpr size_t minArraySize = bucketsCount;

  auto sortComparator = [&getRadixIndex](const auto& x, const auto& y) {
    return getRadixIndex(x) < getRadixIndex(y);
  };
  std::array<size_t, bucketsCount> bucketsBegin, bucketsInsertIndex;
  std::vector<std::tuple<decltype(containerBegin),
                         decltype(containerEnd),
                         size_t>> segments;
  segments.emplace_back(containerBegin, containerEnd, radixIndexWidth - WIDTH);

  while (!segments.empty()) {
    auto [begin, end, shift] = segments.back();
    segments.pop_back();
    // don't sort too small arrays
    if (std::distance(begin, end) < minArraySize) {
      std::sort(begin, end, sortComparator);
      continue;
    }

    auto getBucketIndex = [&getRadixIndex, shift](const auto& it) -> size_t {
      return (getRadixIndex(*it) >> shift) & REMINDER_GETTER;
    };

    // distribute values over buckets
    memset(bucketsBegin.data(), 0, sizeof(bucketsBegin));
    for (auto it = begin; it != end; ++it) {
      ++bucketsBegin[getBucketIndex(it)];
    }
    if (uint64_t index = getBucketIndex(begin);
        bucketsBegin[index] == std::distance(begin, end)) {
      if (shift > 0) {
        segments.emplace_back(begin, end, shift - WIDTH);
      }
      continue;
    }
    size_t prevCount = 0;
    size_t count = 0;
    for (size_t i = 0; i < bucketsCount; ++i) {
      count = bucketsBegin[i];
      bucketsInsertIndex[i] = bucketsBegin[i] = prevCount;
      prevCount += count;
    }

    // put each element in it bucket
    for (auto it = begin; it != end;) {
      size_t bucketIndex = getBucketIndex(it);
      size_t& insertIndex = bucketsInsertIndex[bucketIndex];
      const size_t index = std::distance(begin, it);
      if (index == insertIndex) {
        ++it;
        ++insertIndex;
      } else if (index >= bucketsBegin[bucketIndex] && index < insertIndex) {
        ++it;
      } else {
        std::swap(*it, *std::next(begin, insertIndex));
        ++insertIndex;
      }
    }

    if (shift == 0) {
      continue;
    }

    for (size_t i = 0; i < bucketsCount; ++i) {
      int l = bucketsBegin[i];
      int r = bucketsInsertIndex[i];
      if (r - l < minArraySize) {
        std::sort(std::next(begin, l),
                  std::next(begin, r),
                  sortComparator);
      } else {
        segments.emplace_back(std::next(begin, l),
                              std::next(begin, r),
                              shift - WIDTH);
      }
    }
  }
}

void RadixSortAsc(auto begin, auto end, auto&& getRadixIndex) {
  constexpr size_t optimalStdSortArraySize = 1 << 12;

  if (std::distance(begin, end) >= optimalStdSortArraySize) {
    RadixSort<8>(begin, end, getRadixIndex);
  } else {
    std::sort(begin, end, [&getRadixIndex](const auto& x, const auto& y) {
      return getRadixIndex(x) < getRadixIndex(y);
    });
  }
}

void RadixSortDesc(auto begin, auto end, auto&& getRadixIndex) {
  static constexpr uint64_t maxIndex = std::numeric_limits<uint64_t>::max();

  RadixSortAsc(begin, end, [&getRadixIndex](const auto& value) {
    return maxIndex - getRadixIndex(value);
  });
}