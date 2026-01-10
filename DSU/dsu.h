#pragma once


#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>


class DSU {
 private:
  struct Node {
    size_t Size{1};
    mutable size_t ParentIndex{};
  };

 public:
  explicit DSU(const size_t size) : Tree_(size) {
    Clear();
  }

  void AddSet() {
    Tree_.push_back(Node{
        .Size = 1,
        .ParentIndex = Tree_.size(),
    });
    ++SetsCount_;
  }

  void Unite(const size_t x, const size_t y) {
    assert(x < Tree_.size() && y < Tree_.size());
    auto& xNode = Tree_[GetMainElement(x)];
    auto& yNode = Tree_[GetMainElement(y)];
    if (xNode.ParentIndex == yNode.ParentIndex) {
      return;
    }
    assert(SetsCount_ > 1);
    --SetsCount_;
    if (xNode.Size < yNode.Size) {
      yNode.Size += xNode.Size;
      xNode.ParentIndex = yNode.ParentIndex;
    } else {
      xNode.Size += yNode.Size;
      yNode.ParentIndex = xNode.ParentIndex;
    }
  }

  [[nodiscard]] bool AreUnited(const size_t x, const size_t y) const {
    return GetMainElement(x) == GetMainElement(y);
  }

  [[nodiscard]] size_t GetSetSize(const size_t x) const {
    auto parent = GetMainElement(x);
    return Tree_[parent].Size;
  }

  void Clear() {
    for (size_t i = 0; i < Tree_.size(); ++i) {
      Tree_[i].ParentIndex = i;
      Tree_[i].Size = 1;
    }
    SetsCount_ = Tree_.size();
  }

  [[nodiscard]] size_t GetSetsCount() const {
    return SetsCount_;
  }

  // main element not changes until set is being joined to another set
  size_t GetMainElement(const size_t child) const {
    assert(child < Tree_.size());
    if (Tree_[child].ParentIndex != child) {
      const size_t answer = GetMainElement(Tree_[child].ParentIndex);
      Tree_[child].ParentIndex = answer;
    }
    assert(Tree_[child].ParentIndex < Tree_.size());
    return Tree_[child].ParentIndex;
  }

 private:
  std::vector<Node> Tree_;
  size_t SetsCount_{0};
};