#pragma once

#include <array>

template<typename T, size_t N>
class FixedVector {
private:
    std::array<T, N> m_data;
    size_t m_size;

public:
    FixedVector<T, N>() : m_size{0}, m_data{} { };

    size_t size() { return m_size; }
    T* begin() const { return m_data.data(); }
    T* end() const { return m_data.data() + size; }

    void push_back(T elem) { assert(m_size < N); m_data[m_size++] = elem; }
    T& operator[](size_t i) { assert(i < m_size); return m_data[i]; }
};