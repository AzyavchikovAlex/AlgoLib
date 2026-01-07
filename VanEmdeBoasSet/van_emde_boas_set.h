#pragma once

#include <vector>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <cassert>
#include <optional>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace veb {

#define FORCE_INLINE __attribute__((always_inline)) inline

template<uint64_t W>
struct UIntSelector {
  using type = typename std::conditional<W <= 8, uint8_t,
                                         typename std::conditional<W <= 16,
                                                                   uint16_t,
                                                                   typename std::conditional<
                                                                       W <= 32,
                                                                       uint32_t,
                                                                       uint64_t>::type>::type>::type;
};

FORCE_INLINE int CountTrailingZeros(uint64_t mask) { return __builtin_ctzll(mask); }
FORCE_INLINE int CountLeadingZeros(uint64_t mask) { return __builtin_clzll(mask); }

template<const uint64_t WIDTH = 32, typename T = typename UIntSelector<WIDTH>::type, typename Enable = void>
class TVEBSet;

// ==========================================
// BASE CASE (WIDTH <= 8)
// ==========================================
template<const uint64_t WIDTH, typename T>
class TVEBSet<WIDTH, T, typename std::enable_if<(WIDTH <= 8)>::type> {
  static constexpr size_t
      WORDS = (1ULL << WIDTH) <= 64 ? 1 : (1ULL << (WIDTH - 6));
  uint64_t Data_[WORDS] = {0};

 public:

  TVEBSet() = default;
  explicit TVEBSet(T value) { Insert(value); }

  [[nodiscard]] FORCE_INLINE bool Empty() const {
    for (size_t i = 0; i < WORDS; ++i) {
      if (Data_[i]) return false;
    }
    return true;
  }

  FORCE_INLINE void Insert(T value) {

    Data_[value >> 6] |= (1ULL << (value & 63));
  }

  FORCE_INLINE void Erase(T value) {
    Data_[value >> 6] &= ~(1ULL << (value & 63));
  }

  [[nodiscard]] FORCE_INLINE bool Contains(T value) const {
    return (Data_[value >> 6] >> (value & 63)) & 1ULL;
  }

  [[nodiscard]] FORCE_INLINE T Min() const {
    for (size_t i = 0; i < WORDS; ++i) {
      if (Data_[i])
        return static_cast<T>((i << 6) + CountTrailingZeros(Data_[i]));
    }
    return 0;
  }

  [[nodiscard]] FORCE_INLINE T Max() const {
    for (int i = WORDS - 1; i >= 0; --i) {
      if (Data_[i])
        return static_cast<T>((i << 6) + (63 - CountLeadingZeros(Data_[i])));
    }
    return 0;
  }

  [[nodiscard]] FORCE_INLINE std::optional<T> Next(T value) const {
    auto res = NextInner(value);
    if (res == value) {
      return std::nullopt;
    }
    return res;
  }

  [[nodiscard]] FORCE_INLINE std::optional<T> Prev(T value) const {
    auto res = PrevInner(value);
    if (res == value) {
      return std::nullopt;
    }
    return res;
  }

 private:

  [[nodiscard]] FORCE_INLINE T NextInner(T value) const {
    if (value >= (1ULL << WIDTH) - 1) return value;

    size_t wordIdx = value >> 6;
    size_t bitIdx = value & 63;

    uint64_t mask = 0;
    if (bitIdx < 63) {
      mask = Data_[wordIdx] & ~((1ULL << (bitIdx + 1)) - 1);
    }

    if (mask) {
      return static_cast<T>((wordIdx << 6) + CountTrailingZeros(mask));
    }

    for (size_t i = wordIdx + 1; i < WORDS; ++i) {
      if (Data_[i]) {
        return static_cast<T>((i << 6) + CountTrailingZeros(Data_[i]));
      }
    }

    return value;
  }

  [[nodiscard]] FORCE_INLINE T PrevInner(T value) const {
    if (value == 0) return value;

    size_t wordIdx = value >> 6;
    size_t bitIdx = value & 63;

    uint64_t mask = Data_[wordIdx] & ((1ULL << bitIdx) - 1);

    if (mask) {
      return static_cast<T>((wordIdx << 6) + (63 - CountLeadingZeros(mask)));
    }

    for (int i = static_cast<int>(wordIdx) - 1; i >= 0; --i) {
      if (Data_[i]) {
        return static_cast<T>((i << 6) + (63 - CountLeadingZeros(Data_[i])));
      }
    }

    return value;
  }

  template<const uint64_t W, typename U, typename E> friend
  class TVEBSet;
};

// ==========================================
// COMMON CASE (WIDTH > 8)
// ==========================================
template<const uint64_t WIDTH, typename T>
class TVEBSet<WIDTH, T, typename std::enable_if<(WIDTH > 8)>::type> {
  static_assert(sizeof(T) * 8 >= WIDTH, "T too small");

  static constexpr uint64_t LOWER_WIDTH = WIDTH / 2;
  static constexpr uint64_t HIGHER_WIDTH = WIDTH - LOWER_WIDTH;

  TVEBSet<LOWER_WIDTH, T>* Subsets_[1ULL << HIGHER_WIDTH];
  TVEBSet<HIGHER_WIDTH, T>* SubsetsManager_ = nullptr;

