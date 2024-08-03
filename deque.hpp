#pragma once

#include <iostream>
#include <iterator>
#include <vector>

template <typename T, typename Allocator = std::allocator<T>>
class Deque {
 public:
  Deque() = default;

  Deque(const Allocator& alloc);

  Deque(const Deque& other);

  Deque(size_t count, const Allocator& alloc = Allocator());

  Deque(size_t count, const T& value, const Allocator& alloc = Allocator());

  Deque(Deque&& other) noexcept;

  Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator());

  ~Deque();

  Deque& operator=(const Deque& other);

  Deque& operator=(Deque&& other) noexcept;

  [[nodiscard]] size_t size() const;

  [[nodiscard]] bool empty() const;

  T& operator[](int ind);

  const T& operator[](int ind) const;

  T& at(size_t ind);

  const T& at(size_t ind) const;

  template <typename... Args>
  void emplace_back(Args&&... args);

  template <typename... Args>
  void emplace_front(Args&&... args);

  void push_back(const T& value);

  void push_back(T&& value);

  void pop_back();

  void push_front(const T& value);

  void push_front(T&& value);

  void pop_front();

  [[nodiscard]] bool is_index_inside(size_t bucket_num, size_t elem_num) const;

  template <bool IsConst = false>
  class BaseIterator;

  using iterator = BaseIterator<false>;
  using const_iterator = BaseIterator<true>;
  using reverse_iterator = std::reverse_iterator<BaseIterator<false>>;
  using const_reverse_iterator = std::reverse_iterator<BaseIterator<true>>;

  iterator begin() {
    return iterator(data_, static_cast<int>(first_bucket_),
                    static_cast<int>(first_pos_), size_);
  }

  const_iterator begin() const {
    return const_iterator(data_, static_cast<int>(first_bucket_),
                          static_cast<int>(first_pos_), size_);
  }

  iterator end() {
    return iterator(data_, static_cast<int>(last_bucket_),
                    static_cast<int>(last_pos_) + 1, size_);
  }

  const_iterator end() const {
    return const_iterator(data_, static_cast<int>(last_bucket_),
                          static_cast<int>(last_pos_) + 1, size_);
  }

  const_iterator cbegin() const {
    return const_iterator(const_cast<const T**>(data_),
                          static_cast<int>(first_bucket_),
                          static_cast<int>(first_pos_), size_);
  }

  const_iterator cend() const {
    return const_iterator(const_cast<const T**>(data_),
                          static_cast<int>(last_bucket_),
                          static_cast<int>(last_pos_) + 1, size_);
  }

  reverse_iterator rbegin() {
    return reverse_iterator({data_, static_cast<int>(last_bucket_),
                             static_cast<int>(last_pos_) + 1, size_});
  }

  reverse_iterator rend() {
    return reverse_iterator({data_, static_cast<int>(first_bucket_),
                             static_cast<int>(first_pos_) - 1, size_});
  }

  const_reverse_iterator crbegin() const {
    return const_reverse_iterator({data_, static_cast<int>(last_bucket_),
                                   static_cast<int>(last_pos_), size_});
  }

  const_reverse_iterator crend() const {
    return const_reverse_iterator({data_, static_cast<int>(first_bucket_),
                                   static_cast<int>(first_pos_) - 1, size_});
  }

  template <bool IsConst>
  void insert(BaseIterator<IsConst> iter, const T& value);

  template <bool IsConst>
  void emplace(BaseIterator<IsConst> iter, T&& value);

  template <bool IsConst>
  void erase(BaseIterator<IsConst> iter);

  [[nodiscard]] Allocator get_allocator() const { return alloc_; }

  static const size_t kBucketSize = 5;

 private:
  using alloc = Allocator;
  using bucket_alloc =
      std::allocator_traits<Allocator>::template rebind_alloc<T*>;
  using alloc_traits = std::allocator_traits<Allocator>;
  using bucket_alloc_traits = std::allocator_traits<bucket_alloc>;

  T** reserve(size_t new_cap, alloc& cur_alloc, bucket_alloc& cur_bucket_alloc);

  void my_swap(size_t& lhs, size_t& rhs) {
    std::swap(lhs, rhs);
    rhs = 0;
  }

  void clear();

  alloc alloc_;
  bucket_alloc bucket_alloc_;

  T** data_ = nullptr;
  size_t size_ = 0;
  size_t bucket_cnt_ = 0;
  size_t first_bucket_ = 0;
  size_t last_bucket_ = 0;
  size_t first_pos_ = 0;
  size_t last_pos_ = 0;
};

