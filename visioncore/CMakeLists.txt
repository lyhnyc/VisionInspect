cmake_minimum_required(VERSION 3.20)
project(visioncore VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS OFF)

# ---- 依赖（按你机器的实际路径修改） ----
# 方式一：vcpkg（推荐，配置 CMAKE_TOOLCHAIN_FILE 后 find_package 自动生效）
# 方式二：预编译 OpenCV，直接 set OpenCV_DIR
find_package(OpenCV REQUIRED COMPONENTS core imgproc imgcodecs features2d)
find_package(Eigen3 CONFIG QUIET)   # vcpkg: eigen3
find_package(GTest CONFIG QUIET)    # vcpkg: gtest
find_package(Benchmark CONFIG QUIET) # vcpkg: benchmark

# Intel IPP（oneAPI 安装后取消注释并改路径）
# set(IPP_DIR "C:/Program Files (x86)/Intel/oneAPI/ipp/latest/lib/cmake/ippi")
# find_package(IPP REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

# ---- 核心库 ----
add_library(visioncore SHARED
    src/common/interop.cpp        # C ABI 导出层（唯一对外接口）
    src/common/image_utils.cpp
    src/common/pose_math.cpp      # Eigen 位姿/几何工具
    src/locate/template_locator.cpp
    src/measure/caliper.cpp
    src/defect/blob_detector.cpp
    src/ocr/mark_inspector.cpp
)
target_include_directories(visioncore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(visioncore PUBLIC ${OpenCV_LIBS})
if(TARGET Eigen3::Eigen)
    target_link_libraries(visioncore PUBLIC Eigen3::Eigen)
endif()

# 导出纯 C 接口（供 C# P/Invoke），稳定 ABI
set_target_properties(visioncore PROPERTIES PREFIX "")

# ---- 单元测试 ----
if(TARGET GTest::gtest)
    enable_testing()
    add_executable(vc_tests
        tests/test_blob.cpp
        tests/test_caliper.cpp
        tests/test_pose_math.cpp
    )
    target_link_libraries(vc_tests PRIVATE visioncore GTest::gtest GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(vc_tests)
endif()

# ---- 基准测试 ----
if(TARGET benchmark::benchmark)
    add_executable(vc_bench
        bench/bench_preprocess.cpp
        bench/bench_blob.cpp
    )
    target_link_libraries(vc_bench PRIVATE visioncore benchmark::benchmark)
endif()
