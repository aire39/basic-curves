cmake_minimum_required(VERSION 3.20)
project("basic_bezier_curves")

# set cxx standard
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_C_STANDARD 11)

find_package (Threads REQUIRED)

if (CMAKE_BUILD_TYPE EQUAL "Debug")
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options("-O0")
    endif()
elseif(CMAKE_BUILD_TYPE EQUAL "Release") 
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options("-O3")
    endif()
endif()

set(BUILD_SHARED_LIBS FALSE) # build using the static libraries

add_subdirectory(libs/SFML)

add_executable(basic_bezier_curves
               main.cpp
               Curve.h
               CubicCurve.cpp CubicCurve.h
               LinearCurve.cpp LinearCurve.h
               CurveEffect.cpp CurveEffect.h
        DrawCurve.cpp DrawCurve.h)

target_link_libraries(basic_bezier_curves
                      sfml-window
                      sfml-graphics
                      Threads::Threads)

target_include_directories(basic_bezier_curves PUBLIC
                           $<TARGET_PROPERTY:sfml-window,INTERFACE_INCLUDE_DIRECTORIES>
                           $<TARGET_PROPERTY:sfml-graphics,INTERFACE_INCLUDE_DIRECTORIES>)