  T Min_ = 0;
  T Max_ = 0;
  bool IsEmpty_ = true;

 public:
  explicit TVEBSet() {
    std::memset(Subsets_, 0, sizeof(Subsets_));
  }
  explicit TVEBSet(T value) : TVEBSet() {
    Min_ = Max_ = value;
    IsEmpty_ = false;
  }

  ~TVEBSet() {
    delete SubsetsManager_;
    for (auto ptr : Subsets_) delete ptr;
  }

  [[nodiscard]] FORCE_INLINE bool Empty() const { return IsEmpty_; }
  [[nodiscard]] FORCE_INLINE T Min() const { return Min_; }
  [[nodiscard]] FORCE_INLINE T Max() const { return Max_; }

  void Insert(T value) {
    if (Empty()) {
      IsEmpty_ = false;
      Min_ = Max_ = value;
      return;
    }
    if (value == Min_ || value == Max_) return;
    if (Min_ == Max_) {
      Min_ = std::min(Min_, value);
      Max_ = std::max(Max_, value);
      return;
    }
    if (value < Min_) std::swap(Min_, value);
    else if (value > Max_) std::swap(Max_, value);

    auto high = value >> LOWER_WIDTH;
    auto low = value & ((1ULL << LOWER_WIDTH) - 1);

    if (!Subsets_[high]) {
      Subsets_[high] = new TVEBSet<LOWER_WIDTH, T>(low);

      if (!SubsetsManager_) {
        SubsetsManager_ = new TVEBSet<HIGHER_WIDTH, T>(high);
      } else {
        SubsetsManager_->Insert(high);
      }
    } else {
      Subsets_[high]->Insert(low);
    }
  }

  void Erase(T value) {
    if (Empty()) return;
    if (Min_ == Max_) {
      if (value == Min_) {
        Min_ = Max_ = 0;
        IsEmpty_ = true;
      }
      return;
    }

    auto eraseChild = [&](T high, T low) {
      Subsets_[high]->Erase(low);
      if (Subsets_[high]->Empty()) {
        delete Subsets_[high];
        Subsets_[high] = nullptr;
        SubsetsManager_->Erase(high);
      }
    };

    if (value == Min_) {
      if (!SubsetsManager_ || SubsetsManager_->Empty()) {
        Min_ = Max_;
        return;
      }
      auto high = SubsetsManager_->Min();
      auto low = Subsets_[high]->Min();
      Min_ = (high << LOWER_WIDTH) + low;
      eraseChild(high, low);
      return;
    }

    if (value == Max_) {
      if (!SubsetsManager_ || SubsetsManager_->Empty()) {
        Max_ = Min_;
        return;
      }
      auto high = SubsetsManager_->Max();
      auto low = Subsets_[high]->Max();
      Max_ = (high << LOWER_WIDTH) + low;
      eraseChild(high, low);
      return;
    }

    auto high = value >> LOWER_WIDTH;
    auto low = value & ((1ULL << LOWER_WIDTH) - 1);
    if (Subsets_[high]) eraseChild(high, low);
  }

  [[nodiscard]] bool Contains(T value) const {
    if (Empty()) return false;
    if (value == Min_ || value == Max_) return true;
    auto child = Subsets_[value >> LOWER_WIDTH];
    return child && child->Contains(value & ((1ULL << LOWER_WIDTH) - 1));
  }

  [[nodiscard]] std::optional<T> Next(T value) const {
    auto res = NextInner(value);
    if (res > value) return res;
    return std::nullopt;
  }

  [[nodiscard]] std::optional<T> Prev(T value) const {
    auto res = PrevInner(value);
    if (res < value) return res;
    return std::nullopt;
  }

 private:
  [[nodiscard]] T NextInner(T value) const {
    if (Empty() || value >= Max_) return value;
    if (value < Min_) return Min_;

    auto high = value >> LOWER_WIDTH;
    auto low = value & ((1ULL << LOWER_WIDTH) - 1);

    auto child = Subsets_[high];
    if (child && !child->Empty() && low < child->Max()) {
      return (high << LOWER_WIDTH) + child->NextInner(low);
    }

    if (!SubsetsManager_) return Max_;
    auto nextHigh = SubsetsManager_->NextInner(high);
    if (nextHigh <= high) return Max_;

    return (nextHigh << LOWER_WIDTH) + Subsets_[nextHigh]->Min();
  }

  [[nodiscard]] T PrevInner(T value) const {
    if (Empty() || value <= Min_) return value;
    if (value > Max_) return Max_;

    auto high = value >> LOWER_WIDTH;
    auto low = value & ((1ULL << LOWER_WIDTH) - 1);

    auto child = Subsets_[high];
    if (child && !child->Empty() && low > child->Min()) {
      return (high << LOWER_WIDTH) + child->PrevInner(low);
    }

    if (!SubsetsManager_) return Min_;
    auto prevHigh = SubsetsManager_->PrevInner(high);
    if (prevHigh >= high) return Min_;

    return (prevHigh << LOWER_WIDTH) + Subsets_[prevHigh]->Max();
  }

  template<const uint64_t W, typename U, typename E> friend
  class TVEBSet;
};

} // namespace veb