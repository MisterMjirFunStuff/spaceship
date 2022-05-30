cmake_minimum_required(VERSION 3.10)

project(astroastro)

add_executable(astroastro main.cpp)

target_link_libraries(astroastro GL GLEW SDL2)
