#ifndef VZT_DATA_IMAGE_HPP
#define VZT_DATA_IMAGE_HPP

#include <vector>

template <class ValueType>
struct Image
{
    uint32_t width;
    uint32_t height;
    uint8_t  channels;

    std::vector<ValueType> data;
};

#endif // VZT_DATA_IMAGE_HPP
