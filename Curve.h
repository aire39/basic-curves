#pragma once

#include <vector>
#include <array>
#include <cstdint>
#include <iostream>

enum class CURVE_TYPE : uint16_t {LINEAR, QUADRATIC, CUBIC};

inline std::ostream& operator<<(std::ostream& os, const CURVE_TYPE & curve_type)
{
    switch (curve_type)
    {
        case CURVE_TYPE::LINEAR:
            os << "linear";
            break;

        case CURVE_TYPE::QUADRATIC:
            os << "quadratic";
            break;

        case CURVE_TYPE::CUBIC:
            os << "cubic";
            break;
    }

    return os;
}

enum class CURVE_CONTROL : uint16_t {FREE, ALIGNMENT};
enum class PLACE_ANCHOR : uint16_t {BEG, END};

class ICurve
{
    protected:
        virtual void InterpolatePoints() = 0;

    protected:
        virtual void AddPoint(std::array<float, 2> point) = 0;
        virtual void InsertPoint(std::array<float, 2> point, int32_t index) = 0;
        virtual void DeletePoint(int32_t index) = 0;

    public:
        virtual void UpdatePoint(int32_t index, std::array<float, 2> position, CURVE_CONTROL curve_control) = 0;
        virtual void AddAnchor(std::array<float, 2> point, PLACE_ANCHOR place_anchor) = 0;
        virtual void InsertAnchor(std::array<float, 2> point, int32_t index) = 0;
        virtual void RemoveAnchor(int32_t index) = 0;
        virtual std::vector<std::array<float, 2>> Data() = 0;
        virtual void CloseLoop(bool close_loop) = 0;
        virtual std::pair<std::array<float, 2>, uint32_t> IntersectionOnCurve(std::array<float, 2> position) = 0;
        virtual const std::vector<std::array<float, 2>> & GetPointData() = 0;
        virtual const std::vector<std::array<float, 2>> & HandleData() = 0;
};

struct CurveData
{
    std::vector<std::array<float, 2>>  pointList; // anchor points (mis point between 2 segments that is not a control point unless the intended curve is linear)
    std::vector<std::array<float, 2>*> controlPointList;

    uint32_t id = 0;
    bool isCloseLoop = false;
    bool areHandlesGenerated = true;
    float smoothFactor = 1.0f;
    const CURVE_TYPE curveType = CURVE_TYPE::CUBIC;

    CurveData() = default;
    explicit CurveData(CURVE_TYPE curve_type) : curveType (curve_type) {}
    virtual ~CurveData() = default;
};
