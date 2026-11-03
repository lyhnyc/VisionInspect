# CMakeLists.txt —— visioncore 构建脚本
# 目标：visioncore 静态库 + vc_tests(gtest) + vc_bench(benchmark)
# 依赖：OpenCV（预编译版）、gtest/eigen3（vcpkg）
cmake_minimum_required(VERSION 3.20)
project(visioncore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED COMPONENTS core imgproc)
find_package(GTest CONFIG REQUIRED)

add_library(visioncore STATIC
    src/common/interop.cpp
    src/common/image_utils.cpp
    src/common/pose_math.cpp
    src/defect/blob_detector.cpp
    src/measure/caliper.cpp
    src/locate/template_locator.cpp
    src/ocr/mark_inspector.cpp
)
target_include_directories(visioncore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${OpenCV_INCLUDE_DIRS})
target_link_libraries(visioncore PUBLIC ${OpenCV_LIBS})

add_executable(vc_tests
    tests/test_caliper.cpp
    tests/test_blob.cpp
    tests/test_pose_math.cpp
)
target_link_libraries(vc_tests PRIVATE visioncore GTest::gtest GTest::gtest_main)

# vc_bench 待 D5 接入 Google Benchmark 后启用
# add_executable(vc_bench bench/bench_preprocess.cpp bench/bench_blob.cpp)
# target_link_libraries(vc_bench PRIVATE visioncore)
