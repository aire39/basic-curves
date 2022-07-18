//
// Created by sean on 7/17/2022.
//

#include "CurveEffect.h"

#include <cstdlib>
#include <random>
#include <chrono>
#include <thread>

namespace curve_effect
{
    std::vector<std::array<float, 2>> Noise(std::vector<std::array<float,2>> & curve_data)
    {
        std::vector<std::array<float, 2>> new_data;

        if (!curve_data.empty())
        {
            new_data = std::vector<std::array<float, 2>> (curve_data.size());

            auto time_point = std::chrono::steady_clock::now();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            auto time_point_epoch = std::chrono::steady_clock::now();

            uint32_t seed = std::chrono::duration_cast<std::chrono::milliseconds>(time_point - time_point_epoch).count();

            std::default_random_engine rand_gen(seed);
            std::normal_distribution<float> n_dist(5.0f, 2.0f);

            for (size_t i=0; i<new_data.size(); i++)
            {
                new_data[i][0] = curve_data[i][0] + static_cast<float>(static_cast<int32_t>(n_dist(rand_gen)) % 10);
                new_data[i][1] = curve_data[i][1] + static_cast<float>(static_cast<int32_t>(n_dist(rand_gen)) % 10);
            }
        }

        return new_data;
    }
}