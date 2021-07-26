#pragma once

#include <memory>
#include <stdexcept>
#include <vector>

namespace theapx {

template <class T, size_t kBlockSize>
class stable_block_vector {
 public:
  class iterator {
   public:
    T& operator*() { return owner_->at(pos_); }
    T* operator->() { return &owner_->at(pos_); }

    iterator operator--() {
      --pos_;
      return *this;
    }
    iterator operator--(int) {
      iterator result = *this;
      --pos_;
      return result;
    }
    iterator operator++() {
      ++pos_;
      return *this;
    }
    iterator operator++(int) {
      iterator result = *this;
      ++pos_;
      return result;
    }

    bool operator==(const iterator& other) const {
      return this->owner_ == other.owner_ && this->pos_ == other.pos_;
    }
    bool operator!=(const iterator& other) const { return !operator==(other); }

    iterator(const iterator& other) : owner_(other.owner_), pos_(other.pos_) {}
    iterator(stable_block_vector<T, kBlockSize>* owner, size_t pos)
        : owner_(owner), pos_(pos) {}

   private:
    stable_block_vector<T, kBlockSize>* owner_;
    size_t pos_ = 0;
  };

  stable_block_vector();

  iterator begin() { return iterator(this, 0); }
  iterator end() { return iterator(this, size_); }

  T& at(size_t pos);
  T& operator[](size_t pos) { return at(pos); }

  size_t capacity() const { return capacity_; }
  size_t size() const { return size_; }

  void reserve(size_t s);
  void resize(size_t s);

  void push_back(T& v);
  void push_back(T&& v);

 private:
  std::vector<std::unique_ptr<std::vector<T>>> blocks_;
  size_t capacity_ = 0;
  size_t size_ = 0;
};

// -----------------------------------------------------------------------------

template <class T, size_t kBlockSize>
stable_block_vector<T, kBlockSize>::stable_block_vector() {
  //
}

template <class T, size_t kBlockSize>
T& stable_block_vector<T, kBlockSize>::at(size_t pos) {
  if (pos >= size_ || pos < 0) {
    throw std::out_of_range("pos");
  }

  size_t block = pos / kBlockSize;
  size_t block_pos = pos % kBlockSize;
  return blocks_[block]->at(block_pos);
}

template <class T, size_t kBlockSize>
void stable_block_vector<T, kBlockSize>::reserve(size_t s) {
  size_t blocks_needed = s / kBlockSize + (s % kBlockSize == 0 ? 0 : 1);
  blocks_.reserve(blocks_needed);
  while (blocks_.size() < blocks_needed) {
    std::unique_ptr<std::vector<T>> new_block =
        std::make_unique<std::vector<T>>();
    new_block->reserve(kBlockSize);
    blocks_.push_back(std::move(new_block));
  }
  capacity_ = blocks_.size() * kBlockSize;
}

template <class T, size_t kBlockSize>
void stable_block_vector<T, kBlockSize>::resize(size_t s) {
  if (s == size_) {
    return;
  }

  reserve(s + 1);

  size_t old_full_blocks_count = size_ / kBlockSize;
  size_t old_block_pos = size_ % kBlockSize;
  size_t new_full_blocks_count = s / kBlockSize;
  size_t new_block_pos = s % kBlockSize;

  if (new_full_blocks_count > old_full_blocks_count) {
    for (size_t b = old_full_blocks_count; b < new_full_blocks_count; ++b) {
      blocks_[b]->resize(kBlockSize);
    }
  }

  if (new_full_blocks_count < old_full_blocks_count) {
    for (size_t b = old_full_blocks_count; b > new_full_blocks_count; --b) {
      blocks_[b]->resize(0);
    }
  }

  blocks_[new_full_blocks_count]->resize(new_block_pos);
  size_ = s;
}

template <class T, size_t kBlockSize>
void stable_block_vector<T, kBlockSize>::push_back(T& v) {
  size_t new_size = size() + 1;
  reserve(new_size);
  size_t new_item_block = new_size / kBlockSize;
  if (new_size % kBlockSize == 0) {
    --new_item_block;
  }
  blocks_[new_item_block]->push_back(v);
  size_ = new_size;
}

template <class T, size_t kBlockSize>
void stable_block_vector<T, kBlockSize>::push_back(T&& v) {
  size_t new_size = size() + 1;
  reserve(new_size);
  size_t new_item_block = new_size / kBlockSize;
  if (new_size % kBlockSize == 0) {
    --new_item_block;
  }
  blocks_[new_item_block]->push_back(std::move(v));
  size_ = new_size;
}

}  // namespace theapx
