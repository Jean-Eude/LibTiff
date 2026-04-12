cmake_minimum_required(VERSION 3.16)
project(main)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(
    ../include
    ../include/libtiff
    ../include/tiffdp
    ../include/matplot
)

link_directories(../lib)

file(GLOB SOURCES ../*.cpp ../src/*.cpp)

add_executable(main ${SOURCES})

target_compile_options(main PRIVATE -Wall -O3)

# =========================
# TIFF + matplot
# =========================
target_link_libraries(main PRIVATE
    -ltiff
    matplot
)

# =========================
# VTK (CORRECT FIX UBUNTU 9.1)
# =========================
find_package(VTK REQUIRED COMPONENTS
    CommonCore
    CommonDataModel
    FiltersSources
    RenderingCore
    RenderingOpenGL2
    RenderingVolume
    RenderingVolumeOpenGL2
    InteractionStyle
)

target_link_libraries(main PRIVATE
    VTK::CommonCore
    VTK::CommonDataModel
    VTK::FiltersSources
    VTK::RenderingCore
    VTK::RenderingOpenGL2
    VTK::RenderingVolume
    VTK::RenderingVolumeOpenGL2
    VTK::InteractionStyle
)

vtk_module_autoinit(
    TARGETS main
    MODULES ${VTK_LIBRARIES}
)