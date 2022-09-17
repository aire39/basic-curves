#include <string>
#include <cstdint>
#include <limits>
#include <iostream>

#include <SFML/Window.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <SFML/Window/Mouse.hpp>

#include "Curve.h"
#include "CubicCurve.h"
#include "QuadraticCurve.h"
#include "LinearCurve.h"
#include "DrawCurve.h"
#include "CurveEffect.h"

sf::Vertex linear_curve(sf::Vertex p0, sf::Vertex p1, float t);
sf::Vertex quadratic_curve(sf::Vertex p0, sf::Vertex p1, sf::Vertex p2, float t);
sf::Vertex cubic_curve(sf::Vertex p0, sf::Vertex p1, sf::Vertex p2, sf::Vertex p3, float t);
bool draw_linear_curve(sf::Vertex p0, sf::Vertex p1, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type = sf::PrimitiveType::LineStrip);
bool draw_linear_curve(const std::vector<sf::Vertex>& points, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type, std::vector<sf::Vertex>& result, bool hide_control_point_lines);
bool draw_quadratic_curve(sf::Vertex p0, sf::Vertex p1, sf::Vertex p2, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type = sf::PrimitiveType::LineStrip);
bool draw_quadratic_curve(const std::vector<sf::Vertex>& points, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type, std::vector<sf::Vertex>& result, bool hide_control_point_lines);
bool draw_cubic_curve(sf::Vertex p0, sf::Vertex p1, sf::Vertex p2, sf::Vertex p3, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type = sf::PrimitiveType::LineStrip);
bool draw_cubic_curve(const std::vector<sf::Vertex>& points, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type, std::vector<sf::Vertex>& result, bool hide_control_point_lines);

constexpr int32_t WINDOW_WIDTH = 800;
constexpr int32_t WINDOW_HEIGHT = 800;

#define USE_OLD_CURVES false

