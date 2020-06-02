#pragma once

#include "../Memory/StackAllocator.h"

template<typename T>
struct VectorContainer
{
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    VectorContainer()
        : m_data(nullptr)
        , m_size(0)
        , m_currentIndex(0)
    {
    }

    VectorContainer(size_t size)
        : m_data(nullptr)
        , m_size(size)
        , m_currentIndex(0)
    {
        m_data = static_cast<pointer>(malloc(m_size * sizeof(T)));
    }

    void insert(value_type data)
    {
        if (m_currentIndex >= m_size - 1) {
            LOG_ERROR("VectorContainer: buffer is full");
            return;
        }

        *(m_data + m_currentIndex++) = data;
    }

    value_type get(size_t index)
    {
        if (index > m_currentIndex) {
            LOG_ERROR("VectorContainer: outside range");
            return T();
        }
        return *(m_data + index);
    }

    void clear()
    {
        m_currentIndex = 0u;
    }

    pointer begin() { return m_data; }
    pointer end() { return m_data + m_size - 1; }

    size_t size() const { return m_currentIndex; }
    size_t capacity() const { return m_size; }
    size_t free_space() const { return capacity() - size(); }


    pointer m_data;
    size_t m_size;

    size_t m_currentIndex;
};