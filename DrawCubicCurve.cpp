#include "DrawCubicCurve.h"

#include <cmath>
#include <algorithm>

void DrawCubicCurve::HoverAnimation(ICurve* curve, int32_t x, int32_t y)
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

void DrawCubicCurve::DrawIntersectionPoint(ICurve* curve, int32_t x, int32_t y, sf::RenderWindow & window)
{
    if (curve)
    {
        auto intersect_on_curve = curve->IntersectionOnCurve({(float)x, (float)y});
        auto insert_index = static_cast<int32_t>(intersect_on_curve.second);

        if (insert_index > 1)
        {
            int32_t anchor_index = insert_index - ((insert_index-1) % 3);
            int32_t next_anchor_index = anchor_index + 3;

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

void DrawCubicCurve::SelectedPoint(int32_t index)
{
    selectedPoint = index;
}

void DrawCubicCurve::DrawPoints(ICurve* curve, bool draw_handles, sf::RenderWindow & window)
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

            for (size_t i=0; i<n_points; i++)
            {
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

void DrawCubicCurve::DrawCurve(ICurve* curve, sf::RenderWindow & window, const sf::PrimitiveType & primitive_type)
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
