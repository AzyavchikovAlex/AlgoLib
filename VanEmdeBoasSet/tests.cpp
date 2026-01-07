#include "van_emde_boas_set.h"
using namespace veb;

#include <random>
#include <chrono>
#include <algorithm>
#include <set>
#include <unordered_set>

#include <gtest/gtest.h>

std::mt19937_64 gen(time(nullptr));
std::uniform_int_distribution<uint32_t>
    distrib(0, std::numeric_limits<uint32_t>::max());

template<size_t WIDTH = 32>
void TestContainsMethod(size_t size) {
  std::unordered_set<uint64_t> values;
  TVEBSet<WIDTH> veb_set;
  for (size_t i = 0; i < size; ++i) {
    uint64_t value = distrib(gen) % (1ULL << WIDTH);
    values.insert(value);
    veb_set.Insert(value);

    ASSERT_TRUE(veb_set.Contains(value));

    if (distrib(gen) % 2 == 0) {
      values.erase(value);
      veb_set.Erase(value);

      ASSERT_FALSE(veb_set.Contains(value));
    }
  }

  for (auto value : values) {
    ASSERT_TRUE(veb_set.Contains(value));
  }
}

TEST(BasicOperations, Medium) {
  TestContainsMethod(4000);
}

TEST(BasicOperations, Large) {
  TestContainsMethod(200'000);
}

void TestNextFunction(size_t size) {
  std::set<uint32_t> s;
  TVEBSet<32> veb_set;
  for (size_t i = 0; i < size; ++i) {
    uint32_t val = distrib(gen);
    s.insert(val);
    veb_set.Insert(val);
  }
  for (auto val : s) {
    auto ub = s.upper_bound(val);
    if (ub != s.end()) {
      ASSERT_EQ(veb_set.Next(val), *ub);
    } else {
      ASSERT_EQ(veb_set.Next(val), std::nullopt);
    }
  }

  for (size_t i = 0; i < size; ++i) {
    uint32_t val = distrib(gen);

    auto it = s.upper_bound(val);
    if (it == s.end()) {
      ASSERT_EQ(veb_set.Next(val), std::nullopt);
    } else {
      ASSERT_EQ(veb_set.Next(val), *it);
    }
  }
}

TEST(Next, Medium) {
  TestNextFunction(10'000);
}

void TestPrevFunction(size_t size) {
  std::set<uint32_t> s;
  TVEBSet<32> veb_set;

  for (size_t i = 0; i < size; ++i) {
    uint32_t val = distrib(gen);
    s.insert(val);
    veb_set.Insert(val);
  }

  for (auto val : s) {
    auto it = s.lower_bound(val);

    if (it == s.begin()) {
      ASSERT_EQ(veb_set.Prev(val), std::nullopt);
    } else {
      ASSERT_EQ(veb_set.Prev(val), *std::prev(it));
    }
  }

  for (size_t i = 0; i < size; ++i) {
    uint32_t val = distrib(gen);

    auto it = s.lower_bound(val);
    if (it == s.begin()) {
      ASSERT_EQ(veb_set.Prev(val), std::nullopt);
    } else {
      ASSERT_EQ(veb_set.Prev(val), *std::prev(it));
    }
  }
}

TEST(Prev, Medium) {
  TestPrevFunction(10'000);
}

TEST(Complex, Small) {
  TVEBSet<32> veb;

  // 1. Вставка: [10, 20, 30]
  veb.Insert(10);
  veb.Insert(20);
  veb.Insert(30);

  // 2. Проверка наличия (Contains)
  ASSERT_TRUE(veb.Contains(10) == true);
  ASSERT_TRUE(veb.Contains(20) == true);
  ASSERT_TRUE(veb.Contains(30) == true);
  ASSERT_TRUE(veb.Contains(15) == false); // Этого числа нет

  // 3. Проверка Next (Следующий)
  // Следующий после 10 должен быть 20
  auto next10 = veb.Next(10);
  ASSERT_TRUE(next10.has_value() && *next10 == 20);

  // Следующий после 15 (которого нет) должен быть 20
  auto next15 = veb.Next(15);
  ASSERT_TRUE(next15.has_value() && *next15 == 20);

  // Следующего после 30 быть не должно
  ASSERT_TRUE(!veb.Next(30).has_value());

  // 4. Проверка Prev (Предыдущий)
  // Предыдущий перед 30 должен быть 20
  auto prev30 = veb.Prev(30);
  ASSERT_TRUE(prev30.has_value() && *prev30 == 20);

  // Предыдущий перед 25 (которого нет) должен быть 20
  auto prev25 = veb.Prev(25);
  ASSERT_TRUE(prev25.has_value() && *prev25 == 20);

  // Предыдущего перед 10 быть не должно
  ASSERT_TRUE(!veb.Prev(10).has_value());

  // 5. Удаление (Erase)
  // Удаляем середину (20). Остаются [10, 30]
  veb.Erase(20);

  ASSERT_TRUE(veb.Contains(20) == false);
  ASSERT_TRUE(veb.Contains(10) == true);
  ASSERT_TRUE(veb.Contains(30) == true);

  // 6. Проверка связей после удаления
  // Теперь следующий после 10 должен быть сразу 30 (20 исчезло)
  next10 = veb.Next(10);
  ASSERT_TRUE(next10.has_value() && *next10 == 30);

  // Теперь предыдущий перед 30 должен быть сразу 10
  prev30 = veb.Prev(30);
  ASSERT_TRUE(prev30.has_value() && *prev30 == 10);

  // 7. Очистка
  veb.Erase(10);
  veb.Erase(30);
  ASSERT_TRUE(veb.Empty() == true);
  ASSERT_TRUE(!veb.Contains(10));
}

template<size_t W = 32>
void TestDifferentWidths(size_t size) {
  TestContainsMethod<W>(size);

  if constexpr (W > 1) {
    TestDifferentWidths<W - 1>(size);
  }
}

TEST(WIDTH, BasicTests) {
  TestDifferentWidths<32>(100);
}

TEST(WIDTH, 33) {
  TestDifferentWidths<33>(100);
}



