// CameraService.cs —— 相机抽象层：模拟相机（可回放图片序列/合成图）
// D6 真机时新增 HikCamera : ICamera（海康 MVS SDK），接口不变
using System;
using System.Collections.Generic;
using System.IO;

namespace VisionApp.Services
{
    /// <summary>相机抽象：SDK 集成的隔离层（模拟/海康/其他品牌可插拔）</summary>
    public interface ICamera
    {
        bool Connect();
        void Disconnect();
        Frame? GrabFrame();          // 软触发取一帧
        event Action<Frame>? OnFrame; // 回调模式（连续采集）
        string Name { get; }
    }

    public class Frame
    {
        public byte[] Data;          // BGR
        public int Width, Height, Stride, Channels;
        public DateTime Timestamp = DateTime.Now;
    }

    /// <summary>模拟相机：从 data/sim_images 目录轮播图片；无图时生成合成芯片图</summary>
    public class SimulatedCamera : ICamera
    {
        private readonly List<string> _files = new();
        private int _idx;
        private readonly Random _rnd = new();

        public string Name => "SimulatedCamera";

        public bool Connect()
        {
            _files.Clear();
            var dir = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, "data", "sim_images");
            if (Directory.Exists(dir))
                _files.AddRange(Directory.GetFiles(dir, "*.png"));
            return true;
        }

        public void Disconnect() { }

        public event Action<Frame>? OnFrame;

        public Frame? GrabFrame()
        {
            if (_files.Count > 0)
                return LoadPng(_files[_idx++ % _files.Count]);
            return SyntheticChip();
        }

        private static Frame? LoadPng(string path)
        {
            // 简化：由 WPF 层解码（此处占位，正式版统一用 SkiaSharp/OpenCvSharp 解码）
            return null;
        }

        private Frame SyntheticChip()
        {
            // 合成 640x480 灰度芯片图：背景 + 引脚行 + 随机划痕
            const int w = 640, h = 480, ch = 3, stride = w * ch;
            var data = new byte[h * stride];
            for (int y = 0; y < h; y++)
                for (int x = 0; x < w; x++)
                {
                    int i = y * stride + x * ch;
                    byte g = (byte)(110 + 40 * Math.Sin(x / 40.0));
                    data[i] = data[i + 1] = data[i + 2] = g;
                }
            // 引脚（亮条）
            for (int k = 0; k < 6; k++)
            {
                int y0 = 60 + k * 60;
                for (int y = y0; y < y0 + 18; y++)
                    for (int x = 200; x < 440; x++)
                    {
                        int i = y * stride + x * ch;
                        data[i] = data[i + 1] = data[i + 2] = 200;
                    }
            }
            // 随机划痕缺陷（约 1/3 概率）
            if (_rnd.Next(3) == 0)
            {
                int x0 = _rnd.Next(100, 500), y0 = _rnd.Next(50, 400);
                int len = _rnd.Next(20, 80), dx = _rnd.Next(-1, 2);
                for (int t = 0; t < len; t++)
                {
                    int x = x0 + t, y = y0 + t * dx / 3;
                    int i = y * stride + x * ch;
                    if (i + 2 < data.Length) data[i] = data[i + 1] = data[i + 2] = 30;
                }
            }
            return new Frame { Data = data, Width = w, Height = h, Stride = stride, Channels = ch };
        }
    }

    /// <summary>默认注入模拟相机；真机时替换为 HikCamera</summary>
    public class CameraService
    {
        private readonly ICamera _cam = new SimulatedCamera();
        public bool Connect() => _cam.Connect();
        public Frame? GrabFrame() => _cam.GrabFrame();
    }
}