template <typename T, typename Allocator>
void Deque<T, Allocator>::clear() {
  for (size_t i = 0; i < bucket_cnt_; ++i) {
    for (size_t j = 0; j < kBucketSize; ++j) {
      if (is_index_inside(i, j)) {
        alloc_traits::destroy(alloc_, data_[i] + j);
      }
    }
    alloc_traits::deallocate(alloc_, data_[i], kBucketSize);
  }
  bucket_alloc_traits::deallocate(bucket_alloc_, data_, bucket_cnt_);
}

template <typename T, typename Allocator>
bool Deque<T, Allocator>::is_index_inside(size_t bucket_num,
                                          size_t elem_num) const {
  if (first_bucket_ == last_bucket_) {
    return bucket_num == first_bucket_ && elem_num >= first_pos_ &&
           elem_num <= last_pos_;
  }
  if (bucket_num == first_bucket_) {
    return elem_num >= first_pos_;
  }
  if (bucket_num == last_bucket_) {
    return elem_num <= last_pos_;
  }
  return bucket_num >= first_bucket_ && bucket_num <= last_bucket_;
}

template <typename T, typename Allocator>
template <bool IsConst>
class Deque<T, Allocator>::BaseIterator {
 public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::conditional_t<IsConst, const T, T>;
  using pointer = value_type*;
  using reference = value_type&;
  using difference_type = std::ptrdiff_t;
  using buckets_type_ptr = std::conditional_t<IsConst, const T**, T**>;

  BaseIterator(buckets_type_ptr ptr, int bucket_ind, int elem_ind, size_t size);

  BaseIterator(const BaseIterator& other) = default;

  BaseIterator& operator=(const BaseIterator& other) = default;

  reference operator*() const { return ptr_[bucket_ind_][elem_ind_]; }

  pointer operator->() const { return ptr_[bucket_ind_] + elem_ind_; }

  BaseIterator& operator-=(int cnt);

  BaseIterator& operator+=(int cnt);

  BaseIterator operator-(int cnt) const;

  BaseIterator operator+(int cnt) const;

  BaseIterator operator++(int);

  BaseIterator& operator++();

  BaseIterator operator--(int);

  BaseIterator& operator--();

  bool operator==(const BaseIterator& rhs) const {
    return ptr_ == rhs.ptr_ && bucket_ind_ == rhs.bucket_ind_ &&
           elem_ind_ == rhs.elem_ind_;
  }

  bool operator!=(const BaseIterator& rhs) const { return !(*this == rhs); }

  bool operator<(const BaseIterator& rhs) const {
    return bucket_ind_ < rhs.bucket_ind_ ||
           (bucket_ind_ == rhs.bucket_ind_ && elem_ind_ < rhs.elem_ind_);
  }

  bool operator>(const BaseIterator& rhs) const {
    return !(*this < rhs || *this == rhs);
  }

  bool operator>=(const BaseIterator& rhs) const { return !(*this < rhs); }

  bool operator<=(const BaseIterator& rhs) const { return !(*this > rhs); }

  difference_type operator-(BaseIterator rhs);

