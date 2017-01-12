cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(skeleton_mesh_nearest_points)

find_package(PCL 1.2 REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_executable (skeleton_mesh_nearest_points skeleton_mesh_nearest_points.cpp)
target_link_libraries (skeleton_mesh_nearest_points ${PCL_LIBRARIES})