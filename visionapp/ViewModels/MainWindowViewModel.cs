// MainWindowViewModel —— CommunityToolkit.Mvvm 最小 MVVM
using System;
using System.Collections.ObjectModel;
using System.Threading.Tasks;
using System.Windows.Media.Imaging;
using CommunityToolkit.Mvvm.ComponentModel;
using CommunityToolkit.Mvvm.Input;
using VisionApp.NativeInterop;
using VisionApp.Services;

namespace VisionApp.ViewModels
{
    public partial class DetectRecord : ObservableObject
    {
        public string TimeStr { get; set; } = "";
        public string Verdict { get; set; } = "";
        public int DefectCount { get; set; }
        public double ElapsedMs { get; set; }
    }

    public partial class MainWindowViewModel : ObservableObject
    {
        private readonly CameraService _camera = new();
        private readonly PipelineService _pipeline = new();

        [ObservableProperty] private BitmapSource? _currentImage;
        [ObservableProperty] private string _statusText = "未连接";
        [ObservableProperty] private bool _isContinuous;

        public ObservableCollection<DetectRecord> Results { get; } = new();

        [RelayCommand]
        private async Task ConnectCameraAsync()
        {
            StatusText = "连接中…";
            var ok = await Task.Run(() => _camera.Connect());
            StatusText = ok ? "相机已连接（模拟）" : "连接失败";
        }

        [RelayCommand]
        private async Task TriggerOnceAsync()
        {
            var frame = _camera.GrabFrame();
            if (frame == null) { StatusText = "取图失败"; return; }
            var sw = System.Diagnostics.Stopwatch.StartNew();
            var r = await Task.Run(() => _pipeline.Process(frame));
            sw.Stop();

            CurrentImage = frame.ToBitmapSource(r.Defects);
            Results.Insert(0, new DetectRecord
            {
                TimeStr = DateTime.Now.ToString("HH:mm:ss"),
                Verdict = r.Verdict,
                DefectCount = r.Defects.Count,
                ElapsedMs = Math.Round(sw.Elapsed.TotalMilliseconds, 1)
            });
            StatusText = $"完成：{r.Verdict}（{sw.ElapsedMilliseconds}ms）";
        }

        [RelayCommand]
        private void ToggleContinuous()
        {
            IsContinuous = !IsContinuous;
            StatusText = IsContinuous ? "连续运行中…" : "已停止";
            if (IsContinuous) _ = RunLoopAsync();
        }

        private async Task RunLoopAsync()
        {
            while (IsContinuous)
            {
                TriggerOnceCommand.Execute(null);
                await Task.Delay(120);   // 目标节拍 ≤120ms
            }
        }

        [RelayCommand]
        private void Stop()
        {
            IsContinuous = false;
            StatusText = "已停止";
        }
    }
}
