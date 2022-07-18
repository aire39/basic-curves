#pragma once

#include "Curve.h"
#include <vector>
#include <array>
#include <memory>

class CubicCurve : public ICurve
{
    private:
        static std::array<float, 2> interpolate(const std::array<float, 2>  & a, const std::array<float, 2> & b, const std::array<float, 2> & c, const std::array<float, 2> & d, const float & parametric_t);

        CurveData * curveData = nullptr; // curve data block used by this class to generate the curve data
        std::vector<std::array<float, 2>> curveList; // data that holds the generated curve from CurveData
        std::vector<std::array<float, 2>> handleList; // data that holds the generated curve handles from CurveData

        static constexpr float initialControlDistance = 50.0f; // initial distance from the 1st anchor point created

        static int32_t GetClosestAnchorPoint(int32_t index);
        static bool IsAnchorPoint(int32_t index);

        void interpolateWithCubicHint(); // generate a cubic curve
        void interpolateWithQuadraticHint(); // generate cubic curve from quadratic
        void interpolateWithLinearHint(); // generate cubic cubic curve from linear

    protected:
        void AddPoint(std::array<float, 2> point) override; // // add points
        void InsertPoint(std::array<float, 2> point, int32_t index) override; // insert points
        void DeletePoint(int32_t index) override; // delete points
        void InterpolatePoints() override; // generate curve from CurveData point list

    public:
        CubicCurve() = default;
        explicit CubicCurve(CurveData *curve_data);

        void UpdatePoint(int32_t index, std::array<float, 2> position, CURVE_CONTROL curve_control) override; // update selected point around. If anchor is selected then control points are also updated
        void AddAnchor(std::array<float, 2> point, PLACE_ANCHOR place_anchor) override; // add new point with control points of previous anchor (if exist)
        void InsertAnchor(std::array<float, 2> point, int32_t index) override;
        void RemoveAnchor(int32_t index) override; // remove anchor and control points from curve
        void CloseLoop(bool close_loop) override; // close the curve - create a line from the last point to the first
        std::pair<std::array<float, 2>, uint32_t> IntersectionOnCurve(std::array<float, 2> position) override; // get intersecting position of point on the curve and also the insertion index to insert a new point

        std::vector<std::array<float, 2>> Data() override; // return generated curve data points
        std::vector<std::array<float, 2>> HandleData(); // return generated curve handle data points

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

        static std::unique_ptr<CurveData> NewCurveData(); // create new curve object
};
