#include "DrawCurve.h"

#include <cmath>
#include <algorithm>

void DrawCurve::HoverAnimation(ICurve* curve, int32_t x, int32_t y)
{
    if (!pointRadiusValues.empty())
    {
        const std::vector<std::array<float,2>> & points = curve->GetPointData();
        pointRadiusValues.resize(points.size()); // resize in case an anchor has been deleted

        size_t n_points = points.size();

        for (size_t i=0; i<n_points; i++)
        {
            float x_distance = static_cast<float>(x) - points[i][0];
            float y_distance = static_cast<float>(y) - points[i][1];

            float distance_squared = std::sqrt((x_distance*x_distance) + (y_distance*y_distance));

            if (distance_squared <= initialRadius)
            {
                pointRadiusValues[i] = pointRadiusValues[i] + (hoverGrowthRate * ((1.0f + hoverGrowthRate) + (2.0f + hoverGrowthRate)));
                hoverPoint = static_cast<int32_t>(i);
            }
            else
            {
                pointRadiusValues[i] = pointRadiusValues[i] - (hoverGrowthRate * ((1.0f + hoverGrowthRate) + (2.0f + hoverGrowthRate)));
            }

            pointRadiusValues[i] = std::clamp(pointRadiusValues[i], initialRadius, initialRadius + hoverRadius);
        }
    }
}

void DrawCurve::DrawIntersectionPoint(ICurve* curve, int32_t x, int32_t y, sf::RenderWindow & window)
{
    // The curve needs to exist and work/curve types need to match to do an insertion.
    if (curve && (curve->CurveType() == curve->WorkCurveType()))
    {
        auto intersect_on_curve = curve->IntersectionOnCurve({(float)x, (float)y});
        auto insert_index = static_cast<int32_t>(intersect_on_curve.second);

        if (insert_index >= 1)
        {
            int32_t anchor_index;
            int32_t next_anchor_index;

            if (curve->CurveType() == CURVE_TYPE::CUBIC)
            {
                if (insert_index <= 1)
                {
                    return;
                }

                anchor_index = insert_index - ((insert_index - 1) % 3);
                next_anchor_index = anchor_index + 3;
            }
            else if (curve->CurveType() == CURVE_TYPE::LINEAR)
            {
                anchor_index = (insert_index - 1);
                next_anchor_index = anchor_index + 1;
            }
            else // QUADRATIC
            {
                std::cerr << "Not implemented yet!\n";
                return;
            }

            std::array<float,2> anchor_position = curve->GetPointData()[anchor_index];
            std::array<float,2> anchor_next_position = curve->GetPointData()[next_anchor_index];

            float pos_0_x = static_cast<float>(x) - anchor_position[0];
            float pos_0_y = static_cast<float>(y) - anchor_position[1];
            float pos_0_sqr = std::sqrt((pos_0_x*pos_0_x) + (pos_0_y*pos_0_y));

            float pos_1_x = static_cast<float>(x) - anchor_next_position[0];
            float pos_1_y = static_cast<float>(y) - anchor_next_position[1];
            float pos_1_sqr = std::sqrt((pos_1_x*pos_1_x) + (pos_1_y*pos_1_y));

            if (((pos_0_sqr-initialRadius) >= initialRadius) && ((pos_1_sqr-initialRadius) >= initialRadius))
            {
                sf::CircleShape circle_shape;

                circle_shape.setRadius(initialRadius);
                circle_shape.setOutlineThickness(outlineThickness);
                circle_shape.setFillColor(intersectColor);
                circle_shape.setOutlineColor(outlineColor);
                circle_shape.setPosition(sf::Vector2f(intersect_on_curve.first[0]-initialRadius, intersect_on_curve.first[1]-initialRadius));

                window.draw(circle_shape);
            }
        }
    }
}

void DrawCurve::SelectedPoint(int32_t index)
{
    selectedPoint = index;
}

void DrawCurve::DrawPoints(ICurve* curve, bool draw_handles, sf::RenderWindow & window)
{
    if (curve)
    {
        if (draw_handles)
        {
            const std::vector<std::array<float,2>> & handle_data = curve->HandleData();
            const size_t n_handle_points = handle_data.size();

            std::vector<sf::Vertex> handlePointList (n_handle_points);

            for (size_t i=0; i<n_handle_points; i++)
            {
                handlePointList[i] = sf::Vector2f(handle_data[i][0], handle_data[i][1]);
                handlePointList[i].color = lineColor;
            }

            window.draw(handlePointList.data(), handlePointList.size(), sf::PrimitiveType::Lines);

            // generate radius data for each point. Should only truly resize of the number of points has changed
            const std::vector<std::array<float,2>> & point_data = curve->GetPointData();
            const size_t n_points = point_data.size();
            pointRadiusValues.resize(n_points, initialRadius);

            sf::CircleShape circle_shape;
            circle_shape.setOutlineThickness(outlineThickness);

            const std::vector<std::array<float,2>> & points = curve->GetPointData();
            const std::vector<float> & points_radius = pointRadiusValues;

            size_t start = 0;
            size_t increment = 1;
            size_t ignore_count = 0; // ignore every nth point (0 index inclusive)

            if (curve->CurveType() == CURVE_TYPE::CUBIC)
            {
                if (curve->WorkCurveType() == CURVE_TYPE::LINEAR)
                {
                    start = 1;
                    increment = 3;
                }
                else if (curve->WorkCurveType() == CURVE_TYPE::QUADRATIC)
                {
                    ignore_count = 3;
                }
            }
            else if (curve->CurveType() == CURVE_TYPE::QUADRATIC)
            {
                if (curve->WorkCurveType() == CURVE_TYPE::LINEAR)
                {
                    start = 0;
                    increment = 2;
                }
            }
            else // CURVE_TYPE::LINEAR
            {
                // nothing to implement
            }

            for (size_t i=start; i<n_points; i+=increment)
            {
                if ((ignore_count > 0) && ((i % ignore_count) == 0))
                {
                    continue;
                }

                circle_shape.setFillColor(unselectedColor);
                circle_shape.setOutlineColor(outlineColor);

                if (i == hoverPoint)
                {
                    circle_shape.setFillColor(hoverColor);
                }

                if (i == selectedPoint)
                {
                    if (i == hoverPoint)
                    {
                        circle_shape.setOutlineColor(selectedColor);
                        circle_shape.setFillColor(hoverColor);
                    }
                    else
                    {
                        circle_shape.setFillColor(selectedColor);
                    }
                }

                circle_shape.setRadius(points_radius[i]);
                circle_shape.setPosition((points[i][0] - points_radius[i]), (points[i][1] - points_radius[i]));

                window.draw(circle_shape);
            }
        }

        hoverPoint = -1; // clear hover index
    }
}

void DrawCurve::RenderCurve(ICurve* curve, sf::RenderWindow & window, const sf::PrimitiveType & primitive_type)
{
    const std::vector<std::array<float,2>> & curve_data = curve->Data();
    const size_t n_curve_points = curve_data.size();

    std::vector<sf::Vertex> vertexList (n_curve_points);

    for (size_t i=0; i<n_curve_points; i++)
    {
        vertexList[i] = sf::Vector2f(curve_data[i][0], curve_data[i][1]);
        vertexList[i].color = lineColor;
    }

    window.draw(vertexList.data(), vertexList.size(), primitive_type);
}
