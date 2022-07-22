#include "LinearCurve.h"
#include <algorithm>
#include <limits>
#include <cmath>

#include <iostream>

std::array<float, 2> LinearCurve::interpolate(const std::array<float, 2>  & a, const std::array<float, 2> & b, const float & parametric_t)
{
    // lerp p0(a), p1(b) which will result in xy
    float x = std::lerp(a[0], b[0], parametric_t);
    float y = std::lerp(a[1], b[1], parametric_t);

    // point result on cubic curve
    return {x, y};
}

int32_t LinearCurve::GetClosestAnchorPoint(const int32_t & index)
{
    int32_t anchor_index = index;
    return anchor_index;
}

bool LinearCurve::IsAnchorPoint(int32_t index)
{
    int32_t sel_idx = ((index-1) % 1);
    return (sel_idx == 0);
}

void LinearCurve::interpolateWithLinearHint()
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

        size_t n_segments = (curveData->pointList.size() - 1);
        n_segments = std::clamp(n_segments, size_t(0), std::numeric_limits<size_t>::max());

        for (size_t i=0; i<n_segments; i++)
        {
            uint32_t point_a = (i * 1) + 0;
            uint32_t point_b = (i * 1) + 1;

            std::array<float, 2> new_point {};

            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);
                new_point = interpolate(curveData->pointList[point_a], curveData->pointList[point_b], t);
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
                */
            }

        }

        constexpr size_t min_points_for_closed_curve = 2;
        if (curveData->isCloseLoop && (curveData->pointList.size() > min_points_for_closed_curve))
        {
            std::array<float, 2> new_point {};

            if (!curveData->pointList.empty())
            {
                uint32_t point_a = curveData->pointList.size()-1;
                uint32_t point_b = 0;

                while (t <= (t_constrain_end + epsilon))
                {
                    t = std::min(t, t_constrain_end);
                    new_point = interpolate(curveData->pointList[point_a], curveData->pointList[point_b], t);
                    curveList.push_back(new_point);
                    t += step_size;
                }
            }
        }
    }
    else if ((curveData && curveData->pointList.size() >= (min_points-1)) && curveData->areHandlesGenerated)
    {
        // draw handles if there are at least 3 points
        /*
        uint32_t point_a = 1;
        uint32_t point_b = 2;

        handleList.push_back(curveData->pointList[point_a]);
        handleList.push_back(curveData->pointList[point_a-1]);

        handleList.push_back(curveData->pointList[point_a]);
        handleList.push_back(curveData->pointList[point_b]);
        */
    }
    else
    {
        std::cerr << "no curve data available or not enough data points!\n";
    }
}

void LinearCurve::interpolateWithQuadraticHint()
{
    std::cerr << "no implementation for curve type " << curveData->curveType << "\n";
}

void LinearCurve::interpolateWithCubicHint()
{
    std::cerr << "no implementation for curve type " << curveData->curveType << "\n";
}

void LinearCurve::AddPoint(std::array<float, 2> point)
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

void LinearCurve::InsertPoint(std::array<float, 2> point, int32_t index)
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

void LinearCurve::DeletePoint(int32_t index)
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

void LinearCurve::InterpolatePoints()
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

LinearCurve::LinearCurve(CurveData *curve_data)
{
    curveData = curve_data;
}

void LinearCurve::UpdatePoint(int32_t index, std::array<float, 2> position, CURVE_CONTROL curve_control)
{
    if (curveData && !curveData->pointList.empty())
    {
        // if the point selected to be updated is an anchor point then update the control points to move along
        // with the updated anchor point position

        if (IsAnchorPoint(index))
        {
            //std::array<float, 2> diff = {(position[0] - curveData->pointList[index][0]), (position[1] - curveData->pointList[index][1])};
            //curveData->pointList[index-1] = {(curveData->pointList[index-1][0] + diff[0]), (curveData->pointList[index-1][1] + diff[1])};
            //curveData->pointList[index+1] = {(curveData->pointList[index+1][0] + diff[0]), (curveData->pointList[index+1][1] + diff[1])};

            curveData->pointList[index] = position;
        }
        else
        {
            /*
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
            */
        }

        InterpolatePoints(); // update curve data
    }
    else
    {
        std::cerr << "no curve data available!\n";
    }
}

void LinearCurve::AddAnchor(std::array<float, 2> point, PLACE_ANCHOR place_anchor)
{
    if (place_anchor == PLACE_ANCHOR::END) // add new points to end of the curve
    {
        // add new anchor point
        AddPoint(point); // new anchor point
    }
    else // add new points to beginning of the curve PLACE_ANCHOR::BEG
    {
        // insert in front of the first point
        const int32_t index = 0;

        // add new anchor point
        InsertPoint(point, index); // anchor point
    }

    InterpolatePoints();
}

void LinearCurve::InsertAnchor(std::array<float, 2> point, int32_t index)
{
    if (curveData && (index > 1)) // NEED UPDATE
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

void LinearCurve::RemoveAnchor(int32_t index)
{
    if (curveData && (!curveData->pointList.empty()))
    {
        DeletePoint(index);
        InterpolatePoints();
    }
    else
    {
        std::cerr << "no curve data available or unable to delete because selected point is not an anchor point!\n";
    }
}

void LinearCurve::CloseLoop(bool close_loop)
{
    if(curveData)
    {
        curveData->isCloseLoop = close_loop;
        InterpolatePoints();
    }
}

std::pair<std::array<float, 2>, uint32_t> LinearCurve::IntersectionOnCurve(std::array<float, 2> position)
{
    std::array<float,2> position_on_curve = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};
    uint32_t index_insert_index = -1;

    if (curveData && (curveData->pointList.size() >= 2))
    {
        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;

        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / (curveData->smoothFactor * 2.0f); // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        size_t n_segments = (curveData->pointList.size() - 1);
        n_segments = std::clamp(n_segments, size_t(0), std::numeric_limits<size_t>::max());

        bool found_intersection = false;

        for (size_t i=0; i<n_segments; i++)
        {
            uint32_t point_a = (i * 1) + 0; // anchor point
            uint32_t point_b = (i * 1) + 1; // next anchor point

            std::array<float, 2> new_point {};

            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);
                new_point = interpolate(curveData->pointList[point_a], curveData->pointList[point_b], t);

                // check point
                std::array<float, 2> check_distance = {std::abs(new_point[0] - position[0]), std::abs(new_point[1] - position[1])};
                float dst = std::sqrt((check_distance[0]*check_distance[0]) + (check_distance[1]*check_distance[1]));

                constexpr float default_radius = 10.0f;
                if (dst <= default_radius)
                {
                    position_on_curve = new_point;
                    index_insert_index = point_b;
                    found_intersection = true;
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

std::vector<std::array<float, 2>> LinearCurve::Data()
{
    return curveList;
}

const std::vector<std::array<float, 2>> & LinearCurve::HandleData()
{
    return handleList;
}

const std::vector<std::array<float, 2>> & LinearCurve::GetPointData()
{
    return curveData->pointList;
}

CURVE_TYPE LinearCurve::CurveType()
{
    CURVE_TYPE curve_type = CURVE_TYPE::CUBIC;

    if (curveData)
    {
        curve_type = curveData->curveType;
    }

    return curve_type;
}

std::unique_ptr<CurveData> LinearCurve::NewCurveData()
{
    auto curve_data = std::make_unique<CurveData>(CURVE_TYPE::LINEAR);
    curve_data->smoothFactor = 5.0f;
    return curve_data;
}