int main(int argc, char*argv[])
{
    // initialize and display window using sfml

    sf::ContextSettings context_settings;
    context_settings.depthBits = 24;
    context_settings.antialiasingLevel = 10;
    sf::VideoMode video_mode (WINDOW_WIDTH, WINDOW_HEIGHT);
    std::string window_str_title = "Beszier Curves";

    sf::RenderWindow window (video_mode, window_str_title, sf::Style::Titlebar | sf::Style::Close, context_settings);
    window.setFramerateLimit(60);

    // create text to draw

    bool show_text = true;

    sf::Text txt_control_line_render;
    sf::Text txt_line_mode_message_render;
    sf::Text txt__render;
    sf::Font font;

    if (!font.loadFromFile("ToThePointRegular.ttf"))
    {
        show_text = false;
    }
    else
    {
        txt_control_line_render.setFont(font);
        txt_line_mode_message_render.setFont(font);

        txt_control_line_render.setString("Press B key to render lines/points of curves");

        txt_control_line_render.setFillColor(sf::Color(sf::Color::White));
        txt_line_mode_message_render.setFillColor(sf::Color(sf::Color::Yellow));

        constexpr uint32_t txt_size = 22;
        txt_control_line_render.setCharacterSize(txt_size);
        txt_line_mode_message_render.setCharacterSize(txt_size);

        txt_control_line_render.setStyle(sf::Text::Bold);
        txt_line_mode_message_render.setStyle(sf::Text::Bold);

        txt_control_line_render.setPosition(2, 0);
        //txt_line_mode_message_render.setPosition(800 - txt_line_mode_message_render.getLocalBounds().width, 0);
    }

    // setup window interaction variables

    int32_t mouse_x = 0; // current mouse x location (relative to the window)
    int32_t mouse_y = 0; // current mouse y location (relative to the window)
    int32_t p_mouse_x; // previous mouse x location (relative to the window)
    int32_t p_mouse_y; // previous mouse y location (relative to the window)
    int32_t l_mouse_x = 0; // last position x after initial left mouse click
    int32_t l_mouse_y = 0; // last position y after initial left mouse click
    float mouse_x_dt; // mouse movement x
    float mouse_y_dt; // mouse movement y

    float new_position_x = 0.0f;
    float new_position_y = 0.0f;

    sf::Vector2f last_selected_position;
    bool has_been_selected = false;

    bool control_key_down = false;
    bool shift_key_down = false;
    bool alt_key_down = false;
    bool ignore_click = true; // ignore 1st click to avoid adding velocity when mouse is not moving

    // graphic shapes

    sf::ConvexShape fill_shape;

    float point_radius_size = 10.0f;
    sf::CircleShape circle_draw_shape(point_radius_size);
    circle_draw_shape.setPosition(400 - point_radius_size, 400 - point_radius_size);
    circle_draw_shape.setFillColor(sf::Color::White);

    std::vector<sf::Vertex> line_draw_shape;
    std::vector<float> hover_anim; // animate the anchor/vertex points by enlarging them when hovered over

    sf::PrimitiveType primitive_type = sf::PrimitiveType::LineStrip;

    bool hide_points = false;
    int32_t control_point = -1;
    int32_t curve_samples = 50;

    //CURVE_TYPE curve_type = CURVE_TYPE::QUADRATIC;
    //CURVE_TYPE curve_type = CURVE_TYPE::CUBIC;
    CURVE_TYPE curve_type = CURVE_TYPE::LINEAR;
    bool show_fill = false;
    bool is_close_loop = false;

    // window/drawing loop

    uint32_t n_frames = 0;

    //

    auto curve_data_cubic = CubicCurve::NewCurveData();
    auto curve_data_linear = LinearCurve::NewCurveData();
    auto curve_data_quadratic = QuadraticCurve::NewCurveData();

    CubicCurve cubic_curve (curve_data_linear.get());
    LinearCurve linear_curve (curve_data_linear.get());
    QuadraticCurve quadratic_curve (curve_data_linear.get());
    //LinearCurve linear_curve (curve_data_linear.get());

    DrawCurve draw_cubic_curve;
    DrawCurve d_linear_curve;

    //ICurve * active_curve = &quadratic_curve;
    //ICurve * active_curve = &cubic_curve;
    ICurve * active_curve = &linear_curve;

    while (window.isOpen())
    {
        sf::Event event {};
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            if (event.type == sf::Event::KeyPressed)
            {
                if ((event.key.code == sf::Keyboard::LControl) || (event.key.code == sf::Keyboard::RControl))
                {
                    control_key_down = true;
                }

                if ((event.key.code == sf::Keyboard::LShift) || (event.key.code == sf::Keyboard::RShift))
                {
                    shift_key_down = true;
                }

                if ((event.key.code == sf::Keyboard::LAlt) || (event.key.code == sf::Keyboard::RAlt))
                {
                    alt_key_down = true;
                }
            }

            if (event.type == sf::Event::KeyReleased)
            {
                if ((event.key.code == sf::Keyboard::LControl) || (event.key.code == sf::Keyboard::RControl))
                {
                    control_key_down = false;
                }

                if ((event.key.code == sf::Keyboard::LShift) || (event.key.code == sf::Keyboard::RShift))
                {
                    shift_key_down = false;
                }

                if ((event.key.code == sf::Keyboard::LAlt) || (event.key.code == sf::Keyboard::RAlt))
                {
                    alt_key_down = false;
                }

                if (event.key.code == sf::Keyboard::D)
                {
                    if (!line_draw_shape.empty())
                    {
                        if (control_point >= 0)
                        {
                            //line_draw_shape.erase(line_draw_shape.begin() + control_point);
                            control_point = std::clamp(control_point, 0, static_cast<int32_t>(line_draw_shape.size() - 1));
                        }
                    }

                    if(!active_curve->Data().empty())
                    {
                        active_curve->RemoveAnchor(control_point);
                    }
                }

                if (event.key.code == sf::Keyboard::H)
                {
                    hide_points ^= true;
                }

                if (event.key.code == sf::Keyboard::F)
                {
                    show_fill ^= true;
                }

                if (event.key.code == sf::Keyboard::M)
                {
                    if (curve_type == CURVE_TYPE::LINEAR)
                    {
                        active_curve = &quadratic_curve;
                        curve_type = CURVE_TYPE::QUADRATIC;
                    }
                    else if (curve_type == CURVE_TYPE::QUADRATIC)
                    {
                        active_curve = &cubic_curve;
                        curve_type = CURVE_TYPE::CUBIC;
                    }
                    else // (curve_type == CURVE_TYPE::CUBIC)
                    {
                        active_curve = &linear_curve;
                        curve_type = CURVE_TYPE::LINEAR;
                    }

                    active_curve->ForceInterpolation();
                }

                if (event.key.code == sf::Keyboard::B)
                {
                    if (primitive_type == sf::PrimitiveType::LineStrip)
                    {
                        primitive_type = sf::PrimitiveType::Points;
                    }
                    else
                    {
                        primitive_type = sf::PrimitiveType::LineStrip;
                    }
                }

                if (event.key.code == sf::Keyboard::C)
                {
                    is_close_loop ^= true;
                    active_curve->CloseLoop(is_close_loop);
                }

                if (event.key.code == sf::Keyboard::Add)
                {
                    curve_samples = std::clamp((curve_samples + 1), 0, 100);
                }

                if (event.key.code == sf::Keyboard::Subtract)
                {
                    curve_samples = std::clamp((curve_samples - 1), 0, 100);
                }

                if (event.key.code == sf::Keyboard::P)
                {
                }

                if (event.key.code == sf::Keyboard::Escape)
                {
                    window.close();
                }
            }
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
        {
            p_mouse_x = mouse_x;
            p_mouse_y = mouse_y;

            mouse_x = sf::Mouse::getPosition(window).x;
            mouse_y = sf::Mouse::getPosition(window).y;

            if (ignore_click) // ignore the 1st click to avoid getting a false dt value
            {
                mouse_x_dt = 0;
                mouse_y_dt = 0;
                ignore_click = false;
                bool found_vertex = false;

                // do a simple linear search to see which point has been selected and set the control point if found
                //for (size_t i=0; i<line_draw_shape.size(); i++)
                //for (size_t i=0; i<curve_data_cubic->pointList.size(); i++)
                for (size_t i=0; i<active_curve->GetPointData().size(); i++)
                {
                    //float mouse_object_position_x_relative = std::abs(static_cast<float>(mouse_x) - line_draw_shape[i].position.x);
                    //float mouse_object_position_y_relative = std::abs(static_cast<float>(mouse_y) - line_draw_shape[i].position.y);
                    //float mouse_object_position_x_relative = std::abs(static_cast<float>(mouse_x) - curve_data_cubic->pointList[i][0]);
                    //float mouse_object_position_y_relative = std::abs(static_cast<float>(mouse_y) - curve_data_cubic->pointList[i][1]);
                    float mouse_object_position_x_relative = std::abs(static_cast<float>(mouse_x) - active_curve->GetPointData()[i][0]);
                    float mouse_object_position_y_relative = std::abs(static_cast<float>(mouse_y) - active_curve->GetPointData()[i][1]);

                    if ((mouse_object_position_x_relative < circle_draw_shape.getRadius()) && (mouse_object_position_y_relative < circle_draw_shape.getRadius()))
                    {
                        l_mouse_x = mouse_x;
                        l_mouse_y = mouse_y;

                        //last_selected_position = line_draw_shape[i].position;
                        //last_selected_position = sf::Vector2f(curve_data_cubic->pointList[i][0], curve_data_cubic->pointList[i][1]);
                        last_selected_position = sf::Vector2f(active_curve->GetPointData()[i][0], active_curve->GetPointData()[i][1]);

                        //line_draw_shape[control_point].color = sf::Color::Black; // set last control point color back to white
                        control_point = static_cast<int32_t>(i);
                        //line_draw_shape[control_point].color = sf::Color(215, 95, 115, 255); // set selected control point color to Blue

                        has_been_selected = true;
                        found_vertex = true;
                    }
                    else
                    {
                        //line_draw_shape[i].color = sf::Color::Black;
                    }
                }

                if (control_key_down && alt_key_down && !found_vertex)
                {
                    // add point to beginning of the curve
                    active_curve->AddAnchor({static_cast<float>(mouse_x), static_cast<float>(mouse_y)}, PLACE_ANCHOR::BEG);
                }
                else if (control_key_down && shift_key_down && !found_vertex)
                {
                    // add point to an intersecting point on the curve. work/curve types need to match to do an insertion.
                    if (curve_type == active_curve->WorkCurveType())
                    {
                        std::pair<std::array<float, 2>, int32_t> position_index = active_curve->IntersectionOnCurve({static_cast<float>(mouse_x), static_cast<float>(mouse_y)});
                        active_curve->InsertAnchor(position_index.first, position_index.second);
                    }
                }
                else if (control_key_down && !found_vertex)
                {
                    // add new point to the end of the curve
                    auto add_new_point = sf::Vertex({static_cast<float>(mouse_x), static_cast<float>(mouse_y)}, sf::Color::White);
                    //hover_anim.push_back(0.0f);
                    //line_draw_shape.push_back(add_new_point);

                    //active_curve->AddPoint({add_new_point.position.x, add_new_point.position.y});
                    active_curve->AddAnchor({add_new_point.position.x, add_new_point.position.y}, PLACE_ANCHOR::END);

                    //line_draw_shape[control_point].color = sf::Color::Black; // set last control point color back to white
                    //control_point = static_cast<int32_t>(line_draw_shape.size())-1;
                    //line_draw_shape[control_point].color = sf::Color(215, 68, 92, 255); // set selected control point color to Blue
                }

                if (!found_vertex)
                {
                    control_point = -1;
                }
            }
            else
            {
                constexpr float dt_strength_factor = 1.0f; // increase the strength/amount of velocity from the mouse movement
                mouse_x_dt = static_cast<float>(mouse_x - p_mouse_x) * dt_strength_factor;
                mouse_y_dt = static_cast<float>(mouse_y - p_mouse_y) * dt_strength_factor;

                if (has_been_selected)
                {
                    new_position_x = static_cast<float>(mouse_x - l_mouse_x) + last_selected_position.x;
                    new_position_y = static_cast<float>(mouse_y - l_mouse_y) + last_selected_position.y;
                    //line_draw_shape[control_point].position = {new_position_x, new_position_y};

                    if (shift_key_down)
                    {
                        active_curve->UpdatePoint(control_point, {new_position_x, new_position_y}, CURVE_CONTROL::FREE);
                    }
                    else
                    {
                        active_curve->UpdatePoint(control_point, {new_position_x, new_position_y}, CURVE_CONTROL::ALIGNMENT);
                    }
                }
            }
        }
        else
        {
            if (has_been_selected)
            {
                /*
                if (((line_draw_shape[control_point].position.x) < 0 || (line_draw_shape[control_point].position.x) > WINDOW_WIDTH) ||
                    ((line_draw_shape[control_point].position.y) < 0 || (line_draw_shape[control_point].position.y) > WINDOW_HEIGHT))
                {
                    line_draw_shape[control_point].position = last_selected_position;
                }
                 */
            }

            has_been_selected = false;
            ignore_click = true;
            mouse_x = 0;
            mouse_y = 0;
            last_selected_position = {0.0, 0.0f};
        }

        // draw

        txt_line_mode_message_render.setString("");

        window.clear();

        std::vector<sf::Vertex> fill_buffer;
        if (curve_type == CURVE_TYPE::CUBIC)
        {
#if USE_OLD_CURVES
            if (!draw_cubic_curve(line_draw_shape, static_cast<float>(curve_samples), window, primitive_type, fill_buffer, hide_points))
            {
                txt_line_mode_message_render.setString("Need a minimal of 4 points to draw a cubic bezier curve!");
                txt_line_mode_message_render.setPosition((WINDOW_WIDTH - (txt_line_mode_message_render.getLocalBounds().width + 2)) / 2, WINDOW_HEIGHT-30);
            }
#else
            draw_cubic_curve.HoverAnimation(&cubic_curve, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
            draw_cubic_curve.SelectedPoint(control_point);
            draw_cubic_curve.RenderCurve(&cubic_curve, window, primitive_type);

            if (control_key_down && shift_key_down)
            {
                draw_cubic_curve.DrawIntersectionPoint(&cubic_curve, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, window);
            }

            draw_cubic_curve.DrawPoints(&cubic_curve, !hide_points, window);
#endif
        }
        else if (curve_type == CURVE_TYPE::QUADRATIC)
        {
#if USE_OLD_CURVES
            if (!draw_quadratic_curve(line_draw_shape, static_cast<float>(curve_samples), window, primitive_type, fill_buffer, hide_points))
            {
                txt_line_mode_message_render.setString("Need a minimal of 3 points to draw a quadratic bezier curve!");
                txt_line_mode_message_render.setPosition((WINDOW_WIDTH - (txt_line_mode_message_render.getLocalBounds().width + 2)) / 2, WINDOW_HEIGHT-30);
            }
#else

            d_linear_curve.HoverAnimation(&quadratic_curve, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
            d_linear_curve.SelectedPoint(control_point);
            d_linear_curve.RenderCurve(&quadratic_curve, window, primitive_type);

            if (control_key_down && shift_key_down)
            {
                d_linear_curve.DrawIntersectionPoint(&quadratic_curve, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, window);
            }

            d_linear_curve.DrawPoints(&quadratic_curve, !hide_points, window);

#endif
        }
        else //if (curve_type == CURVE_TYPE::LINEAR)
        {
#if USE_OLD_CURVES
            if(!draw_linear_curve(line_draw_shape, static_cast<float>(curve_samples), window, primitive_type, fill_buffer, hide_points))
            {
                txt_line_mode_message_render.setString("Need a minimal of 2 points to draw a linear 'bezier' curve!");
                txt_line_mode_message_render.setPosition((WINDOW_WIDTH - (txt_line_mode_message_render.getLocalBounds().width + 2)) / 2, WINDOW_HEIGHT-30);
            }
#else
            d_linear_curve.HoverAnimation(&linear_curve, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
            d_linear_curve.SelectedPoint(control_point);
            d_linear_curve.RenderCurve(&linear_curve, window, primitive_type);

            if (control_key_down && shift_key_down)
            {
                d_linear_curve.DrawIntersectionPoint(&linear_curve, sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y, window);
            }

            d_linear_curve.DrawPoints(&linear_curve, !hide_points, window);
#endif
        }

        if (show_text)
        {
            window.draw(txt_control_line_render);
            window.draw(txt_line_mode_message_render);
        }

        if (!hide_points)
        {
            int32_t mouse_hover_x = sf::Mouse::getPosition(window).x;
            int32_t mouse_hover_y = sf::Mouse::getPosition(window).y;
            int32_t k=0;
            //for (const auto & v : line_draw_shape)
            /*
            for (const auto & v : curve_data_cubic->pointList)
            {
                //float mouse_object_position_x_relative = std::abs(static_cast<float>(mouse_hover_x) - v.position.x);
                //float mouse_object_position_y_relative = std::abs(static_cast<float>(mouse_hover_y) - v.position.y);
                float mouse_object_position_x_relative = std::abs(static_cast<float>(mouse_hover_x) - v[0]);
                float mouse_object_position_y_relative = std::abs(static_cast<float>(mouse_hover_y) - v[1]);
                bool hover_color = false;
/*
                if ((mouse_object_position_x_relative <= point_radius_size) &&
                    (mouse_object_position_y_relative <= point_radius_size))
                {
                    hover_anim[k] += hover_anim[k] + 0.1f;
                    hover_color = true;
                }
                else
                {
                    hover_anim[k] -= ((5.0f - hover_anim[k]) + 0.1f);
                }

                hover_anim[k] = std::clamp(hover_anim[k], 0.0f, 5.0f);

                circle_draw_shape.setOutlineThickness(2.0f);
                circle_draw_shape.setOutlineColor(sf::Color::White);

                //circle_draw_shape.setRadius(point_radius_size + hover_anim[k]);

                //circle_draw_shape.setPosition(v.position - sf::Vector2(circle_draw_shape.getRadius(), circle_draw_shape.getRadius()));
                circle_draw_shape.setPosition(sf::Vector2f(v[0], v[1])- sf::Vector2(circle_draw_shape.getRadius(), circle_draw_shape.getRadius()));

                //auto p_color = v.color;
                auto p_color = sf::Color::Black;
                if (hover_color)
                {
                    if(control_point == k)
                    {
                        circle_draw_shape.setFillColor(sf::Color(30, 156, 239, 255));
                        //circle_draw_shape.setOutlineColor(v.color);
                        circle_draw_shape.setOutlineColor(sf::Color::White);
                    }
                    else
                    {
                        circle_draw_shape.setFillColor(sf::Color(30, 156, 239, 255));
                    }
                }
                else
                {
                    //circle_draw_shape.setFillColor(v.color);
                    circle_draw_shape.setFillColor(sf::Color::White);
                }

                window.draw(circle_draw_shape);
                circle_draw_shape.setFillColor(p_color);
                circle_draw_shape.setOutlineColor(sf::Color::White);

                k++;
            }
            */
        }

        if (show_fill)
        {
            /*
            if (!fill_buffer.empty())
            {
                fill_shape.setPointCount(fill_buffer.size());
                fill_shape.setFillColor(sf::Color::Yellow);

                for (size_t i = 0; i < fill_buffer.size(); i++)
                    fill_shape.setPoint(i, fill_buffer[i].position);

                window.draw(fill_shape);
            }
             */
        }

        n_frames++;
        window.display();
    }

    return EXIT_SUCCESS;
}

sf::Vertex linear_curve(sf::Vertex p0, sf::Vertex p1, float t)
{
    return  p0.position + t * (p1.position - p0.position);
}

sf::Vertex quadratic_curve(sf::Vertex p0, sf::Vertex p1, sf::Vertex p2, float t)
{
    auto p_0 = linear_curve(p0, p1, t); // line from point p_0 to p_1
    auto p_1 = linear_curve(p1, p2, t); // line from point p_1 to p_2

    // the 2 lines together actual form a line tangent of the quadratic curve of the control point (p1)
    // do s linear curve on the results and get the quadratic (this can be done in a more straight forward way)

    return linear_curve(p_0, p_1, t);
}

sf::Vertex cubic_curve(sf::Vertex p0, sf::Vertex p1, sf::Vertex p2, sf::Vertex p3, float t)
{
    auto p_0 = quadratic_curve(p0, p1, p2, t); // line from point p_0 to p_1
    auto p_1 = quadratic_curve(p1, p2, p3, t); // line from point p_1 to p_2

    // the 2 lines together actual form a line tangent of the quadratic curve of the control point (p1)
    // do s linear curve on the results and get the quadratic (this can be done in a more straight forward way)

    return linear_curve(p_0, p_1, t);
}

bool draw_linear_curve(sf::Vertex p0, sf::Vertex p1, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type)
{
    bool is_drawing = true;
    std::vector<sf::Vertex> vertex_list;

    constexpr float t_constrain_beg = 0.0f;
    constexpr float t_constrain_end = 1.0f;

    float t = t_constrain_beg; // 0 <= t <= 1.0

    float step_size = 1.0f / s; // the larger s is the smoother the line
    constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

    while (t <= (t_constrain_end + epsilon))
    {
        t = std::clamp(t, t_constrain_beg, t_constrain_end);

        sf::Vertex v = linear_curve(p0, p1, t);
        vertex_list.push_back(v);
        t += step_size;
    }

    window.draw(vertex_list.data(), vertex_list.size(), primitive_type);

    return is_drawing;
}

bool draw_linear_curve(const std::vector<sf::Vertex>& points, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type, std::vector<sf::Vertex>& result, bool hide_control_point_lines)
{
    bool is_drawing = false;
    constexpr size_t min_points = 2;
    if (points.size() >= min_points)
    {
        is_drawing = true;
        std::vector<sf::Vertex> vertex_list;

        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;
        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / s; // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        const size_t n_iterations = points.size()-1;

        for (size_t i=0; i<n_iterations; i++)
        {
            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);

                sf::Vertex v = linear_curve(points[i], points[i + 1], t);
                vertex_list.push_back(v);

                t += step_size;
            }

            t = t_constrain_beg;
        }

        window.draw(vertex_list.data(), vertex_list.size(), primitive_type);
        result = vertex_list;
    }

    return is_drawing;
}

bool draw_quadratic_curve(sf::Vertex p0, sf::Vertex p1, sf::Vertex p2, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type)
{
    bool is_drawing = true;
    std::vector<sf::Vertex> vertex_list;

    constexpr float t_constrain_beg = 0.0f;
    constexpr float t_constrain_end = 1.0f;

    float t = t_constrain_beg; // 0 <= t <= 1.0

    float step_size = 1.0f / s; // the larger s is the smoother the line
    constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

    while (t <= (t_constrain_end + epsilon))
    {
        t = std::clamp(t, t_constrain_beg, t_constrain_end);

        sf::Vertex v = quadratic_curve(p0, p1, p2, t);
        vertex_list.push_back(v);
        t += step_size;
    }

    window.draw(vertex_list.data(), vertex_list.size(), primitive_type);

    return is_drawing;
}

bool draw_quadratic_curve(const std::vector<sf::Vertex>& points, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type, std::vector<sf::Vertex>& result, bool hide_control_point_lines)
{
    bool is_drawing = false;
    constexpr size_t min_points = 3;
    if (points.size() >= min_points)
    {
        is_drawing = true;
        std::vector<sf::Vertex> vertex_list;

        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;

        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / s; // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        sf::Vertex v;
        sf::Vertex rv;

        size_t n_iterations = ((points.size() - 3)  / 2) + 1;
        n_iterations = std::clamp(n_iterations, size_t(0), std::numeric_limits<size_t>::max());

        for (size_t i=0; i<n_iterations; i++)
        {
            uint32_t point_a = (i * 2) + 0;
            uint32_t point_b = (i * 2) + 1;
            uint32_t point_c = (i * 2) + 2;

            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);
                v = quadratic_curve(points[point_a], points[point_b], points[point_c], t);
                vertex_list.push_back(v);
                t += step_size;
            }

            t = t_constrain_beg;

            if (!hide_control_point_lines)
            {
                draw_linear_curve(points[point_a], points[point_b], 1.0f, window);
            }

        }

        window.draw(vertex_list.data(), vertex_list.size(), primitive_type);
        result = vertex_list;
    }

    return is_drawing;
}

