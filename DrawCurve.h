#pragma once

#include "Curve.h"

#include <vector>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

class DrawCurve
{
    private:
        sf::Color hoverColor = sf::Color(30, 156, 239, 255); // sky blue
        sf::Color selectedColor = sf::Color(215, 95, 115, 255); // matt red
        sf::Color intersectColor = sf::Color(59, 165, 92, 255); // matt green
        sf::Color unselectedColor = sf::Color::Black;
        sf::Color outlineColor = sf::Color::White;
        sf::Color lineColor = sf::Color::White;

        std::vector<float> pointRadiusValues;

        float hoverGrowthRate = 0.25f;
        float hoverRadius = 5.0f;
        float initialRadius = 10.0f;
        float outlineThickness = 2.0f;
        int32_t selectedPoint = -1;
        int32_t hoverPoint = -1;

    public:
        DrawCurve() = default;
        ~DrawCurve() = default;

        void HoverAnimation(ICurve* curve, int32_t x, int32_t y);
        void SelectedPoint(int32_t index);

        void DrawIntersectionPoint(ICurve* curve, int32_t x, int32_t y, sf::RenderWindow & window);
        void RenderCurve(ICurve* curve, sf::RenderWindow & window, const sf::PrimitiveType & primitive_type = sf::PrimitiveType::LineStrip);
    void DrawPoints(ICurve* curve, bool draw_handles, sf::RenderWindow & window);
};
