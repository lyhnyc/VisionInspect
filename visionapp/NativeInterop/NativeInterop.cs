// NativeInterop.cs —— C# P/Invoke 封装层（visioncore.dll 的唯一入口）
// 结构体布局必须与 interop.h 的 pack(1) 完全一致
using System;
using System.Runtime.InteropServices;

namespace VisionApp.NativeInterop
{
    // ---------- 错误码 ----------
    public static class VcErr
    {
        public const int OK = 0;
        public const int HANDLE = -1;
        public const int PARAM = -2;
        public const int BUF_TOO_SMALL = -3;
        public const int IMAGE = -4;
        public const int NOT_FOUND = -5;
    }

    // ---------- POD 结构体（LayoutKind.Sequential + Pack=1 对齐 interop.h） ----------
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct VcPointF { public float X, Y; }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct VcLocateResult
    {
        public float Dx;        // X 平移（像素）
        public float Dy;
        public float ThetaDeg;  // 旋转角
        public float Score;
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct VcBlobResult
    {
        public VcPointF Center;
        public float Width;
        public float Height;
        public float Area;
        public int   Type;      // 0=未知 1=划痕 2=缺失 3=污点
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct VcMeasureResult
    {
        public VcPointF P1;
        public VcPointF P2;
        public float DistancePx;
        public float EdgeStrength;
    }

    // ---------- P/Invoke 绑定 ----------
    public static class Vc
    {
        const string Dll = "visioncore";

        [DllImport(Dll)] public static extern int VC_Create(int dummy);
        [DllImport(Dll)] public static extern void VC_Destroy(long long handle);

        [DllImport(Dll)]
        public static extern int VC_LoadImage(long long handle,
            byte[] data, int w, int h, int stride, int channels);

        [DllImport(Dll)]
        public static extern int VC_SetTemplate(long long handle, byte[] tpl, int w, int h);

        [DllImport(Dll)]
        public static extern int VC_Locate(long long handle, out VcLocateResult result);

        [DllImport(Dll)]
        public static extern int VC_DetectDefects(long long handle,
            float minArea, float maxArea,
            [Out] VcBlobResult[] outBuf, int maxCount, out int outCount);

        [DllImport(Dll)]
        public static extern int VC_MeasureCaliper(long long handle,
            float p0x, float p0y, float p1x, float p1y, float width,
            out VcMeasureResult result);

        [DllImport(Dll)] public static extern IntPtr VC_Version();

        public static string Version() =>
            Marshal.PtrToStringAnsi(VC_Version()) ?? "";
    }

    // ---------- RAII 风格安全封装（业务代码只跟这个类打交道） ----------
    public sealed class VisionEngine : IDisposable
    {
        private long _handle;
        private bool _disposed;

        public VisionEngine()
        {
            int h = Vc.VC_Create(0);
            if (h == 0) throw new InvalidOperationException("VC_Create failed");
            _handle = h;
        }

        public void LoadImage(byte[] data, int w, int h, int stride, int channels)
        {
            Check(Vc.VC_LoadImage(_handle, data, w, h, stride, channels));
        }

        public VcLocateResult Locate()
        {
            Check(Vc.VC_Locate(_handle, out var r));
            return r;
        }

        public VcBlobResult[] DetectDefects(float minArea, float maxArea, int maxCount = 64)
        {
            var buf = new VcBlobResult[maxCount];
            Check(Vc.VC_DetectDefects(_handle, minArea, maxArea, buf, maxCount, out var n));
            return buf[..n];
        }

        public VcMeasureResult MeasureCaliper(System.Drawing.PointF p0,
            System.Drawing.PointF p1, float width)
        {
            Check(Vc.VC_MeasureCaliper(_handle, p0.X, p0.Y, p1.X, p1.Y, width, out var r));
            return r;
        }

        private static void Check(int err)
        {
            if (err != VcErr.OK)
                throw new VisionCoreException($"visioncore error: {err}");
        }

        public void Dispose()
        {
            if (!_disposed) { Vc.VC_Destroy(_handle); _disposed = true; }
            GC.SuppressFinalize(this);
        }

        ~VisionEngine() => Dispose();
    }

    public class VisionCoreException : Exception
    {
        public VisionCoreException(string msg) : base(msg) { }
    }
}
