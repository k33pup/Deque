# Deque

### This is `std::deque<T>` implementation using **buckets** of length 5

## Here is the interface that the class corresponds to

- Constructors
  - `Deque()` - default, no allocation
  - `Deque(const Allocator&)` - no allocation, just sets the allocator
  - `Deque(const Deque&)` - copy constructor
  - `Deque(size_t count, const Allocator& alloc = Allocator())` - creates a deque of the size count, `T` default construct
  - `Deque(size_t count, const T& value, const Allocator& alloc = Allocator())` - creates a deque of the size count, `T` uses **value** to construct
  - `Deque(Deque&& other)` - move constructor
  - `Deque(std::initializer_list<T> init, const Allocator& alloc = Allocator())` - constructor from initializer_list
- `Destructor`
- `operator=(const Deque& other)` - using copy
- `operator=(Deque&& other)` - using move
- `size_t size()` - returns current size
- `bool empty()` - returns true if the deque is empty otherwise false
- Element access (accesses must work for a guaranteed `O(1)`)
  - `operator[]` (no valid index check)
  - `at()` - with valid index check. Throws `std::out_of_range`
- Change methods (must work for amortized `O(1)`)
  - `push_back`
  - `emplace_back`
  - `pop_back` (no valid size check)
  - `push_front`
  - `emplace_front`
  - `pop_front` (no valid size check)

## Deque also supports working with iterators

### Internal type `iterator` can

- Increment, decrement
- Addition with an integer
- Subtraction of an integer
- Comparisons <,>,<=,>=,==,!=
- Taking the difference from two iterators
- Dereference (`operator*`). Returns `T&`
- `operator->` (Returns `T*`)
- various uses: `value_type`, `pointer`, `iterator_category`, `reference`
- Internal `const_iterator` type. The difference from the usual one is that it does not allow you to change the element lying under it. Conversion (including implicit conversion) from non-constant to constant is acceptable. But reverse conversion is not allowed.
- Internal type `reverse_iterator` (uses `std::reverse_iterator`)
- Methods for accessing iterators:
- `begin`, `cbegin` - return an iterator (constant iterator) to the first element of the deque
  - `end`, `cend` - returns an iterator (constant iterator) to the "element following the last one"
- `rbegin`, `rend`, `crbegin`, `crend` - reverses iterators to the corresponding elements
- `Insert(iterator, const T&)` method - inserts an element iteratively. All the elements to the right are shifted one to the right. Works for `O(n)`
- `Emplace(iterator, T&&)` method - inserts an rvalue element.
- `Erase(iterator)` method - deletes an element by iterator. All elements to the right are shifted one to the left. Works for `O(n)`
