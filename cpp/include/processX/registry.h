#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <functional>
#include <optional>
#include <cassert>
#include <utility>

namespace px {

  template<class T>
  struct Handle {
    uint32_t index{UINT32_MAX};
    uint32_t generation{0};
    bool valid() const { return index != UINT32_MAX; }
  };

  template<class T>
  struct Slot {
    T value{};
    uint32_t generation{1};
    bool alive{false};
    uint32_t next_free{UINT32_MAX};
  };

  template<class T>
  class Registry {
  public:
    using H = Handle<T>;

    H add(T obj) {
      if (free_head_ != UINT32_MAX) {
        uint32_t idx = free_head_;
        Slot<T>& s = slots_[idx];
        free_head_ = s.next_free;
        s.value = std::move(obj);
        s.alive = true;
        return { idx, s.generation };          // ← matches current gen
      }
      Slot<T> s;
      s.value = std::move(obj);
      s.generation = 1;
      s.alive = true;
      slots_.push_back(std::move(s));
      return { static_cast<uint32_t>(slots_.size()-1), 1 };
    }

    bool contains(H h) const {
      return h.index < slots_.size() &&
            slots_[h.index].alive &&
            slots_[h.index].generation == h.generation;
    }

    bool erase(H h) {
      if (!contains(h)) return false;
      Slot<T>& s = slots_[h.index];
      s.alive = false;
      s.generation++;                 // ← invalidate stale handles
      s.next_free = free_head_;
      free_head_ = h.index;
      return true;
    }

    T& get(H h) { 
      assert(contains(h)); 
      return slots_[h.index].value; 
    }

    const T& get(H h) const { 
      assert(contains(h)); 
      return slots_[h.index].value; 
    }

    template <class Fn> void for_each(Fn&& fn) {
      for (auto& s : slots_) if (s.alive) fn(s.value);
    }
    
    size_t size_alive() const {
      size_t n=0; for (auto& s: slots_) if (s.alive) ++n; return n;
    }

  private:
    std::vector<Slot<T>> slots_;
    uint32_t free_head_{UINT32_MAX};
  };

} // end px namespace
