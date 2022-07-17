#pragma once

#include "Curve.h"
#include <array>
#include <memory>

class CubicCurve : public ICurve
{
    private:
        static std::array<float, 2> interpolate(const std::array<float, 2>  & a, const std::array<float, 2> & b, const std::array<float, 2> & c, const std::array<float, 2> & d, const float & parametric_t);

        CurveData * curveData = nullptr;
        std::vector<std::array<float, 2>> curveList;
        std::vector<std::array<float, 2>> handleList;

    protected:
        void InterpolatePoints() override;

    public:
        CubicCurve() = default;
        explicit CubicCurve(CurveData *curve_data);

        void AddPoint(std::array<float, 2> point) override;
        void UpdatePoint(int32_t index, std::array<float, 2> position) override;
        void DeletePoint(int32_t index) override;
        void AddAnchor(std::array<float, 2> point) override;
        void RemoveAnchor(std::array<float, 2> point) override;
        void CloseLoop(bool close_loop) override;

        std::vector<std::array<float, 2>> Data() override;
        std::vector<std::array<float, 2>> HandleData();

        CubicCurve & operator= (const std::unique_ptr<CurveData> & rhs)
        {
            this->curveData = rhs.get();
            return *this;
        }

        CubicCurve & operator= (std::unique_ptr<CurveData>&& rhs)
        {
            this->curveData = rhs.get();
            return *this;
        }

        static std::unique_ptr<CurveData> NewCurveData();
};
