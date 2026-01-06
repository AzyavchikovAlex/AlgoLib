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
#include <map>

namespace veb {

namespace {

// ==========================================
// POOL ALLOCATOR
// ==========================================
template<size_t ObjectSize, size_t ChunkSize = 16384>
class PoolAllocator {
  struct Node {
    Node* next;
  };

  static Node* FreeHead;
  static char* CurrentChunk;
  static size_t CurrentOffset;
  static std::vector<char*> Chunks;

 public:
  static void* allocate(size_t n) {
    if (n != ObjectSize) {
      return ::operator new(n);
    }
    if (FreeHead) {
      void* ptr = FreeHead;
      FreeHead = FreeHead->next;
      return ptr;
    }
    if (!CurrentChunk || CurrentOffset + ObjectSize > ChunkSize) {
      CurrentChunk = new char[ChunkSize];
      Chunks.push_back(CurrentChunk);
      CurrentOffset = 0;
    }
    void* ptr = CurrentChunk + CurrentOffset;
    CurrentOffset += ObjectSize;
    return ptr;
  }

  static void deallocate(void* ptr, size_t n) {
    if (n != ObjectSize) {
      ::operator delete(ptr);
      return;
    }
    if (!ptr) return;
    Node* node = static_cast<Node*>(ptr);
    node->next = FreeHead;
    FreeHead = node;
  }

  static void cleanup() {
    for (char* chunk : Chunks) delete[] chunk;
    Chunks.clear();
    FreeHead = nullptr;
    CurrentChunk = nullptr;
    CurrentOffset = 0;
  }
};

template<size_t S, size_t C> typename PoolAllocator<S, C>::Node
    * PoolAllocator<S, C>::FreeHead = nullptr;
template<size_t S, size_t C> char* PoolAllocator<S, C>::CurrentChunk = nullptr;
template<size_t S, size_t C> size_t PoolAllocator<S, C>::CurrentOffset = 0;
template<size_t S, size_t C> std::vector<char*> PoolAllocator<S, C>::Chunks;

#define FORCE_INLINE __attribute__((always_inline)) inline

// ==========================================
// ВСПОМОГАТЕЛЬНЫЕ СТРУКТУРЫ
// ==========================================

template<uint64_t W>
struct UIntSelector {
  using type =
      typename std::conditional<
          W <= 8, uint8_t,
          typename std::conditional<
              W <= 16,
              uint16_t,
              typename std::conditional<W <= 32, uint32_t, uint64_t>::type
          >::type
      >::type;
};

FORCE_INLINE int CountTrailingZeros(uint64_t mask) {
  return __builtin_ctzll(mask);
}

FORCE_INLINE int CountLeadingZeros(uint64_t mask) {
  return __builtin_clzll(mask);
}

}

template<const uint64_t WIDTH = 32, typename T = typename UIntSelector<WIDTH>::type, typename Enable = void>
class TVEBSet;

// ==========================================
// BASE CASE (WIDTH <= 4)
// ==========================================
template<const uint64_t WIDTH, typename T>
class TVEBSet<WIDTH, T, typename std::enable_if<(WIDTH <= 4)>::type> {
  uint64_t Data_ = 0;

 public:
  static void* operator new(size_t size) {
    return PoolAllocator<sizeof(TVEBSet)>::allocate(size);
  }
  static void operator delete(void* ptr) {
    PoolAllocator<sizeof(TVEBSet)>::deallocate(ptr, sizeof(TVEBSet));
  }

  TVEBSet() = default;
  explicit TVEBSet(T value) { Data_ = (1ULL << value); }

  [[nodiscard]] FORCE_INLINE bool Empty() const { return Data_ == 0; }

  FORCE_INLINE void Insert(T value) { Data_ |= (1ULL << value); }
  FORCE_INLINE void Erase(T value) { Data_ &= ~(1ULL << value); }
  [[nodiscard]] FORCE_INLINE bool Contains(T value) const {
    return (Data_ >> value) & 1ULL;
  }

  [[nodiscard]] FORCE_INLINE T Min() const {
    return Data_ ? static_cast<T>(CountTrailingZeros(Data_)) : 0;
  }

  [[nodiscard]] FORCE_INLINE T Max() const {
    return Data_ ? static_cast<T>(63 - CountLeadingZeros(Data_)) : 0;
  }

  [[nodiscard]] FORCE_INLINE std::optional<T> Next(T value) const {
    if (value >= 63) return std::nullopt;
    uint64_t mask = Data_ & ~((1ULL << (value + 1)) - 1);
    if (mask == 0) return std::nullopt;
    return static_cast<T>(CountTrailingZeros(mask));
  }

  [[nodiscard]] FORCE_INLINE std::optional<T> Prev(T value) const {
    if (value == 0) return std::nullopt;
    uint64_t mask = Data_ & ((1ULL << value) - 1);
    if (mask == 0) return std::nullopt;
    return static_cast<T>(63 - CountLeadingZeros(mask));
  }

  [[nodiscard]] FORCE_INLINE T NextInner(T value) const {
    auto res = Next(value);
    return res.value_or(value);
  }

  [[nodiscard]] FORCE_INLINE T PrevInner(T value) const {
    auto res = Prev(value);
    return res.value_or(value);
  }

