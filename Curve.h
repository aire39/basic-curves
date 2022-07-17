#pragma once

#include <vector>
#include <array>
#include <cstdint>

enum class CURVE_TYPE {LINEAR, QUADRATIC, CUBIC};

class ICurve
{
    protected:
        virtual void InterpolatePoints() = 0;

    public:
        virtual void AddPoint(std::array<float, 2> point) = 0;
        virtual void UpdatePoint(int32_t index, std::array<float, 2> position) = 0;
        virtual void DeletePoint(int32_t index) = 0;
        virtual void AddAnchor(std::array<float, 2> point) = 0;
        virtual void RemoveAnchor(std::array<float, 2> point) = 0;
        virtual std::vector<std::array<float, 2>> Data() = 0;
        virtual void CloseLoop(bool close_loop) = 0;
};

class CurveData
{
    private:
        CURVE_TYPE curveType = CURVE_TYPE::CUBIC;

    public:
        CurveData() = default;
        explicit CurveData(CURVE_TYPE curve_type) : curveType (curve_type) {}
        virtual ~CurveData() = default;

        std::vector<std::array<float, 2>>  pointList;
        std::vector<std::array<float, 2>*> controlPointList;

        uint32_t id = 0;
        bool isCloseLoop = false;
        bool hidePoints = false;
        bool hideControlPoints = false;
        float smoothFactor = 1.0f;
};
