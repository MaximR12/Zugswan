#pragma once

#include <array>
#include <cassert>

template<typename T, size_t N>
class FixedVector {
private:
    std::array<T, N> m_data;
    size_t m_size;

public:
    FixedVector<T, N>() : m_size{0} { };

    size_t size() { return m_size; }
    T& back() { return m_data[m_size-1]; }
    T* begin() const { return m_data.data(); }
    T* end() const { return m_data.data() + size; }

    void clear() { m_size = 0; }
    void push_back() { assert(m_size < N); m_size++; }
    void push_back(const T& elem) { assert(m_size < N); m_data[m_size++] = elem; }
    void pop_back() { assert(m_size > 0); --m_size; }
    T& operator[](size_t i) { assert(i < m_size); return m_data[i]; }
};