bool draw_cubic_curve(sf::Vertex p0, sf::Vertex p1, sf::Vertex p2, sf::Vertex p3, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type)
{
    bool is_drawing = true;
    std::vector<sf::Vertex> vertex_list;

    constexpr float t_constrain_beg = 0.0f;
    constexpr float t_constrain_end = 1.0f;

    float t = t_constrain_beg; // 0 <= t <= 1.0

    float step_size = 1.0f / s; // the larger s is the smoother the line
    constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

    while (t <= (t_constrain_end + epsilon))
    {
        t = std::clamp(t, t_constrain_beg, t_constrain_end);

        sf::Vertex v = cubic_curve(p0, p1, p2, p3, t);
        vertex_list.push_back(v);
        t += step_size;
    }

    window.draw(vertex_list.data(), vertex_list.size(), primitive_type);

    return is_drawing;
}

bool draw_cubic_curve(const std::vector<sf::Vertex>& points, float s, sf::RenderWindow & window, sf::PrimitiveType primitive_type, std::vector<sf::Vertex>& result, bool hide_control_point_lines)
{
    bool is_drawing = false;
    constexpr size_t min_points = 4;
    if (points.size() >= min_points)
    {
        is_drawing = true;
        std::vector<sf::Vertex> vertex_list;

        constexpr float t_constrain_beg = 0.0f;
        constexpr float t_constrain_end = 1.0f;

        float t = t_constrain_beg; // 0 <= t <= 1.0

        float step_size = 1.0f / s; // the larger s is the smoother the line
        constexpr float epsilon = 0.0001f; // for step size values that may not exactly reach 1.0f

        sf::Vertex v;
        sf::Vertex rv;

        size_t n_iterations = ((points.size() - 4)  / 3) + 1;
        n_iterations = std::clamp(n_iterations, size_t(0), std::numeric_limits<size_t>::max());

        for (size_t i=0; i<n_iterations; i++)
        {
            uint32_t point_a = (i * 3) + 0;
            uint32_t point_b = (i * 3) + 1;
            uint32_t point_c = (i * 3) + 2;
            uint32_t point_d = (i * 3) + 3;

            while (t <= (t_constrain_end + epsilon))
            {
                t = std::min(t, t_constrain_end);
                v = cubic_curve(points[point_a], points[point_b], points[point_c], points[point_d], t);
                vertex_list.push_back(v);
                t += step_size;
            }

            t = t_constrain_beg;

            if (!hide_control_point_lines)
            {
                draw_linear_curve(points[point_a], points[point_b], 1.0f, window);
                draw_linear_curve(points[point_c], points[point_d], 1.0f, window);
            }

        }

        bool close_the_loop = false;
        if (close_the_loop)
        {
            if (points.size() > 0)
            {
                t = t_constrain_beg;
                while (t <= (t_constrain_end + epsilon))
                {
                    t = std::min(t, t_constrain_end);
                    v = cubic_curve(points[points.size()-1], ((points[points.size()-1].position - points[0].position) / 2.0f) + sf::Vector2f(-50.0f,50.0f), ((points[points.size()-1].position - points[0].position) / 2.0f) +  - sf::Vector2f(50.0f,-50.0f), points[0], t);
                    vertex_list.push_back(v);
                    t += step_size;
                }
            }
        }

        window.draw(vertex_list.data(), vertex_list.size(), primitive_type);
        result = vertex_list;
    }

    return is_drawing;
}
