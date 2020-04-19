#pragma once

enum class DefaultSize : size_t
{
    OneByte = 1,
    OneKB = 1024,
    OneMB = OneKB * OneKB,
    TenMB = OneMB * 10,
};

template<typename T, size_t s = static_cast<size_t>(DefaultSize::TenMB)>
class StackAllocator
{
public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;

    struct Block
    {
        pointer ptr;
        size_t size;
    };

    StackAllocator(size_t bytes = s)
    {
        LOG("StackAllocator: Pre Allocating contiguous block of %d bytes", bytes);
        m_begin = static_cast<pointer>(malloc(bytes));
        m_current = m_begin;
    }

    ~StackAllocator()
    {
        free(m_begin);
    }

    // static constexpr unsigned alignment;
    // static constexpr goodSize(size_t);
    // 
    // Block allocate(size_t);
    // Block allocateALl();
    // Block alignedAllocate(size_t, unsigned);
    // Block alignedReallocate(size_t, unsigned); // _aligned_malloc
    // bool expand(Block& b, size_t delta);
    // void reallocate(Block& b, size_t size);
    // bool owns(Block& b);
    // void deallocate(Block& b);
    // void deallocateAll();

    size_t used_size() const { return sizeof(value_type) * (m_current - m_begin); }
    size_t total_size() const { return (m_begin + s) - m_begin; }
    size_t free_size() const { return total_size() - used_size(); }

    void info() const
    {
        int usage = static_cast<int>(100 * static_cast<float>(used_size()) / total_size());
#if _DEBUG
        LOG("Usage %d %%: [used:%d free:%d total:%d]", usage, used_size(), free_size(), total_size());
#endif
    }

    pointer allocate(size_t size)
    {
        // auto n1 = roundToAligned(size);
        // if (n1 > (d_ + s) - p_) {
        //     return { nullptr, 0 };
        // }
        // Block result = { p_, size };
        // p_ += n1;
        // return result;

        size_t bytesNeeded = size * sizeof(value_type);
        if (bytesNeeded <= 0 || bytesNeeded > free_size())
        {
            LOG("StackAllocator: cannot allocate space.");
            return nullptr;
        }

        pointer loc = m_current;
        // advance current pointer.
        m_current = loc + size;

#if _DEBUG
        info();
#endif

        return loc;
    }

    bool owns(pointer data)
    {
        return data >= m_begin && data < m_begin + s;
    }

    void deallocate(pointer data)
    {
        // rollback only if this is the last pointer allocated.
        // stack allocator. ( can only rollback last allocated memory. not in the middle of the stack )
        if (data == m_current)
        {
            m_current = data;
        }

#if _DEBUG
        info();
#endif
    }

    void deallocateAll()
    {
        m_current = m_begin;
    }

private:
    pointer m_begin;
    pointer m_current;
};
