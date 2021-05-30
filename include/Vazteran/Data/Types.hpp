
#ifndef VAZTERAN_TYPES_HPP
#define VAZTERAN_TYPES_HPP

#include <cstdint>

#include <glm/glm.hpp>

namespace vzt {
    template<typename Type>
    struct Size2D {
        Type width;
        Type height;
    };

    using Color = glm::vec4;
}


#endif //VAZTERAN_TYPES_HPP
