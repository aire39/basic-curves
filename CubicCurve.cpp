#include "CubicCurve.h"
#include <algorithm>
#include <limits>
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

int32_t CubicCurve::GetClosestAnchorPoint(const int32_t & index)
{
    // check if selected point is an anchor point.

    int32_t anchor_index = index;
    int32_t sel_idx = ((index-1) % 3); // check if point is the left (control point), middle (anchor point), or right (control point)
    bool is_anchor = (sel_idx == 0); // if value is 0 then the index is the anchor point and if not then we need to figure out where it is...

    // if not an anchor point then check to find the nearest anchor point. if the index value is 1 then the
    // anchor point is 1 less than the current index value else the anchor point is 1 more than the current
    // index
    //
    // example: (3 possible indexes)
    // (3) -- (4) -- (5) => 3 vertices by index (4) is the center vertex on the curve
    //      /    \
    //   (...) (...)
    //
    // 1. (4 - 1) % 3 = 0 selected anchor point
    // 2. (5 - 1) % 3 = 1 selected control point to the right of anchor point
    // 3. (3 - 1) % 3 = 2 selected control point to the left of the anchor point

    if (!is_anchor)
    {
        if (sel_idx == 1)
        {
            anchor_index = (index - 1);
        }
        else
        {
            anchor_index = (index + 1);
        }
    }

    return anchor_index;
}

bool CubicCurve::IsAnchorPoint(int32_t index)
{
    int32_t sel_idx = ((index-1) % 3);
    return (sel_idx == 0);
}

