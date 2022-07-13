#pragma once

#include <vector>
#include <array>
#include <cstdint>

class Curve
{
    protected:
        std::vector<std::array<float, 2>> pointList;
        std::vector<std::array<float, 2>> curveList;
        std::vector<std::array<float, 2>*> controlPointList;

        uint32_t id = 0;
        bool hidePoints = false;
        bool hideControlPoints = false;

        float smoothFactor = 1.0f;

        virtual void InterpolatePoint() = 0;

    public:
        virtual void AddPoint(std::array<float, 2> point) = 0;
        virtual void AddAnchor(std::array<float, 2> point) = 0;
        virtual void RemoveAnchor(std::array<float, 2> point) = 0;

        std::vector<std::array<float,2>> & CurveData()
        {
            return curveList;
        }

        uint32_t NumberOfPoints()
        {
            return pointList.size();
        }
};
