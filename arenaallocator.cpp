#include "common.hpp"
#include <cassert>
#include <memory>
#include <utility>
#include <string>
#include <print>

class ArenaAllocator
{
public:

    ArenaAllocator() = default;
    ~ArenaAllocator() 
    {
        Destroy();
    };

    ArenaAllocator(const ArenaAllocator&) = delete;
    void operator=(const ArenaAllocator&) = delete;
    
    void Allocate(u64 desiredSize) noexcept
    {
        assert(!m_capacity && !m_head && !m_buffer);
        m_capacity = desiredSize;
        m_buffer =  new u8[m_capacity];
        m_head = m_buffer;
    }

    template<typename T, typename... Args>
    [[nodiscard]]
    T* Store(Args&&... args)
    {
        void* p{ m_head };
        std::size_t space{ (std::size_t)(m_buffer + m_capacity - m_head)}; 
        std::size_t space_temp = space;
        
        if (!std::align(alignof(T), sizeof(T), p, space))
            return nullptr;

        m_head = (u8*)p + sizeof(T);

        return new (p) T(std::forward<Args>(args)...);
    }

    void Destroy()
    {
        if (m_buffer)
        {
            delete[] m_buffer;
            m_head = nullptr;
            m_buffer = nullptr;
            m_capacity = 0;
        }
    }

    const void* GetBuffer() const
    {
        return m_buffer;
    }
    
private:

    u8* m_buffer{ nullptr };
    std::size_t m_capacity{ 0 };
    u8* m_head{ nullptr };
};

struct A;

struct Foo
{
    std::string a;
    std::string b;
    A* c;
};

struct A
{
    f64 b;
    f32 a;
};

i32 main()
{
    ArenaAllocator aa;
    aa.Allocate(1 << 10);
    Foo* f1 = aa.Store<Foo>("ho");
    ptrdiff_t bytes1 = ((char *)f1) - ((char *)aa.GetBuffer());
    std::println("Address of f1 = {}", bytes1);
    
    A* a1 = aa.Store<A>(2.0f, 2.0);
    ptrdiff_t bytes2 = ((char *)a1) - ((char *)aa.GetBuffer());
    std::println("Address of a1 = {}", bytes2);
    
    char* c1 = aa.Store<char>('c');
    ptrdiff_t bytes3 = ((char *)c1) - ((char *)aa.GetBuffer());
    std::println("Address of c1 = {}", bytes3);
    
    Foo* d1 = aa.Store<Foo>("a", "b", nullptr);
    ptrdiff_t bytes4 = ((char *)d1) - ((char *)aa.GetBuffer());
    std::println("Address of c1 = {}", bytes4);
    

    return 0;
}