 private:
  buckets_type_ptr ptr_ = nullptr;
  int bucket_ind_ = 0;
  int elem_ind_ = 0;
};

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::BaseIterator<IsConst>::BaseIterator(
    typename Deque<T, Allocator>::BaseIterator<IsConst>::buckets_type_ptr ptr,
    int bucket_ind, int elem_ind, size_t size)
    : ptr_(ptr) {
  if (size == 0) {
    bucket_ind_ = elem_ind_ = 0;
    return;
  }
  if (elem_ind >= static_cast<int>(Deque<T, Allocator>::kBucketSize)) {
    ++bucket_ind;
    elem_ind = 0;
  }
  if (elem_ind < 0) {
    --bucket_ind;
    elem_ind = static_cast<int>(Deque<T, Allocator>::kBucketSize) - 1;
  }
  elem_ind_ = elem_ind;
  bucket_ind_ = bucket_ind;
}

template <typename T, typename Allocator>
T** Deque<T, Allocator>::reserve(size_t new_cap, alloc& cur_alloc,
                                 bucket_alloc& cur_bucket_alloc) {
  if (new_cap <= bucket_cnt_) {
    return data_;
  }
  T** new_data = bucket_alloc_traits::allocate(cur_bucket_alloc, new_cap);
  size_t ind = 0;
  try {
    for (; ind < (new_cap - bucket_cnt_) / 2; ++ind) {
      new_data[ind] = alloc_traits::allocate(cur_alloc, kBucketSize);
    }
  } catch (...) {
    for (size_t i = 0; i <= ind; ++i) {
      alloc_traits::deallocate(cur_alloc, new_data[i], kBucketSize);
    }
    bucket_alloc_traits::deallocate(cur_bucket_alloc, new_data, new_cap);
    throw;
  }
  ind = (new_cap - bucket_cnt_) / 2 + bucket_cnt_;
  try {
    for (; ind < new_cap; ++ind) {
      new_data[ind] = alloc_traits::allocate(cur_alloc, kBucketSize);
    }
  } catch (...) {
    for (size_t i = 0; i <= ind; ++i) {
      alloc_traits::deallocate(cur_alloc, new_data[i], kBucketSize);
    }
    bucket_alloc_traits::deallocate(cur_bucket_alloc, new_data, new_cap);
    throw;
  }

  for (size_t i = 0; i < bucket_cnt_; ++i) {
    new_data[(new_cap - bucket_cnt_) / 2 + i] = data_[i];
  }
  return new_data;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Allocator& alloc) : alloc_(alloc) {}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(const Deque& other)
    : size_(other.size_),
      first_bucket_(other.first_bucket_),
      last_bucket_(other.last_bucket_),
      first_pos_(other.first_pos_),
      last_pos_(other.last_pos_) {
  alloc_ = alloc_traits::select_on_container_copy_construction(other.alloc_);
  bucket_alloc_ = bucket_alloc_traits::select_on_container_copy_construction(
      other.bucket_alloc_);
  data_ = reserve(other.bucket_cnt_, alloc_, bucket_alloc_);
  bucket_cnt_ = other.bucket_cnt_;
  size_t bucket_ind = 0;
  size_t elem_ind = 0;
  try {
    for (size_t i = 0; i < other.bucket_cnt_; ++i) {
      bucket_ind = i;
      for (size_t j = 0; j < kBucketSize; ++j) {
        elem_ind = j;
        alloc_traits::construct(alloc_, data_[i] + j, other.data_[i][j]);
      }
    }
  } catch (...) {
    for (size_t i = 0; i < bucket_cnt_; ++i) {
      for (size_t j = 0; j < kBucketSize; ++j) {
        if (i < bucket_ind || (i == bucket_ind && j < elem_ind)) {
          alloc_traits::destroy(alloc_, data_[i] + j);
        }
      }
      alloc_traits::deallocate(alloc_, data_[i], kBucketSize);
    }
    bucket_alloc_traits::deallocate(bucket_alloc_, data_, bucket_cnt_);
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const Allocator& alloc)
    : alloc_(alloc), size_(count), last_pos_((count - 1) % kBucketSize) {
  if (count == 0) {
    return;
  }
  data_ = reserve((count - 1) / kBucketSize + 1, alloc_, bucket_alloc_);
  bucket_cnt_ = (count - 1) / kBucketSize + 1;
  last_bucket_ = bucket_cnt_ - 1;
  size_t bucket_ind = 0;
  size_t elem_ind = 0;
  try {
    for (size_t i = 0; i < bucket_cnt_; ++i) {
      bucket_ind = i;
      for (size_t j = 0; j < kBucketSize; ++j) {
        if (i * kBucketSize + j + 1 <= count) {
          elem_ind = j;
          alloc_traits::construct(alloc_, data_[i] + j);
        }
      }
    }
  } catch (...) {
    for (size_t i = 0; i < bucket_cnt_; ++i) {
      for (size_t j = 0; j < kBucketSize; ++j) {
        if (i < bucket_ind || (i == bucket_ind && j < elem_ind)) {
          alloc_traits::destroy(alloc_, data_[i] + j);
        }
      }
      alloc_traits::deallocate(alloc_, data_[i], kBucketSize);
    }
    bucket_alloc_traits::deallocate(bucket_alloc_, data_, bucket_cnt_);
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(size_t count, const T& value, const Allocator& alloc)
    : alloc_(alloc), size_(count), last_pos_((count - 1) % kBucketSize) {
  if (count == 0) {
    return;
  }
  data_ = reserve((count - 1) / kBucketSize + 1, alloc_, bucket_alloc_);
  bucket_cnt_ = (count - 1) / kBucketSize + 1;
  last_bucket_ = bucket_cnt_ - 1;
  size_t bucket_ind = 0;
  size_t elem_ind = 0;
  try {
    for (size_t i = 0; i < bucket_cnt_; ++i) {
      bucket_ind = i;
      for (size_t j = 0; j < kBucketSize; ++j) {
        if (i * kBucketSize + j + 1 <= count) {
          elem_ind = j;
          alloc_traits::construct(alloc_, data_[i] + j, value);
        }
      }
    }
  } catch (...) {
    for (size_t i = 0; i < bucket_cnt_; ++i) {
      for (size_t j = 0; j < kBucketSize; ++j) {
        if (i < bucket_ind || (i == bucket_ind && j < elem_ind)) {
          alloc_traits::destroy(alloc_, data_[i] + j);
        }
      }
      alloc_traits::deallocate(alloc_, data_[i], kBucketSize);
    }
    bucket_alloc_traits::deallocate(bucket_alloc_, data_, bucket_cnt_);
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(Deque<T, Allocator>&& other) noexcept
    : data_(other.data_),
      size_(other.size_),
      bucket_cnt_(other.bucket_cnt_),
      first_bucket_(other.first_bucket_),
      last_bucket_(other.last_bucket_),
      first_pos_(other.first_pos_),
      last_pos_(other.last_pos_) {
  other.data_ = nullptr;
  other.size_ = 0;
  other.bucket_cnt_ = 0;
  other.first_bucket_ = 0;
  other.last_bucket_ = 0;
  other.first_pos_ = 0;
  other.last_pos_ = 0;
}

template <typename T, typename Allocator>
Deque<T, Allocator>::Deque(std::initializer_list<T> init,
                           const Allocator& alloc)
    : alloc_(alloc),
      size_(init.size()),
      last_pos_((init.size() - 1) % kBucketSize) {
  if (init.size() == 0) {
    return;
  }
  data_ = reserve((init.size() - 1) / kBucketSize + 1, alloc_, bucket_alloc_);
  bucket_cnt_ = (init.size() - 1) / kBucketSize + 1;
  last_bucket_ = bucket_cnt_ - 1;
  size_t bucket_ind = 0;
  size_t elem_ind = 0;
  auto init_it = init.begin();
  try {
    for (size_t i = 0; i < bucket_cnt_; ++i) {
      bucket_ind = i;
      for (size_t j = 0; j < kBucketSize; ++j) {
        if (i * kBucketSize + j + 1 <= init.size()) {
          elem_ind = j;
          alloc_traits::construct(alloc_, data_[i] + j, std::move(*init_it));
          ++init_it;
        }
      }
    }
  } catch (...) {
    for (size_t i = 0; i < bucket_cnt_; ++i) {
      for (size_t j = 0; j < kBucketSize; ++j) {
        if (i < bucket_ind || (i == bucket_ind && j < elem_ind)) {
          alloc_traits::destroy(alloc_, data_[i] + j);
        }
      }
      alloc_traits::deallocate(alloc_, data_[i], kBucketSize);
    }
    bucket_alloc_traits::deallocate(bucket_alloc_, data_, bucket_cnt_);
    throw;
  }
}

template <typename T, typename Allocator>
Deque<T, Allocator>::~Deque() {
  clear();
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(const Deque& other) {
  if (&other == this) {
    return *this;
  }
  alloc next_alloc = alloc_;
  if constexpr (alloc_traits::propagate_on_container_copy_assignment::value) {
    next_alloc = other.alloc_;
  }
  bucket_alloc next_bucket_alloc = bucket_alloc_;
  if constexpr (alloc_traits::propagate_on_container_copy_assignment::value) {
    next_bucket_alloc = other.bucket_alloc_;
  }
  T** new_data =
      bucket_alloc_traits::allocate(next_bucket_alloc, other.bucket_cnt_);
  for (size_t i = 0; i < other.bucket_cnt_; ++i) {
    new_data[i] = alloc_traits::allocate(next_alloc, kBucketSize);
  }
  size_t bucket_ind = 0;
  size_t elem_ind = 0;
  auto other_it = other.cbegin();
  try {
    for (size_t i = 0; i < other.bucket_cnt_; ++i) {
      bucket_ind = i;
      for (size_t j = 0; j < kBucketSize; ++j) {
        if (other.is_index_inside(i, j)) {
          elem_ind = j;
          alloc_traits::construct(next_alloc, new_data[i] + j, *other_it);
          ++other_it;
        }
      }
    }
  } catch (...) {
    for (size_t i = 0; i < other.bucket_cnt_; ++i) {
      for (size_t j = 0; j < kBucketSize; ++j) {
        if (other.is_index_inside(i, j) &&
            (i < bucket_ind || (i == bucket_ind && j < elem_ind))) {
          alloc_traits::destroy(next_alloc, new_data[i] + j);
        }
      }
      alloc_traits::deallocate(next_alloc, new_data[i], kBucketSize);
    }
    bucket_alloc_traits::deallocate(next_bucket_alloc, new_data,
                                    other.bucket_cnt_);
    throw;
  }
  clear();
  data_ = new_data;
  alloc_ = next_alloc;
  bucket_alloc_ = next_bucket_alloc;
  bucket_cnt_ = other.bucket_cnt_;
  size_ = other.size_;
  first_bucket_ = other.first_bucket_;
  last_bucket_ = other.last_bucket_;
  first_pos_ = other.first_pos_;
  last_pos_ = other.last_pos_;
  return *this;
}

template <typename T, typename Allocator>
Deque<T, Allocator>& Deque<T, Allocator>::operator=(Deque&& other) noexcept {
  if (&other == this) {
    return *this;
  }
  Allocator next_alloc = alloc_;
  if constexpr (alloc_traits::propagate_on_container_move_assignment::value) {
    alloc_ = other.alloc_;
  }
  bucket_alloc next_bucket_alloc = bucket_alloc_;
  if constexpr (bucket_alloc_traits::propagate_on_container_move_assignment::
                    value) {
    bucket_alloc_ = other.bucket_alloc_;
  }
  clear();
  data_ = std::move(other.data_);
  alloc_ = next_alloc;
  bucket_alloc_ = next_bucket_alloc;
  bucket_cnt_ = my_swap(bucket_cnt_, other.bucket_cnt_);
  size_ = my_swap(size_, other.size_);
  first_bucket_ = my_swap(first_bucket_, other.first_bucket_);
  first_pos_ = my_swap(first_pos_, other.first_pos_);
  last_bucket_ = my_swap(last_bucket_, other.last_bucket_);
  last_pos_ = my_swap(last_pos_, other.last_pos_);
}

template <typename T, typename Allocator>
size_t Deque<T, Allocator>::size() const {
  return size_;
}

template <typename T, typename Allocator>
bool Deque<T, Allocator>::empty() const {
  return size_ == 0;
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::operator[](int ind) {
  if (ind <= static_cast<int>(kBucketSize - first_pos_ - 1)) {
    return data_[first_bucket_][first_pos_ + ind];
  }
  int bucket_ind = (ind - (static_cast<int>(kBucketSize - first_pos_)) - 1) /
                       static_cast<int>(kBucketSize) +
                   1;
  int elem_ind = (ind - (static_cast<int>(kBucketSize - first_pos_))) %
                 static_cast<int>(kBucketSize);
  return data_[static_cast<int>(first_bucket_) + bucket_ind][elem_ind];
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::operator[](int ind) const {
  if (ind <= static_cast<int>(kBucketSize - first_pos_ - 1)) {
    return data_[first_bucket_][first_pos_ + ind];
  }
  int bucket_ind = (ind - (static_cast<int>(kBucketSize - first_pos_)) - 1) /
                       static_cast<int>(kBucketSize) +
                   1;
  int elem_ind = (ind - (static_cast<int>(kBucketSize - first_pos_))) %
                 static_cast<int>(kBucketSize);
  return data_[static_cast<int>(first_bucket_) + bucket_ind][elem_ind];
}

template <typename T, typename Allocator>
T& Deque<T, Allocator>::at(size_t ind) {
  if (ind >= size_) {
    throw std::out_of_range("Index out of range!");
  }
  return operator[](ind);
}

template <typename T, typename Allocator>
const T& Deque<T, Allocator>::at(size_t ind) const {
  if (ind >= size_) {
    throw std::out_of_range("Index out of range!");
  }
  return operator[](ind);
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_front(Args&&... args) {
  if (first_pos_ > 0) {
    try {
      alloc_traits::construct(alloc_, data_[first_bucket_] + first_pos_ - 1,
                              std::forward<Args>(args)...);
    } catch (...) {
      throw;
    }
    --first_pos_;
  } else if (first_bucket_ > 0) {
    try {
      alloc_traits::construct(alloc_,
                              data_[first_bucket_ - 1] + kBucketSize - 1,
                              std::forward<Args>(args)...);
    } catch (...) {
      throw;
    }
    --first_bucket_;
    first_pos_ = kBucketSize - 1;
  } else if (first_bucket_ == 0) {
    if (data_ == nullptr) {
      data_ = reserve(3, alloc_, bucket_alloc_);
      try {
        alloc_traits::construct(alloc_, data_[1] + kBucketSize - 1,
                                std::forward<Args>(args)...);
      } catch (...) {
        for (size_t i = 0; i < 3; ++i) {
          alloc_traits::deallocate(alloc_, data_[i], kBucketSize);
        }
        bucket_alloc_traits::deallocate(bucket_alloc_, data_, 3);
        data_ = nullptr;
        throw;
      }
      first_bucket_ = 1;
      first_pos_ = kBucketSize - 1;
      last_bucket_ = 1;
      last_pos_ = first_pos_;
      size_ = 1;
      bucket_cnt_ = 3;
      return;
    }

    size_t next_capacity = bucket_cnt_ * 2 + 1;
    T** new_data = reserve(next_capacity, alloc_, bucket_alloc_);
    bucket_alloc_traits::deallocate(bucket_alloc_, data_, bucket_cnt_);
    data_ = new_data;
    try {
      alloc_traits::construct(
          alloc_,
          data_[(next_capacity - bucket_cnt_) / 2 - 1] + kBucketSize - 1,
          std::forward<Args>(args)...);
    } catch (...) {
      throw;
    }
    last_bucket_ += (next_capacity - bucket_cnt_) / 2;
    first_bucket_ = (next_capacity - bucket_cnt_) / 2 - 1;
    first_pos_ = kBucketSize - 1;
    bucket_cnt_ = next_capacity;
  }
  ++size_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(const T& value) {
  emplace_front(value);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_front(T&& value) {
  emplace_front(std::move(value));
}

template <typename T, typename Allocator>
template <typename... Args>
void Deque<T, Allocator>::emplace_back(Args&&... args) {
  if (data_ == nullptr) {
    data_ = reserve(3, alloc_, bucket_alloc_);
    try {
      alloc_traits::construct(alloc_, data_[1], std::forward<Args>(args)...);
    } catch (...) {
      for (size_t i = 0; i < 3; ++i) {
        alloc_traits::deallocate(alloc_, data_[i], kBucketSize);
      }
      bucket_alloc_traits::deallocate(bucket_alloc_, data_, 3);
      data_ = nullptr;
      throw;
    }
    first_bucket_ = 1;
    first_pos_ = 0;
    last_bucket_ = 1;
    last_pos_ = 0;
    size_ = 1;
    bucket_cnt_ = 3;
    return;
  }
  if (last_pos_ < kBucketSize - 1) {
    try {
      alloc_traits::construct(alloc_, data_[last_bucket_] + last_pos_ + 1,
                              std::forward<Args>(args)...);
    } catch (...) {
      throw;
    }
    ++last_pos_;
  } else if (last_bucket_ < bucket_cnt_ - 1) {
    try {
      alloc_traits::construct(alloc_, data_[last_bucket_ + 1],
                              std::forward<Args>(args)...);
    } catch (...) {
      throw;
    }
    ++last_bucket_;
    last_pos_ = 0;
  } else if (last_bucket_ == bucket_cnt_ - 1) {
    size_t next_capacity = bucket_cnt_ * 2 + 1;
    T** new_data = reserve(next_capacity, alloc_, bucket_alloc_);
    bucket_alloc_traits::deallocate(bucket_alloc_, data_, bucket_cnt_);
    data_ = new_data;
    try {
      alloc_traits::construct(
          alloc_, data_[last_bucket_ + (next_capacity - bucket_cnt_) / 2 + 1],
          std::forward<Args>(args)...);
    } catch (...) {
      throw;
    }
    first_bucket_ += (next_capacity - bucket_cnt_) / 2;
    last_bucket_ += (next_capacity - bucket_cnt_) / 2 + 1;
    last_pos_ = 0;
    bucket_cnt_ = next_capacity;
  }
  ++size_;
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(const T& value) {
  emplace_back(value);
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::push_back(T&& value) {
  emplace_back(std::move(value));
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_back() {
  --size_;
  alloc_traits::destroy(alloc_, data_[last_bucket_] + last_pos_);
  if (last_pos_ == 0) {
    last_pos_ = kBucketSize - 1;
    --last_bucket_;
  } else {
    --last_pos_;
  }
}

template <typename T, typename Allocator>
void Deque<T, Allocator>::pop_front() {
  --size_;
  alloc_traits::destroy(alloc_, data_[first_bucket_] + first_pos_);
  if (first_pos_ == kBucketSize - 1) {
    first_pos_ = 0;
    ++first_bucket_;
  } else {
    ++first_pos_;
  }
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template BaseIterator<IsConst>
Deque<T, Allocator>::BaseIterator<IsConst>::operator++(int) {
  auto tmp = *this;
  if (elem_ind_ < static_cast<int>(kBucketSize) - 1) {
    ++elem_ind_;
  } else {
    ++bucket_ind_;
    elem_ind_ = 0;
  }
  return tmp;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template BaseIterator<IsConst>&
Deque<T, Allocator>::BaseIterator<IsConst>::operator++() {
  if (elem_ind_ < static_cast<int>(kBucketSize) - 1) {
    ++elem_ind_;
  } else {
    ++bucket_ind_;
    elem_ind_ = 0;
  }
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template BaseIterator<IsConst>
Deque<T, Allocator>::BaseIterator<IsConst>::operator--(int) {
  auto tmp = *this;
  if (elem_ind_ > 0) {
    --elem_ind_;
  } else {
    --bucket_ind_;
    elem_ind_ = static_cast<int>(kBucketSize) - 1;
  }
  return tmp;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template BaseIterator<IsConst>&
Deque<T, Allocator>::BaseIterator<IsConst>::operator--() {
  if (elem_ind_ > 0) {
    --elem_ind_;
  } else {
    --bucket_ind_;
    elem_ind_ = static_cast<int>(kBucketSize) - 1;
  }
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template BaseIterator<IsConst>&
Deque<T, Allocator>::BaseIterator<IsConst>::operator+=(int cnt) {
  if (cnt < 0) {
    return operator-=(abs(cnt));
  }
  if (cnt <= static_cast<int>(kBucketSize) - elem_ind_ - 1) {
    elem_ind_ += cnt;
  } else {
    cnt -= static_cast<int>(kBucketSize) - elem_ind_ - 1;
    --cnt;
    ++bucket_ind_;
    bucket_ind_ += cnt / static_cast<int>(kBucketSize);
    elem_ind_ = cnt % static_cast<int>(kBucketSize);
  }
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template BaseIterator<IsConst>&
Deque<T, Allocator>::BaseIterator<IsConst>::operator-=(int cnt) {
  if (cnt < 0) {
    return operator+=(abs(cnt));
  }
  if (cnt <= elem_ind_) {
    elem_ind_ -= cnt;
  } else {
    cnt -= elem_ind_;
    --cnt;
    --bucket_ind_;
    bucket_ind_ -= cnt / static_cast<int>(kBucketSize);
    elem_ind_ =
        static_cast<int>(kBucketSize) - 1 - cnt % static_cast<int>(kBucketSize);
  }
  return *this;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template BaseIterator<IsConst>
Deque<T, Allocator>::BaseIterator<IsConst>::operator+(int cnt) const {
  auto tmp = *this;
  tmp += cnt;
  return tmp;
}

template <typename T, typename Allocator>
template <bool IsConst>
typename Deque<T, Allocator>::template BaseIterator<IsConst>
Deque<T, Allocator>::BaseIterator<IsConst>::operator-(int cnt) const {
  auto tmp = *this;
  tmp -= cnt;
  return tmp;
}

template <typename T, typename Allocator>
template <bool IsConst>
Deque<T, Allocator>::BaseIterator<IsConst>::difference_type
Deque<T, Allocator>::BaseIterator<IsConst>::operator-(BaseIterator rhs) {
  if (bucket_ind_ == rhs.bucket_ind_) {
    return elem_ind_ - rhs.elem_ind_;
  }
  return elem_ind_ + static_cast<int>(Deque<T>::kBucketSize) - rhs.elem_ind_ +
         (bucket_ind_ - rhs.bucket_ind_ - 1) *
             static_cast<int>(Deque<T>::kBucketSize);
}

template <typename T, typename Allocator>
template <bool IsConst>
void Deque<T, Allocator>::insert(BaseIterator<IsConst> iter, const T& value) {
  if (iter == begin()) {
    push_front(value);
    return;
  }
  if (iter == end()) {
    push_back(value);
    return;
  }
  int ind = iter - begin();
  push_back(value);
  for (int i = static_cast<int>(size_) - 1; i > ind; --i) {
    std::swap(operator[](i), operator[](i - 1));
  }
}

template <typename T, typename Allocator>
template <bool IsConst>
void Deque<T, Allocator>::emplace(BaseIterator<IsConst> iter, T&& value) {
  if (iter == begin()) {
    push_front(value);
    return;
  }
  if (iter == end()) {
    push_back(value);
    return;
  }
  int ind = iter - begin();
  push_back(value);
  for (int i = static_cast<int>(size_) - 1; i > ind; --i) {
    std::swap(operator[](i), operator[](i - 1));
  }
}

template <typename T, typename Allocator>
template <bool IsConst>
void Deque<T, Allocator>::erase(BaseIterator<IsConst> iter) {
  if (iter == begin()) {
    pop_front();
    return;
  }
  if (iter == end() - 1) {
    pop_back();
    return;
  }
  int ind = iter - begin();
  for (int i = ind; i > 0; --i) {
    std::swap(operator[](i), operator[](i - 1));
  }
  pop_front();
}
