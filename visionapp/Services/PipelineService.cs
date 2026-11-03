// PipelineService.cs —— 检测流水线编排：定位 → 缺陷 → 测量 → 判级
// （D6 后重构为 C++ 侧三级流水线，本类保留为编排与判级层）
using System;
using System.Collections.Generic;
using VisionApp.NativeInterop;

namespace VisionApp.Services
{
    public class DefectBox
    {
        public float X, Y, W, H;    // 显示用（像素）
        public int Type;            // 0=未知 1=划痕 2=缺失 3=污点
    }

    public class DetectResult
    {
        public string Verdict = "OK";       // OK / NG / RETRY
        public List<DefectBox> Defects = new();
        public double LocateScore;
    }

    public class PipelineService
    {
        private VisionEngine? _engine;

        private VisionEngine Engine => _engine ??= new VisionEngine();

        public DetectResult Process(Frame frame)
        {
            var r = new DetectResult();
            try
            {
                Engine.LoadImage(frame.Data, frame.Width, frame.Height,
                                 frame.Stride, frame.Channels);

                // 1) 定位（BLOB 结果的坐标可由定位结果做旋正，此处先直接检测）
                var loc = Engine.Locate();
                r.LocateScore = loc.Score;

                // 2) 缺陷检测
                var blobs = Engine.DetectDefects(minArea: 30f, maxArea: 5000f);
                foreach (var b in blobs)
                    r.Defects.Add(new DefectBox
                    {
                        X = b.Center.X - b.Width / 2,
                        Y = b.Center.Y - b.Height / 2,
                        W = b.Width, H = b.Height, Type = b.Type
                    });

                // 3) 判级：无缺陷=OK；>=1=NG
                r.Verdict = r.Defects.Count == 0 ? "OK" : "NG";
            }
            catch (VisionCoreException)
            {
                r.Verdict = "RETRY";   // 算法级失败 → 降级不崩溃
            }
            return r;
        }
    }

    public static class FrameExtensions
    {
        // Frame → BitmapSource 并叠加缺陷框（D7 实现 WriteableBitmap 高效路径）
        public static System.Windows.Media.Imaging.BitmapSource ToBitmapSource(
            this Frame f, IEnumerable<DefectBox> defects)
        {
            var handle = System.Runtime.InteropServices.GCHandle.Alloc(
                f.Data, System.Runtime.InteropServices.GCHandleType.Pinned);
            try
            {
                var bs = System.Windows.Media.Imaging.BitmapSource.Create(
                    f.Width, f.Height, 96, 96,
                    System.Windows.Media.PixelFormats.Bgr24, null,
                    handle.AddrOfPinnedObject(), f.Stride * f.Height, f.Stride);
                bs.Freeze();
                return bs;
            }
            finally { handle.Free(); }
        }
    }
}