void CubicCurve::interpolateWithCubicHint()
{
    constexpr size_t min_points = 4;
    curveList.clear();
    handleList.clear();

    if (curveData && curveData->pointList.size() >= min_points)
    {
        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;

        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / curveData->smoothFactor; // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        size_t n_segments = (curveData->pointList.size() / 3) - 1;
        n_segments = std::clamp(n_segments, size_t(0), std::numeric_limits<size_t>::max());

        for (size_t i=0; i<n_segments; i++)
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

            if (curveData->areHandlesGenerated)
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

        constexpr size_t min_points_for_closed_curve = 6;
        if (curveData->isCloseLoop && (curveData->pointList.size() > min_points_for_closed_curve))
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
    else if ((curveData && curveData->pointList.size() >= (min_points-1)) && curveData->areHandlesGenerated)
    {
        // draw handles if there are at least 3 points
        uint32_t point_a = 1;
        uint32_t point_b = 2;

        handleList.push_back(curveData->pointList[point_a]);
        handleList.push_back(curveData->pointList[point_a-1]);

        handleList.push_back(curveData->pointList[point_a]);
        handleList.push_back(curveData->pointList[point_b]);
    }
    else
    {
        std::cerr << "no curve data available or not enough data points!\n";
    }
}

void CubicCurve::interpolateWithQuadraticHint()
{
    constexpr size_t min_points = 4;
    curveList.clear();
    handleList.clear();

    if (curveData && curveData->pointList.size() >= min_points)
    {
        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;

        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / curveData->smoothFactor; // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        size_t n_segments = (curveData->pointList.size() / 2) - 1;
        n_segments = std::clamp(n_segments, size_t(0), std::numeric_limits<size_t>::max());

        for (size_t i=0; i<n_segments; i++)
        {
            uint32_t point_a = (i * 2) + 0;
            uint32_t point_b = (i * 2) + 1;
            uint32_t point_c = point_b;
            uint32_t point_d = (i * 2) + 2;

            std::array<float, 2> new_point {};

            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);
                new_point = interpolate(curveData->pointList[point_a], curveData->pointList[point_b], curveData->pointList[point_c], curveData->pointList[point_d], t);
                curveList.push_back(new_point);
                t += step_size;
            }

            t = t_constrain_beg;

            if (curveData->areHandlesGenerated)
            {
                /*
                handleList.emplace_back(curveData->pointList[point_a]);
                handleList.emplace_back(curveData->pointList[point_a-1]);

                handleList.emplace_back(curveData->pointList[point_a]);
                handleList.emplace_back(curveData->pointList[point_b]);

                handleList.emplace_back(curveData->pointList[point_d]);
                handleList.emplace_back(curveData->pointList[point_c]);

                handleList.emplace_back(curveData->pointList[point_d]);
                handleList.emplace_back(curveData->pointList[point_d+1]);
                */

                //handleList.emplace_back(curveData->pointList[point_a]);
                //handleList.emplace_back(curveData->pointList[point_a-1]);

                handleList.emplace_back(curveData->pointList[point_a]);
                handleList.emplace_back(curveData->pointList[point_b]);

                handleList.emplace_back(curveData->pointList[point_b]);
                handleList.emplace_back(curveData->pointList[point_b]);

                handleList.emplace_back(curveData->pointList[point_d]);
                handleList.emplace_back(curveData->pointList[point_c]);

                handleList.emplace_back(curveData->pointList[point_d]);
                handleList.emplace_back(curveData->pointList[point_d+1]);
            }

        }

        constexpr size_t min_points_for_closed_curve = 4;
        if (curveData->isCloseLoop && (curveData->pointList.size() > min_points_for_closed_curve))
        {
            std::array<float, 2> new_point {};

            if (!curveData->pointList.empty())
            {
                uint32_t point_a = curveData->pointList.size()-2;
                uint32_t point_b = curveData->pointList.size()-1;
                uint32_t point_c = point_b;
                uint32_t point_d = 0;

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
    else if ((curveData && curveData->pointList.size() >= (min_points-1)) && curveData->areHandlesGenerated)
    {
        // draw handles if there are at least 3 points
        uint32_t point_a = 1;
        uint32_t point_b = 2;

        handleList.push_back(curveData->pointList[point_a]);
        handleList.push_back(curveData->pointList[point_a-1]);

        handleList.push_back(curveData->pointList[point_a]);
        handleList.push_back(curveData->pointList[point_b]);
    }
    else
    {
        std::cerr << "no curve data available or not enough data points!\n";
    }
}
//NEEDS TO BE DONE
void CubicCurve::interpolateWithLinearHint()
{
    constexpr size_t min_points = 2;
    curveList.clear();
    handleList.clear();

    if (curveData && curveData->pointList.size() >= min_points)
    {
        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;

        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / curveData->smoothFactor; // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        size_t n_segments = curveData->pointList.size()- 1;
        n_segments = std::clamp(n_segments, size_t(0), std::numeric_limits<size_t>::max());

        if (!curveUpscaleData)
        {
            curveUpscaleData = std::make_unique<CurveData>(CURVE_TYPE::QUADRATIC);
        }

        if ((curveUpscaleData->pointList.size() / 3) != curveData->pointList.size())
        {
            curveUpscaleData->pointList.clear();
        }

        for (size_t i=0; i<n_segments; i++)
        {
            uint32_t point_a = (i * 1) + 0;
            uint32_t point_c = point_a;

            uint32_t point_b = (i * 1) + 1;
            uint32_t point_d = point_b;

            std::array<float, 2> control_point_b {(curveData->pointList[point_b][0] - curveData->pointList[point_a][0]), (curveData->pointList[point_b][1] - curveData->pointList[point_a][1])};
            float mag = std::sqrt((control_point_b[0]*control_point_b[0]) + (control_point_b[1]*control_point_b[1]));
            control_point_b[0] = curveData->pointList[point_a][0] + ((control_point_b[0] / mag) * 50.0f);
            control_point_b[1] = curveData->pointList[point_a][1] + ((control_point_b[1] / mag) * 50.0f);

            std::array<float, 2> control_point_d {(curveData->pointList[point_b][0] - curveData->pointList[point_a][0]), (curveData->pointList[point_b][1] - curveData->pointList[point_a][1])};
            mag = std::sqrt((control_point_b[0]*control_point_b[0]) + (control_point_b[1]*control_point_b[1]));
            control_point_d[0] = curveData->pointList[point_a][0] + ((control_point_b[0] / mag) * 50.0f);
            control_point_d[1] = curveData->pointList[point_a][1] + ((control_point_b[1] / mag) * 50.0f);

            if (curveUpscaleData->pointList.size() >= (curveData->pointList.size() * 2))
            {
                curveUpscaleData->pointList[i*2 + 0] = curveData->pointList[point_a];
                curveUpscaleData->pointList[i*2 + 1] = control_point_b;

                curveUpscaleData->pointList[i*2 + 2] = curveData->pointList[point_c];
                if (i == (n_segments-1))
                {
                    std::array<float, 2> control_point_c {-(curveData->pointList[point_c][0] - curveData->pointList[0][0]), -(curveData->pointList[point_c][1] - curveData->pointList[0][1])};
                    mag = std::sqrt((control_point_c[0]*control_point_c[0]) + (control_point_c[1]*control_point_c[1]));
                    control_point_c[0] = curveData->pointList[point_c][0] + ((control_point_c[0] / mag) * 50.0f);
                    control_point_c[1] = curveData->pointList[point_c][1] + ((control_point_c[1] / mag) * 50.0f);
                    curveUpscaleData->pointList[i*2 + 3] = control_point_c;
                }
                else
                {
                    std::array<float, 2> control_point_c {(curveData->pointList[point_c+1][0] - curveData->pointList[point_c][0]), (curveData->pointList[point_c+1][1] - curveData->pointList[point_c][1])};
                    mag = std::sqrt((control_point_c[0]*control_point_c[0]) + (control_point_c[1]*control_point_c[1]));
                    control_point_c[0] = curveData->pointList[point_c][0] + ((control_point_c[0] / mag) * 50.0f);
                    control_point_c[1] = curveData->pointList[point_c][1] + ((control_point_c[1] / mag) * 50.0f);
                    curveUpscaleData->pointList[i*2 + 3] = control_point_c;
                }
            }
            else
            {
                if ((i == (n_segments-1)) && (i==0))
                {
                    curveUpscaleData->pointList.push_back(curveData->pointList[point_a]);
                    curveUpscaleData->pointList.push_back(control_point_b);
                    curveUpscaleData->pointList.push_back(curveData->pointList[point_c]);

                    std::array<float, 2> control_point_c {-(curveData->pointList[point_c][0] - curveData->pointList[0][0]), -(curveData->pointList[point_c][1] - curveData->pointList[0][1])};
                    mag = std::sqrt((control_point_c[0]*control_point_c[0]) + (control_point_c[1]*control_point_c[1]));
                    control_point_c[0] = curveData->pointList[point_c][0] + ((control_point_c[0] / mag) * 50.0f);
                    control_point_c[1] = curveData->pointList[point_c][1] + ((control_point_c[1] / mag) * 50.0f);
                    curveUpscaleData->pointList.push_back(control_point_c);
                }
                else if (i == (n_segments-1))
                {
                    curveUpscaleData->pointList.push_back(curveData->pointList[point_c]);

                    std::array<float, 2> control_point_c {-(curveData->pointList[point_c][0] - curveData->pointList[0][0]), -(curveData->pointList[point_c][1] - curveData->pointList[0][1])};
                    mag = std::sqrt((control_point_c[0]*control_point_c[0]) + (control_point_c[1]*control_point_c[1]));
                    control_point_c[0] = curveData->pointList[point_c][0] + ((control_point_c[0] / mag) * 50.0f);
                    control_point_c[1] = curveData->pointList[point_c][1] + ((control_point_c[1] / mag) * 50.0f);
                    curveUpscaleData->pointList.push_back(control_point_c);
                }
                else
                {
                    curveUpscaleData->pointList.push_back(curveData->pointList[point_a]);
                    curveUpscaleData->pointList.push_back(control_point_b);

                    curveUpscaleData->pointList.push_back(curveData->pointList[point_c]);

                    std::array<float, 2> control_point_c {(curveData->pointList[point_c+1][0] - curveData->pointList[point_c][0]), (curveData->pointList[point_c+1][1] - curveData->pointList[point_c][1])};
                    mag = std::sqrt((control_point_c[0]*control_point_c[0]) + (control_point_c[1]*control_point_c[1]));
                    control_point_c[0] = curveData->pointList[point_c][0] + ((control_point_c[0] / mag) * 50.0f);
                    control_point_c[1] = curveData->pointList[point_c][1] + ((control_point_c[1] / mag) * 50.0f);
                    curveUpscaleData->pointList.push_back(control_point_c);
                }

            }

            std::array<float, 2> new_point {};

            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);
                new_point = interpolate(curveData->pointList[point_a], control_point_b, control_point_d, curveData->pointList[point_c], t);
                curveList.push_back(new_point);
                t += step_size;
            }

            t = t_constrain_beg;

            if (curveData->areHandlesGenerated)
            {
                handleList.emplace_back(curveData->pointList[point_a]);
                handleList.emplace_back(control_point_b);

                handleList.emplace_back(curveData->pointList[point_c]);

                if (i == (n_segments-1))
                {
                    std::array<float, 2> control_point_c {-(curveData->pointList[point_c][0] - curveData->pointList[0][0]), -(curveData->pointList[point_c][1] - curveData->pointList[0][1])};
                    mag = std::sqrt((control_point_c[0]*control_point_c[0]) + (control_point_c[1]*control_point_c[1]));
                    control_point_c[0] = curveData->pointList[point_c][0] + ((control_point_c[0] / mag) * 50.0f);
                    control_point_c[1] = curveData->pointList[point_c][1] + ((control_point_c[1] / mag) * 50.0f);
                    handleList.emplace_back(control_point_c);
                }
                else
                {
                    std::array<float, 2> control_point_c {(curveData->pointList[point_c+1][0] - curveData->pointList[point_c][0]), (curveData->pointList[point_c+1][1] - curveData->pointList[point_c][1])};
                    mag = std::sqrt((control_point_c[0]*control_point_c[0]) + (control_point_c[1]*control_point_c[1]));
                    control_point_c[0] = curveData->pointList[point_c][0] + ((control_point_c[0] / mag) * 50.0f);
                    control_point_c[1] = curveData->pointList[point_c][1] + ((control_point_c[1] / mag) * 50.0f);
                    handleList.emplace_back(control_point_c);
                }
            }

        }

        constexpr size_t min_points_for_closed_curve = 4;
        if (curveData->isCloseLoop && (curveData->pointList.size() > min_points_for_closed_curve))
        {
            /*
            std::array<float, 2> new_point {};

            if (!curveData->pointList.empty())
            {
                uint32_t point_a = curveData->pointList.size()-2;
                uint32_t point_b = curveData->pointList.size()-1;
                uint32_t point_c = point_b;

                std::array<float, 2> control_point_b {(curveData->pointList[point_b][0] - curveData->pointList[point_a][0]), (curveData->pointList[point_b][1] - curveData->pointList[point_a][1])};

                while (t <= (t_constrain_end + epsilon))
                {
                    t = std::min(t, t_constrain_end);
                    new_point = interpolate(curveData->pointList[point_a], curveData->pointList[point_b], curveData->pointList[point_c], t);
                    curveList.push_back(new_point);
                    t += step_size;
                }
            }
             */
        }
    }
    else if ((curveData && curveData->pointList.size() >= (min_points)) && curveData->areHandlesGenerated)
    {
        // TODO: generate handles data
        // draw handles if there are at least 3 points
/*
        uint32_t point_a = 0;
        uint32_t point_b = 1;

        handleList.push_back(curveData->pointList[point_a]);
        handleList.push_back(curveData->pointList[point_b]);
*/
    }
    else
    {
        std::cerr << "no curve data available or not enough data points!\n";
    }
}

