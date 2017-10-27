namespace ustd {
namespace iter {

  template <typename T, typename StepType>
  struct range_by {
    T first;
    T last;
    StepType step;

    range_by(T first, T last, StepType step)
        : first(first), last(last), step(step) {}

    struct range_by_iterator {
      T current;
      StepType step;

      range_by_iterator(T current, StepType step)
          : current(current), step(step) {}

      range_by_iterator operator++(int) {
        auto ret = *this;
        current += step;
        return ret;
      }
      range_by_iterator& operator++() {
        current += step;
        return *this;
      }
      T& operator*() { return current; }
      T const& operator*() const { return current; }
      T* operator->() { return &current; }
      T const* operator->() const { return &current; }

      auto operator==(range_by_iterator const& rhs) const {
        return current >= rhs.current;
      }
      auto operator!=(range_by_iterator const& rhs) const {
        return !(*this == rhs);
      }
    };

    auto begin() const { return range_by_iterator(first, step); }
    auto end() const { return range_by_iterator(last, step); }
  };

  template <typename T>
  struct range_type {
    T first;
    T last;

    range(T first, T last)
        : first(static_cast<T>(first)), last(static_cast<T>(last)) {}

    struct range_iterator {
      T current;

      range_iterator(T current) : current(current) {}

      range_iterator operator++(int) {
        auto ret = *this;
        current++;
        return ret;
      }
      range_iterator& operator++() {
        ++current;
        return *this;
      }
      T& operator*() { return current; }
      T const& operator*() const { return current; }
      T* operator->() { return &current; }
      T const* operator->() { return &current; }

      bool operator==(range_iterator const& rhs) const {
        return current >= rhs.current;
      }
      bool operator!=(range_iterator const& rhs) const {
        return !(*this == rhs);
      }
    };

    auto begin() const { return range_iterator(first); }
    auto end() const { return range_iterator(last); }

    template <typename StepType>
    auto by(StepType step) const {
      return range_by<T, StepType>(first, last, step);
    }
  };

  template <typename T>
  range_type<T> range(T first, T second) {
    return range_type<T>(first, second);
  }

  template <typename T>
  range_type<T> range(T second) {
    return range_type<T>(static_cast<T>(0), second);
  }

} // namespace iter
} // namespace ustd