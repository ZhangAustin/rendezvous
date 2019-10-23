#ifndef ARRAY_UTILS_H
#define ARRAY_UTILS_H

#include <cstdlib>

template<typename T>
void fillArray(T* array, const T& value, std::size_t size)
{
    for (std::size_t i = 0; i < size; ++i)
    {
        array[i] = value;
    }
}

#endif //!ARRAY_UTILS_H