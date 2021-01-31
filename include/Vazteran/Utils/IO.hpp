//
// Created by Cyprien Plateau--Holleville on 21/01/2021.
//

#ifndef VAZTERAN_IO_HPP
#define VAZTERAN_IO_HPP

#include <fstream>
#include <string>
#include <vector>

namespace vzt
{
    std::vector<char> ReadShaderFile(const std::string& file);
}

#endif //VAZTERAN_IO_HPP
