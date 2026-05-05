#include <iostream>
#include <memory>

int main()
{
    std::allocator<int> allocl;
    static_assert(std::is_same_v<int, decltype(allocl)::value_type>);

    int* p1 = allocl.allocate(1);
    // *p1 = 1;

    std::cout << "p1: " << *p1 << '\n';
    
    allocl.deallocate(p1, 1);
    
    std::cout << "p1: " << *p1 << '\n';
    
    return 0;
}