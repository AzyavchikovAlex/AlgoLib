#pragma once

#pragma once

#include <vector>
#include <cstdint>
#include <stddef.h>
#include <limits>


// warning: not thread safe!
template<typename T, typename K>
void RadixSortAsc(std::vector<T> &values, K getRadixIndex) {
  constexpr size_t REMINDER_WIDTH = 16;
  constexpr uint64_t BUCKETS_COUNT = (static_cast<uint64_t>(1) << REMINDER_WIDTH);
  constexpr size_t ITERATIONS_COUNT = (64 / (REMINDER_WIDTH)) + (64 % (REMINDER_WIDTH) == 0 ? 0 : 1);
  static std::vector<size_t> buckets(BUCKETS_COUNT);

  const size_t valuesCount = values.size();
  static std::vector<T> buffer;
  buffer.resize(valuesCount);

  for (size_t iteration = 0; iteration < ITERATIONS_COUNT; ++iteration) {
    const uint64_t shift = iteration * REMINDER_WIDTH;
    const uint64_t reminderGetter = (BUCKETS_COUNT - 1) << shift;
    for (size_t i = 0; i < BUCKETS_COUNT; ++i) {
      buckets[i] = 0;
    }
    size_t max = 0;
    for (size_t i = 0; i < valuesCount; ++i) {
      max = ++buckets[((getRadixIndex(values[i]) & reminderGetter) >> shift)];
    }
    if (max == valuesCount) {
      continue; // already sorted
    } {
      size_t bufferCount = 0;
      for (size_t i = 0, prevCount = 0; i < BUCKETS_COUNT; ++i) {
        bufferCount = buckets[i];
        buckets[i] = prevCount;
        prevCount += bufferCount;
      }
    }

    for (size_t i = 0; i < valuesCount; ++i) {
      buffer[buckets[((getRadixIndex(values[i]) & reminderGetter) >> shift)]++] = values[i];
    }
    std::swap(buffer, values);
  }
}

template<typename T, typename K>
void RadixSortDesc(std::vector<T> &values, K getRadixIndex) {
  static constexpr uint64_t maxRadixIndex = std::numeric_limits<uint64_t>::max();
  RadixSortAsc(values, [&getRadixIndex](const T &value) {
    return maxRadixIndex - getRadixIndex(value);
  });
}