  template<const uint64_t W, typename U, typename E> friend
  class TVEBSet;
};

// ==========================================
// INTERMEDIATE CASE (4 < WIDTH <= 16)
// ==========================================
template<const uint64_t WIDTH, typename T>
class TVEBSet<WIDTH,
              T,
              typename std::enable_if<(WIDTH > 4 && WIDTH <= 16)>::type> {
  static_assert(sizeof(T) * 8 >= WIDTH, "T too small");

  static constexpr uint64_t LOWER_WIDTH = WIDTH / 2;
  static constexpr uint64_t HIGHER_WIDTH = WIDTH - LOWER_WIDTH;

  TVEBSet<LOWER_WIDTH, T>* Subsets_[1ULL << HIGHER_WIDTH];
  TVEBSet<HIGHER_WIDTH, T>* SubsetsManager_ = nullptr;

  T Min_ = 0;
  T Max_ = 0;
  bool IsEmpty_ = true;

 public:
  static void* operator new(size_t size) {
    return PoolAllocator<sizeof(TVEBSet)>::allocate(size);
  }
  static void operator delete(void* ptr) {
    PoolAllocator<sizeof(TVEBSet)>::deallocate(ptr,
                                               sizeof(TVEBSet));
  }

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

  [[nodiscard]] FORCE_INLINE bool Contains(T value) const {
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

  [[nodiscard]] FORCE_INLINE T NextInner(T value) const {
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

  [[nodiscard]] FORCE_INLINE T PrevInner(T value) const {
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


// ==========================================
// ROOT (WIDTH > 16)
// ==========================================
template<const uint64_t WIDTH, typename T, typename Enable>
class TVEBSet {
  static constexpr size_t LOWER_BITS_COUNT = 16;
  static constexpr size_t HIGHER_BITS_COUNT = WIDTH - LOWER_BITS_COUNT;

  static constexpr T LOWER_MASK = (static_cast<T>(1) << LOWER_BITS_COUNT) - 1;

 public:
  explicit TVEBSet() = default;

  ~TVEBSet() {
    for (auto& pair : Subsets_) {
      delete pair.second;
    }
    Subsets_.clear();
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

    T high = value >> LOWER_BITS_COUNT;
    T low = value & LOWER_MASK;

    auto it = Subsets_.find(high);
    if (it == Subsets_.end()) {
      auto* newSubset = new TVEBSet<LOWER_BITS_COUNT>(low);
      Subsets_.emplace(high, newSubset);
    } else {
      it->second->Insert(low);
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

    auto eraseChild = [&](auto childIt, T childValue) {
      childIt->second->Erase(childValue);
      if (childIt->second->Empty()) {
        delete childIt->second;
        Subsets_.erase(childIt);
      }
    };

    if (value == Min_) {
      if (Subsets_.empty()) {
        Min_ = Max_;
        return;
      }

      auto it = Subsets_.begin();
      T high = it->first;
      T low = it->second->Min();

      Min_ = (high << LOWER_BITS_COUNT) | low;

      eraseChild(it, low);
      return;
    }

    if (value == Max_) {
      if (Subsets_.empty()) {
        Max_ = Min_;
        return;
      }

      auto it = Subsets_.rbegin();
      T high = it->first;
      T low = it->second->Max();

      Max_ = (high << LOWER_BITS_COUNT) | low;

      eraseChild(std::prev(it.base()), low);
      return;
    }

    T high = value >> LOWER_BITS_COUNT;
    T low = value & LOWER_MASK;

    auto it = Subsets_.find(high);
    if (it != Subsets_.end()) {
      eraseChild(it, low);
    }
  }

  [[nodiscard]] FORCE_INLINE bool Contains(T value) const {
    if (Empty()) return false;
    if (value == Min_ || value == Max_) return true;

    T high = value >> LOWER_BITS_COUNT;
    T low = value & LOWER_MASK;

    auto it = Subsets_.find(high);
    return it != Subsets_.end() && it->second->Contains(low);
  }

  [[nodiscard]] std::optional<T> Next(T value) const {
    if (Empty() || value >= Max_) return std::nullopt;
    if (value < Min_) return Min_;

    T high = value >> LOWER_BITS_COUNT;
    T low = value & LOWER_MASK;

    if (auto it = Subsets_.find(high); it != Subsets_.end()) {
      auto nextLow = it->second->NextInner(low);
      if (nextLow > low) {
        return (high << LOWER_BITS_COUNT) | nextLow;
      }
    }

    if (auto itNext = Subsets_.upper_bound(high); itNext != Subsets_.end()) {
      return (itNext->first << LOWER_BITS_COUNT) | itNext->second->Min();
    }

    return Max_;
  }

  [[nodiscard]] std::optional<T> Prev(T value) const {
    if (Empty() || value <= Min_) return std::nullopt;
    if (value > Max_) return Max_;

    T high = value >> LOWER_BITS_COUNT;
    T low = value & LOWER_MASK;

    if (auto it = Subsets_.find(high); it != Subsets_.end()) {
      auto prevLow = it->second->PrevInner(low);
      if (prevLow < low) {
        return (high << LOWER_BITS_COUNT) | prevLow;
      }
    }

    if (auto itNext = Subsets_.lower_bound(high); itNext != Subsets_.begin()) {
      auto itPrev = std::prev(itNext);
      return (itPrev->first << LOWER_BITS_COUNT) | itPrev->second->Max();
    }

    return Min_;
  }

 private:
  std::map<T, TVEBSet<LOWER_BITS_COUNT>*> Subsets_;
  T Min_ = 0;
  T Max_ = 0;
  bool IsEmpty_ = true;
};

}  // namespace veb
