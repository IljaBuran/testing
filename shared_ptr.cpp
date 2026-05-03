#include <iostream>
#include <cstdint>
#include <utility>
#include <cassert>
#include <type_traits>

#include "common.h"

namespace my
{
    template<typename T>
    class shared_ptr
    {
    private:

        T* data;
        u32* cnt = 0;

    public:

        template<typename ...Params>
        requires (!(sizeof...(Params) == 1 && (... || std::is_same_v<std::decay_t<Params>, shared_ptr>)))
        shared_ptr(Params&&... params)
        {
            data = new T(std::forward<Params>(params)...);
            cnt = new u32(1);
        }

        shared_ptr(const shared_ptr& other)
        {
            assert(other.data);

            data = other.data;
            cnt  = other.cnt;

            (*cnt)++;
        }

        ~shared_ptr()
        {
            release();
        }

        void operator=(const shared_ptr& other)
        {
            if (this == &other)
                return;

            release();

            data = other.data;
            cnt  = other.cnt;

            (*cnt)++;

        }

        T& operator*()
        {
            return *data;
        }

        void release()
        {
            if (cnt != nullptr)
            {
                (*cnt)--;

                if (*cnt == 0)
                {
                    delete data;
                    delete cnt;
                }

                data = nullptr;
                cnt  = nullptr;
            }
        }

        T* get()
        {
            return data;
        }

        u32 count() const
        {
            return *cnt;
        }

        T* operator->()
        {
            return data;
        }

    };

    template<typename T, typename ...Params>
    shared_ptr<T> make_shared(Params&&... params)
    {
        return shared_ptr<T>(std::forward<Params>(params)...);
    }
}

struct foo
{
    f32 f{0};
    i32   i{0};
};

i32 main()
{
    my::shared_ptr<foo> spFoo = my::make_shared<foo>(foo{1.0f, 3});

    my::shared_ptr<foo> spFoo2 = spFoo;

    spFoo2->f = 1.5f;

    // std::cout << spFoo->f << ' ' << spFoo->i << '\n';


    my::shared_ptr<my::shared_ptr<foo>> test = my::make_shared<my::shared_ptr<foo>>(spFoo);

    std::cout << (*test)->f++ << '\n';

    return 0;
}