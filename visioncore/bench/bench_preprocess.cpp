// bench_preprocess.cpp —— 光场校正/Gaussian 的 OpenCV vs IPP 对比骨架（D5 填充数据）
#include <benchmark/benchmark.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core.hpp>

namespace {
cv::Mat make_gray(int w, int h) {
    cv::Mat g(h, w, CV_8UC1);
    cv::randu(g, 0, 255);
    return g;
}
}

static void BM_Gaussian_OpenCV(benchmark::State& state) {
    auto img = make_gray(2048, 2048);
    cv::Mat out;
    for (auto _ : state)
        cv::GaussianBlur(img, out, cv::Size(0, 0), 25);
    state.SetItemsProcessed(state.iterations() * img.total());
}
BENCHMARK(BM_Gaussian_OpenCV);

// TODO(D5): BM_Gaussian_IPP —— ippiFilterGauss 替换版，出对比表
// TODO(D5): BM_Absdiff_IPP / BM_Threshold_IPP

BENCHMARK_MAIN();
