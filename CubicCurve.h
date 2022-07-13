#pragma once

#include "Curve.h"
#include <array>

class CubicCurve : public Curve
{
    private:
        std::array<float, 2> interpolate(const std::array<float, 2>  & a, const std::array<float, 2> & b, const std::array<float, 2> & c, const std::array<float, 2> & d, const float & parametric_t);

    protected:
        void InterpolatePoint() override;

    public:
        CubicCurve();

        void AddPoint(std::array<float, 2> point) override;
        void AddAnchor(std::array<float, 2> point) override;
        void RemoveAnchor(std::array<float, 2> point) override;
};
