//
// Created by sean on 7/13/2022.
//

#include "CubicCurve.h"
#include <limits>
#include <algorithm>
#include <vector>
#include <cmath>

std::array<float, 2> CubicCurve::interpolate(const std::array<float, 2>  & a, const std::array<float, 2> & b, const std::array<float, 2> & c, const std::array<float, 2> & d, const float & parametric_t)
{
    // lerp p0(a), p1(b) and p1(b), p2(c) which will result in pq0(q0)
    float x0 = std::lerp(a[0], b[0], parametric_t);
    float y0 = std::lerp(a[1], b[1], parametric_t);

    float x1 = std::lerp(b[0], c[0], parametric_t);
    float y1 = std::lerp(b[1], c[1], parametric_t);

    float xq0 = std::lerp(x0, x1, parametric_t);
    float yq0 = std::lerp(y0, y1, parametric_t);

    // lerp p1(b), p2(c) and p2(c), p3(d) which will result in pq1(q1)
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
    if (pointList.size() >= min_points)
    {
        curveList.clear();

        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;

        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / smoothFactor; // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        size_t n_iterations = ((pointList.size() - 4)  / 3) + 1;
        n_iterations = std::clamp(n_iterations, size_t(0), std::numeric_limits<size_t>::max());

        for (size_t i=0; i<n_iterations; i++)
        {
            uint32_t point_a = (i * 3) + 0;
            uint32_t point_b = (i * 3) + 1;
            uint32_t point_c = (i * 3) + 2;
            uint32_t point_d = (i * 3) + 3;

            std::array<float, 2> new_point;

            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);
                new_point = interpolate(pointList[point_a], pointList[point_b], pointList[point_c], pointList[point_d], t);
                curveList.push_back(new_point);
                t += step_size;
            }

            t = t_constrain_beg;

            if (!hideControlPoints)
            {
                //draw_linear_curve(pointList[point_a], pointList[point_b], 1.0f, window);
                //draw_linear_curve(pointList[point_c], pointList[point_d], 1.0f, window);
            }

        }
/*
        bool close_the_loop = false;
        if (close_the_loop)
        {
            if (!pointList.empty())
            {
                t = t_constrain_beg;
                while (t <= (t_constrain_end + epsilon))
                {
                    t = std::min(t, t_constrain_end);
                    v = cubic_curve(pointList[pointList.size()-1], ((pointList[pointList.size()-1].position - pointList[0].position) / 2.0f) + sf::Vector2f(-50.0f,50.0f), ((pointList[pointList.size()-1].position - pointList[0].position) / 2.0f) +  - sf::Vector2f(50.0f,-50.0f), pointList[0], t);
                    curveList.push_back(v);
                    t += step_size;
                }
            }
        }
*/
    }
}

CubicCurve::CubicCurve()
{
    smoothFactor = 50.0f;
}

void CubicCurve::AddPoint(std::array<float, 2> point)
{
    this->pointList.push_back(point);
    InterpolatePoints();
}

void CubicCurve::AddAnchor(std::array<float, 2> point)
{
    /*
    AddPoint(point); // control_point
    AddPoint(point); // control_point
    AddPoint(point); // new anchor point
     InterpolatePoints();
    */
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