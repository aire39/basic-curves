//
// Created by sean on 7/13/2022.
//

#include "CubicCurve.h"
#include <limits>
#include <algorithm>
#include <vector>
#include <cmath>

#include <iostream>

std::array<float, 2> CubicCurve::interpolate(const std::array<float, 2>  & a, const std::array<float, 2> & b, const std::array<float, 2> & c, const std::array<float, 2> & d, const float & parametric_t)
{
    // lerp p0(a), p1(b) and p1(b), p2(c) which will result in q0
    float x0 = std::lerp(a[0], b[0], parametric_t);
    float y0 = std::lerp(a[1], b[1], parametric_t);

    float x1 = std::lerp(b[0], c[0], parametric_t);
    float y1 = std::lerp(b[1], c[1], parametric_t);

    float xq0 = std::lerp(x0, x1, parametric_t);
    float yq0 = std::lerp(y0, y1, parametric_t);

    // lerp p1(b), p2(c) and p2(c), p3(d) which will result in q1
    float x2 = std::lerp(b[0], c[0], parametric_t);
    float y2 = std::lerp(b[1], c[1], parametric_t);

    float x3 = std::lerp(c[0], d[0], parametric_t);
    float y3 = std::lerp(c[1], d[1], parametric_t);

    float xq1 = std::lerp(x2, x3, parametric_t);
    float yq1 = std::lerp(y2, y3, parametric_t);

    // lerp the results of pq0(q0) and pq1(q1) which results in pc
    float xpc = std::lerp(xq0, xq1, parametric_t);
    float ypc = std::lerp(yq0, yq1, parametric_t);

    // point result on cubic curve
    return {xpc, ypc};
}

void CubicCurve::InterpolatePoints()
{
    constexpr size_t min_points = 4;
    if (curveData && curveData->pointList.size() >= min_points)
    {
        curveList.clear();
        handleList.clear();

        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;

        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / curveData->smoothFactor; // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        //size_t n_iterations = ((curveData->pointList.size() - 4)  / 3) + 1;
        size_t n_iterations = (curveData->pointList.size() / 3) - 1;
        n_iterations = std::clamp(n_iterations, size_t(0), std::numeric_limits<size_t>::max());

        for (size_t i=0; i<n_iterations; i++)
        {
            uint32_t point_a = (i * 3) + 1;
            uint32_t point_b = (i * 3) + 2;
            uint32_t point_c = (i * 3) + 3;
            uint32_t point_d = (i * 3) + 4;

            std::array<float, 2> new_point {};

            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);
                new_point = interpolate(curveData->pointList[point_a], curveData->pointList[point_b], curveData->pointList[point_c], curveData->pointList[point_d], t);
                curveList.push_back(new_point);
                t += step_size;
            }

            t = t_constrain_beg;

            if (!curveData->hideControlPoints)
            {
                handleList.emplace_back(curveData->pointList[point_a]);
                handleList.emplace_back(curveData->pointList[point_a-1]);

                handleList.emplace_back(curveData->pointList[point_a]);
                handleList.emplace_back(curveData->pointList[point_b]);

                handleList.emplace_back(curveData->pointList[point_d]);
                handleList.emplace_back(curveData->pointList[point_c]);

                handleList.emplace_back(curveData->pointList[point_d]);
                handleList.emplace_back(curveData->pointList[point_d+1]);
            }

        }

        if (curveData->isCloseLoop && (curveData->pointList.size() > 6))
        {
            std::array<float, 2> new_point {};

            if (!curveData->pointList.empty())
            {
                uint32_t point_a = curveData->pointList.size()-2;
                uint32_t point_b = curveData->pointList.size()-1;
                uint32_t point_c = 0;
                uint32_t point_d = 1;

                while (t <= (t_constrain_end + epsilon))
                {
                    t = std::min(t, t_constrain_end);
                    new_point = interpolate(curveData->pointList[point_a], curveData->pointList[point_b], curveData->pointList[point_c], curveData->pointList[point_d], t);
                    curveList.push_back(new_point);
                    t += step_size;
                }
            }
        }
    }
    else
    {
        std::cerr << "no curve data available or not enough data points!\n";
    }
}

CubicCurve::CubicCurve(CurveData *curve_data)
{
    curveData = curve_data;
}

void CubicCurve::AddPoint(std::array<float, 2> point)
{
    if (curveData)
    {
        curveData->pointList.push_back(point);
        InterpolatePoints();
    }
    else
    {
        std::cerr << "no curve data available!\n";
    }
}

