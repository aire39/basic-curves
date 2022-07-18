#pragma once

#include <vector>
#include <array>

namespace curve_effect
{
    std::vector<std::array<float, 2>> Noise(std::vector<std::array<float,2>> & curve_data);
}