#include <gtest/gtest.h>
#include <vector>
#include <numeric>
#include <algorithm>
#include <random>
#include <set>

#include "dsu.h"

class DSUTest : public ::testing::Test {
 protected:
  void SetUp() override {}
};

// 1. Тест инициализации
TEST_F(DSUTest, Initialization) {
  size_t n = 10;
  DSU dsu(n);

  EXPECT_EQ(dsu.GetSetsCount(), n);
  for (size_t i = 0; i < n; ++i) {
    EXPECT_EQ(dsu.GetSetSize(i), 1);
    EXPECT_EQ(dsu.GetMainElement(i), i);
  }
}

// 2. Тест базового объединения
TEST_F(DSUTest, BasicUnite) {
  DSU dsu(5);

  dsu.Unite(0, 1);
  EXPECT_TRUE(dsu.AreUnited(0, 1));
  EXPECT_EQ(dsu.GetSetsCount(), 4);
  EXPECT_EQ(dsu.GetSetSize(0), 2);

  dsu.Unite(2, 3);
  dsu.Unite(0, 2); // Объединяем {0,1} и {2,3}

  EXPECT_TRUE(dsu.AreUnited(1, 3));
  EXPECT_EQ(dsu.GetSetsCount(), 2);
  EXPECT_EQ(dsu.GetSetSize(3), 4);
}

// 3. Тест самообъединения и повторного объединения
TEST_F(DSUTest, RedundantUnite) {
  DSU dsu(3);

  dsu.Unite(0, 0); // Само с собой
  EXPECT_EQ(dsu.GetSetsCount(), 3);

  dsu.Unite(0, 1);
  size_t countBefore = dsu.GetSetsCount();
  dsu.Unite(0, 1); // Повторно
  EXPECT_EQ(dsu.GetSetsCount(), countBefore);
}

// 4. Тест функции AddSet
TEST_F(DSUTest, AddSet) {
  DSU dsu(2);
  dsu.Unite(0, 1);

  dsu.AddSet(); // Должен появиться элемент с индексом 2
  EXPECT_EQ(dsu.GetSetsCount(), 2);
  EXPECT_EQ(dsu.GetSetSize(2), 1);
  EXPECT_FALSE(dsu.AreUnited(0, 2));

  dsu.Unite(1, 2);
  EXPECT_TRUE(dsu.AreUnited(0, 2));
  EXPECT_EQ(dsu.GetSetSize(0), 3);
}

// 5. Тест Clear
TEST_F(DSUTest, Clear) {
  DSU dsu(10);
  dsu.Unite(0, 1);
  dsu.Unite(2, 3);
  dsu.Unite(0, 2);

  dsu.Clear();
  EXPECT_EQ(dsu.GetSetsCount(), 10);
  for (size_t i = 0; i < 10; ++i) {
    EXPECT_EQ(dsu.GetSetSize(i), 1);
    EXPECT_FALSE(dsu.AreUnited(i, (i + 1) % 10));
  }
}

// 6. Рандомизированный стресс-тест
TEST_F(DSUTest, RandomizedStressTest) {
  const size_t INITIAL_SIZE = 50;
  const int OPERATIONS = 2000;
  DSU dsu(INITIAL_SIZE);

  // Наивная структура данных для проверки:
  // Каждому индексу сопоставляем ID его группы
  std::vector<int> ground_truth(INITIAL_SIZE);
  std::iota(ground_truth.begin(), ground_truth.end(), 0);
  int next_id = INITIAL_SIZE;

  std::mt19937 rng(time(nullptr));

  for (int i = 0; i < OPERATIONS; ++i) {
    std::uniform_int_distribution<int> op_dist(0, 3);
    int op = op_dist(rng);

    size_t current_size = ground_truth.size();
    std::uniform_int_distribution<size_t> idx_dist(0, current_size - 1);

    if (op == 0) { // Unite
      size_t u = idx_dist(rng);
      size_t v = idx_dist(rng);
      dsu.Unite(u, v);

      int id_u = ground_truth[u];
      int id_v = ground_truth[v];
      if (id_u != id_v) {
        for (auto& id : ground_truth) {
          if (id == id_u) id = id_v;
        }
      }
    }
    else if (op == 1) { // AreUnited
      size_t u = idx_dist(rng);
      size_t v = idx_dist(rng);
      EXPECT_EQ(dsu.AreUnited(u, v), (ground_truth[u] == ground_truth[v]));
    }
    else if (op == 2) { // GetSetSize
      size_t u = idx_dist(rng);
      int target_id = ground_truth[u];
      size_t expected_size = std::count(ground_truth.begin(), ground_truth.end(), target_id);
      EXPECT_EQ(dsu.GetSetSize(u), expected_size);
    }
    else if (op == 3) { // AddSet
      dsu.AddSet();
      ground_truth.push_back(next_id++);
    }

    // Периодическая проверка общего количества множеств
    if (i % 100 == 0) {
      std::set<int> unique_sets(ground_truth.begin(), ground_truth.end());
      EXPECT_EQ(dsu.GetSetsCount(), unique_sets.size());
    }
  }
}

// 7. Тест на глубокое дерево (проверка сжатия путей)
TEST_F(DSUTest, DeepTreePathCompression) {
  size_t n = 1000;
  DSU dsu(n);

  // Создаем бамбук: 0-1, 1-2, 2-3...
  for (size_t i = 0; i < n - 1; ++i) {
    dsu.Unite(i, i + 1);
  }

  // При первом вызове GetMainElement(0) произойдет сжатие пути
  size_t root = dsu.GetMainElement(0);

  // Теперь все элементы должны указывать на корень напрямую или почти напрямую
  for (size_t i = 0; i < n; ++i) {
    EXPECT_EQ(dsu.GetMainElement(i), root);
  }
}