void CubicCurve::UpdatePoint(int32_t index, std::array<float, 2> position)
{
    if (curveData)
    {
        // if the point selected to be updated is an anchor point then update the control points to move along
        // with the updated anchor point position

        bool is_anchor = (((index-1) % 3) == 0);
        if (is_anchor)
        {
            std::array<float, 2> diff = {(position[0] - curveData->pointList[index][0]), (position[1] - curveData->pointList[index][1])};
            curveData->pointList[index-1] = {(curveData->pointList[index-1][0] + diff[0]), (curveData->pointList[index-1][1] + diff[1])};
            curveData->pointList[index+1] = {(curveData->pointList[index+1][0] + diff[0]), (curveData->pointList[index+1][1] + diff[1])};
        }

        curveData->pointList[index] = position;

        InterpolatePoints(); // update curve data
    }
    else
    {
        std::cerr << "no curve data available!\n";
    }
}

void CubicCurve::DeletePoint(int32_t index)
{
    if (curveData)
    {
        curveData->pointList.erase(curveData->pointList.begin() + index);
    }
    else
    {
        std::cerr << "no curve data available!\n";
    }
}

void CubicCurve::AddAnchor(std::array<float, 2> point)
{
    constexpr float initial_control_distance = 50.0f;

    if (curveData->pointList.empty() || (curveData->pointList.size() < 4))
    {
        if (curveData->pointList.empty())
        {
            std::array<float, 2> new_control_point0 = {point[0] - initial_control_distance, point[1]}; // put control point to the left of the anchor point
            AddPoint(new_control_point0); // control point

            AddPoint(point); // point

            std::array<float, 2> new_control_point = {point[0] + initial_control_distance, point[1]}; // put control point to the left of the anchor point
            AddPoint(new_control_point); // control point
        }
        else
        {
            std::array<float, 2> new_control_point0 = {point[0] - initial_control_distance, point[1]}; // put control point to the left of the anchor point
            AddPoint(new_control_point0); // control point

            AddPoint(point); // point

            std::array<float, 2> new_control_point1 = {point[0] + initial_control_distance, point[1]}; // put control point to the left of the anchor point
            AddPoint(new_control_point1); // control point
        }
    }
    else
    {
        // find the differences of the last anchor points and it's control points so it can be added to the new created segment points
        // get last control and anchor points
        std::array<float, 2> last_anchor_point = curveData->pointList[curveData->pointList.size()-2];
        std::array<float, 2> r_control_point = curveData->pointList[curveData->pointList.size()-1];
        std::array<float, 2> l_control_point = curveData->pointList[curveData->pointList.size()-3];

        // get the individual differences from each of the control points to the anchor point
        std::array<float, 2> last_anchor_offset0 = {(l_control_point[0] - last_anchor_point[0]), (l_control_point[1] - last_anchor_point[1])};
        std::array<float, 2> last_anchor_offset1 = {(r_control_point[0] - last_anchor_point[0]), (r_control_point[1] - last_anchor_point[1])};

        // add new segment by adding 3 new points to the list
        // add new control point (left control point)
        std::array<float, 2> new_control_point_0 = {(point[0] + last_anchor_offset0[0]), (point[1] + last_anchor_offset0[1])};
        AddPoint(new_control_point_0); // control point

        // add new anchor point
        AddPoint(point); // new anchor point

        // add new control point (right control point)
        std::array<float, 2> new_control_point_1 = {(point[0] + last_anchor_offset1[0]), (point[1] + last_anchor_offset1[1])};
        AddPoint(new_control_point_1); // control point <-- still need to check this
    }

    InterpolatePoints();
}

void CubicCurve::RemoveAnchor(std::array<float, 2> point)
{/*
    for (const auto p : pointList)
    {
        if (p == point)
        {

        }
    }

    //pointList.erase(pointList.begin());
    InterpolatePoint();
    */
}

void CubicCurve::CloseLoop(bool close_loop)
{
    if(curveData)
    {
        curveData->isCloseLoop = close_loop;
        InterpolatePoints();
    }
}

std::vector<std::array<float, 2>> CubicCurve::Data()
{
    return curveList;
}

std::vector<std::array<float, 2>> CubicCurve::HandleData()
{
    return handleList;
}

std::unique_ptr<CurveData> CubicCurve::NewCurveData()
{
    auto curve_data = std::make_unique<CurveData>(CURVE_TYPE::CUBIC);
    curve_data->smoothFactor = 50.0f;
    return curve_data;
}
