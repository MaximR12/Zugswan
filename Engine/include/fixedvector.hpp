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
    T* begin() { return m_data.data(); }
    T* end() { return &m_data[m_size]; }

    void clear() { m_size = 0; }
    void push_vec(FixedVector<T, N>& vec, size_t pos); 
    void push_back() { assert(m_size < N); m_size++; }
    void push_back(const T& elem) { assert(m_size < N); m_data[m_size++] = elem; }
    void pop_back() { assert(m_size > 0); --m_size; }
    T& operator[](size_t i) { assert(i < m_size); return m_data[i]; }
};

template<typename T, size_t N>
inline void FixedVector<T, N>::push_vec(FixedVector<T, N>& vec, size_t pos) {
    for(size_t i = pos; i < pos + vec.size(); ++i)
        m_data[i] = vec[i-pos];
    m_size = std::max(pos + vec.size(), m_size);
}