// bench_blob.cpp —— BLOB 检测 benchmark（D5 填充数据）
#include <benchmark/benchmark.h>
#include <opencv2/imgproc.hpp>
#include "defect/blob_detector.h"

namespace {
cv::Mat make_chip(int w, int h) {
    cv::Mat g(h, w, CV_8UC1);
    cv::randu(g, 0, 255);
    for (int k = 0; k < 6; ++k)
        cv::rectangle(g, {200, 60 + k * 60, 240, 18}, cv::Scalar(200), -1);
    return g;
}
}

static void BM_BlobPipeline(benchmark::State& state) {
    auto img = make_chip(640, 480);
    vc::BlobDetector det;
    std::vector<vc::Blob> blobs;
    for (auto _ : state) {
        blobs.clear();
        det.detect(img, 30.f, 5000.f, blobs);
        benchmark::DoNotOptimize(blobs);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_BlobPipeline);

BENCHMARK_MAIN();