void CubicCurve::AddPoint(std::array<float, 2> point)
{
    if (curveData)
    {
        curveData->pointList.push_back(point);
    }
    else
    {
        std::cerr << "no curve data available!\n";
    }
}

void CubicCurve::InsertPoint(std::array<float, 2> point, int32_t index)
{
    if (curveData)
    {
        curveData->pointList.insert(curveData->pointList.begin() + index, point);
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

void CubicCurve::InterpolatePoints()
{
    if (curveData->curveType == CURVE_TYPE::CUBIC)
    {
        interpolateWithCubicHint();
    }
    else if (curveData->curveType == CURVE_TYPE::QUADRATIC)
    {
        interpolateWithQuadraticHint();
    }
    else // CURVE_TYPE::LINEAR
    {
        interpolateWithLinearHint();
    }
}

CubicCurve::CubicCurve(CurveData *curve_data)
{
    curveData = curve_data;
}

void CubicCurve::UpdatePoint(int32_t index, std::array<float, 2> position, CURVE_CONTROL curve_control)
{
    if (curveData && !curveData->pointList.empty())
    {
        // if the point selected to be updated is an anchor point then update the control points to move along
        // with the updated anchor point position

        if (IsAnchorPoint(index))
        {
            std::array<float, 2> diff = {(position[0] - curveData->pointList[index][0]), (position[1] - curveData->pointList[index][1])};
            curveData->pointList[index-1] = {(curveData->pointList[index-1][0] + diff[0]), (curveData->pointList[index-1][1] + diff[1])};
            curveData->pointList[index+1] = {(curveData->pointList[index+1][0] + diff[0]), (curveData->pointList[index+1][1] + diff[1])};

            curveData->pointList[index] = position;
        }
        else
        {
            // if the point selected to be updated is a control point then depending on the type of control update the
            // points to either be moved freely or have the opposite control point align with the control point

            if (curve_control == CURVE_CONTROL::FREE)
            {
                curveData->pointList[index] = position;
            }
            else // CURVE_CONTROL::ALIGNMENT
            {
                int32_t anchor_point = GetClosestAnchorPoint(index);
                std::array<float, 2> diff_0 = {(position[0] - curveData->pointList[anchor_point][0]), (position[1] - curveData->pointList[anchor_point][1])};

                curveData->pointList[index] = position;

                if (index == (anchor_point-1))
                {
                    curveData->pointList[anchor_point+1] = {(curveData->pointList[anchor_point-1][0] - 2.0f * diff_0[0]), (curveData->pointList[anchor_point-1][1] - 2.0f * diff_0[1])};
                }
                else
                {
                    curveData->pointList[anchor_point-1] = {(curveData->pointList[anchor_point+1][0] - 2.0f * diff_0[0]), (curveData->pointList[anchor_point+1][1] - 2.0f * diff_0[1])};
                }
            }
        }

        InterpolatePoints(); // update curve data
    }
    else
    {
        std::cerr << "no curve data available!\n";
    }
}

void CubicCurve::AddAnchor(std::array<float, 2> point, PLACE_ANCHOR place_anchor)
{
    if (curveData->pointList.empty() || (curveData->pointList.size() < 4))
    {
        if (curveData->pointList.empty())
        {
            std::array<float, 2> new_control_point0 = {point[0] - initialControlDistance, point[1]}; // put control point to the left of the anchor point
            AddPoint(new_control_point0); // control point

            AddPoint(point); // point

            std::array<float, 2> new_control_point = {point[0] + initialControlDistance, point[1]}; // put control point to the left of the anchor point
            AddPoint(new_control_point); // control point
        }
        else
        {
            std::array<float, 2> new_control_point0 = {point[0] - initialControlDistance, point[1]}; // put control point to the left of the anchor point
            AddPoint(new_control_point0); // control point

            AddPoint(point); // point

            std::array<float, 2> new_control_point1 = {point[0] + initialControlDistance, point[1]}; // put control point to the left of the anchor point
            AddPoint(new_control_point1); // control point
        }
    }
    else
    {
        if (place_anchor == PLACE_ANCHOR::END) // add new points to end of the curve
        {
            // find the differences of the last anchor points, and it's control points, so it can be added to the new created segment points
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
            AddPoint(new_control_point_1); // control point
        }
        else // add new points to beginning of the curve PLACE_ANCHOR::BEG
        {
            // find the differences of the last anchor points, and it's control points, so it can be added to the new created segment points
            // get last control and anchor points
            const int32_t index = 0;
            int32_t anchor_point_index = GetClosestAnchorPoint(index);

            std::array<float, 2> last_anchor_point = curveData->pointList[anchor_point_index];
            std::array<float, 2> r_control_point = curveData->pointList[anchor_point_index+1];
            std::array<float, 2> l_control_point = curveData->pointList[anchor_point_index-1];

            // get the individual differences from each of the control points to the anchor point
            std::array<float, 2> last_anchor_offset0 = {(l_control_point[0] - last_anchor_point[0]), (l_control_point[1] - last_anchor_point[1])};
            std::array<float, 2> last_anchor_offset1 = {(r_control_point[0] - last_anchor_point[0]), (r_control_point[1] - last_anchor_point[1])};

            // add new segment by adding 3 new points to the list
            // add new control point (right control point)
            std::array<float, 2> new_control_point_1 = {(point[0] + last_anchor_offset1[0]), (point[1] + last_anchor_offset1[1])};
            InsertPoint(new_control_point_1, index); // control point

            // add new anchor point
            InsertPoint(point, index); // anchor point

            // add new control point (left control point)
            std::array<float, 2> new_control_point_0 = {(point[0] + last_anchor_offset0[0]), (point[1] + last_anchor_offset0[1])};
            InsertPoint(new_control_point_0, index); // control point
        }
    }

    InterpolatePoints();
}

void CubicCurve::InsertAnchor(std::array<float, 2> point, int32_t index)
{
    if (curveData && (index > 1))
    {
        // find the differences of the last anchor points, and it's control points, so it can be added to the new created segment points
        // get last control and anchor points
        int32_t anchor_point_index = GetClosestAnchorPoint(index);

        std::array<float, 2> last_anchor_point = curveData->pointList[anchor_point_index];
        std::array<float, 2> r_control_point = curveData->pointList[anchor_point_index+1];
        std::array<float, 2> l_control_point = curveData->pointList[anchor_point_index+2];

        // get the individual differences from each of the control points to the anchor point
        std::array<float, 2> last_anchor_offset0 = {(point[0] - l_control_point[0]) / 2.0f, (point[1] - l_control_point[1]) / 2.0f};
        std::array<float, 2> last_anchor_offset1 = {(point[0] - r_control_point[0]) / 2.0f, (point[1] - r_control_point[1]) / 2.0f};

        // add new segment by adding 3 new points to the list
        // add new control point (right control point)
        std::array<float, 2> new_control_point_1 = {(point[0] + last_anchor_offset1[0]), (point[1] + last_anchor_offset1[1])};
        InsertPoint(new_control_point_1, (index+1)); // control point

        // add new anchor point
        InsertPoint(point, (index+1)); // anchor point

        // add new control point (left control point)
        std::array<float, 2> new_control_point_0 = {(point[0] + last_anchor_offset0[0]), (point[1] + last_anchor_offset0[1])};
        InsertPoint(new_control_point_0, (index+1)); // control point

        InterpolatePoints();
    }
    else
    {
        std::cerr << "no intersection found! Unable to insert a new point!\n";
    }
}

void CubicCurve::RemoveAnchor(int32_t index)
{
    if (curveData && (curveData->pointList.empty() || (curveData->pointList.size() >= 3)))
    {
        index = GetClosestAnchorPoint(index);

        // remove 3 points --> the 2 control points and anchor point
        int32_t remove_index = (index-1);
        for (size_t i=0; i<3; i++)
        {
            DeletePoint(remove_index);
        }

        InterpolatePoints();
    }
    else
    {
        std::cerr << "no curve data available or unable to delete because selected point is not an anchor point!\n";
    }
}

void CubicCurve::CloseLoop(bool close_loop)
{
    if(curveData)
    {
        curveData->isCloseLoop = close_loop;
        InterpolatePoints();
    }
}

std::pair<std::array<float, 2>, uint32_t> CubicCurve::IntersectionOnCurve(std::array<float, 2> position)
{
    std::array<float,2> position_on_curve = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    uint32_t index_insert_index = -1;

    if (curveData && (curveData->pointList.size() > 5))
    {
        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;

        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / (curveData->smoothFactor * 2.0f); // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        size_t n_segments = (curveData->pointList.size() / 3) - 1;
        n_segments = std::clamp(n_segments, size_t(0), std::numeric_limits<size_t>::max());

        bool found_intersection = false;

        for (size_t i=0; i<n_segments; i++)
        {
            uint32_t point_a = (i * 3) + 1; // anchor point
            uint32_t point_b = (i * 3) + 2; // control point
            uint32_t point_c = (i * 3) + 3; // control point
            uint32_t point_d = (i * 3) + 4; // anchor point

            std::array<float, 2> new_point {};

            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);
                new_point = interpolate(curveData->pointList[point_a], curveData->pointList[point_b], curveData->pointList[point_c], curveData->pointList[point_d], t);

                // check point
                std::array<float, 2> check_distance = {std::abs(new_point[0] - position[0]), std::abs(new_point[1] - position[1])};
                float dst = std::sqrt((check_distance[0]*check_distance[0]) + (check_distance[1]*check_distance[1]));

                constexpr float default_radius = 10.0f;
                if (dst <= default_radius)
                {
                    position_on_curve = new_point;
                    found_intersection = true;
                    index_insert_index = point_b;
                    break;
                }

                t += step_size;
            }

            if (found_intersection)
            {
                break;
            }

            t = t_constrain_beg;
        }
    }

    return {position_on_curve, index_insert_index};
}

std::vector<std::array<float, 2>> CubicCurve::Data()
{
    return curveList;
}

const std::vector<std::array<float, 2>> & CubicCurve::HandleData()
{
    return handleList;
}

const std::vector<std::array<float, 2>> & CubicCurve::GetPointData()
{
    return curveData->pointList;
}

void CubicCurve::ForceInterpolation()
{
    InterpolatePoints();
}

CURVE_TYPE CubicCurve::CurveType()
{
    CURVE_TYPE curve_type = CURVE_TYPE::UNKNOWN;

    if (curveData)
    {
        curve_type = curveData->curveType;
    }

    return curve_type;
}

CURVE_TYPE CubicCurve::WorkCurveType()
{
    return CURVE_TYPE::CUBIC;
}

std::unique_ptr<CurveData> CubicCurve::NewCurveData()
{
    auto curve_data = std::make_unique<CurveData>(CURVE_TYPE::CUBIC);
    curve_data->smoothFactor = 50.0f;
    return curve_data;
}
