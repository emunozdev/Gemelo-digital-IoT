#pragma once
#include <Arduino.h>

template <typename T, size_t N>
class RingBuffer {
 public:
  void push(const T &v) {
    portENTER_CRITICAL(&_mux);
    _buf[_head] = v;
    _head = (_head + 1) % N;

    if (_count == N) { 
      _tail = (_tail + 1) % N; _dropped++; 
    } else { 
      _count++; 
    }
    
    portEXIT_CRITICAL(&_mux);
  }

  bool pop(T &out) {
    bool ok = false;
    portENTER_CRITICAL(&_mux);
    if (_count > 0) {
      out   = _buf[_tail];
      _tail = (_tail + 1) % N;
      _count--;
      ok = true;
    }
    portEXIT_CRITICAL(&_mux);
    return ok;
  }

  size_t size() {
    portENTER_CRITICAL(&_mux);
    size_t s = _count;
    portEXIT_CRITICAL(&_mux);
    return s;
  }

  uint32_t overruns() const { return _dropped; }

 private:
  T        _buf[N];
  size_t   _head = 0, _tail = 0, _count = 0;
  uint32_t _dropped = 0;
  portMUX_TYPE _mux = portMUX_INITIALIZER_UNLOCKED;
};
