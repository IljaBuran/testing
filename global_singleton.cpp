#include "common.hpp"

#include <cstddef>
#include <cassert>
#include <memory>

// this is from Nicolas Fleury's talk at 
template<typename T>
class GlobalStorage
{
public:
    
    T& Construct()
    {
        assert(!m_alive);
        m_alive = true;
        return *std::construct_at(reinterpret_cast<T*>(m_buffer));
    }

    void Destroy()
    {
        assert(m_alive);
        std::destroy_at(reinterpret_cast<T*>(m_buffer));
        m_alive = false;
    }

    T& GetInstance()
    {
        assert(m_alive);
        return *std::launder(reinterpret_cast<T*>(m_buffer));
    }
    
private:
    
    alignas(T) std::byte m_buffer[sizeof(T)];
    bool m_alive = false;
};

class Renderer
{
    
}

int main()
{
    
}
