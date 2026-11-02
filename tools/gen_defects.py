# 合成缺陷图生成器（D2 使用）
# 用法: python gen_defects.py --out data/sim_images -n 200 --defect-rate 0.35
# 生成芯片俯视图: 背景光照渐变 + 引脚行 + 随机缺陷(划痕/缺失/污点) + 标记字符
import argparse, os, math, random
import numpy as np
import cv2

def chip_image(w=640, h=480, rng=None):
    rng = rng or np.random.default_rng()
    x = np.arange(w)[None, :].repeat(h, 0)
    y = np.arange(h)[:, None].repeat(w, 1)
    img = (110 + 40 * np.sin(x / 40.0) * np.cos(y / 60.0)).astype(np.uint8)
    # 引脚行
    for k in range(6):
        y0 = 60 + k * 60
        img[y0:y0+18, 200:440] = 200
    # 标记区（深色矩形，供 OCR 测试用）
    img[400:440, 240:400] = 60
    cv2.putText(img, "ACME-7411", (248, 430), cv2.FONT_HERSHEY_SIMPLEX,
                0.7, 220, 2, cv2.LINE_AA)
    return img

def add_defect(img, rng):
    kind = rng.choice(["scratch", "missing", "smudge"])
    h, w = img.shape
    if kind == "scratch":      # 划痕：细暗线
        x0, y0 = rng.integers(100, w-100), rng.integers(50, h-50)
        ang = rng.uniform(-0.3, 0.3)
        length = rng.integers(30, 90)
        x1 = int(x0 + length * math.cos(ang)); y1 = int(y0 + length * math.sin(ang))
        cv2.line(img, (int(x0), int(y0)), (x1, y1), 30, 2)
    elif kind == "missing":    # 引脚缺失：把某引脚行抹掉
        k = rng.integers(0, 6)
        img[60+k*60:60+k*60+18, 200:440] = 110
    else:                      # 污点：暗斑
        cx, cy = rng.integers(80, w-80), rng.integers(50, h-60)
        cv2.circle(img, (cx, cy), int(rng.integers(6, 16)), 45, -1)
    return kind

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="data/sim_images")
    ap.add_argument("-n", type=int, default=200)
    ap.add_argument("--defect-rate", type=float, default=0.35)
    args = ap.parse_args()
    os.makedirs(args.out, exist_ok=True)
    rng = np.random.default_rng(42)
    labels = []
    for i in range(args.n):
        img = chip_image(rng=rng)
        kind = None
        if rng.random() < args.defect_rate:
            kind = add_defect(img, rng)
        p = os.path.join(args.out, f"chip_{i:04d}.png")
        cv2.imwrite(p, img)
        labels.append((p, kind or "ok"))
    with open(os.path.join(args.out, "labels.csv"), "w") as f:
        f.write("file,defect\n")
        for p, k in labels:
            f.write(f"{os.path.basename(p)},{k}\n")
    print(f"generated {args.n} images -> {args.out}")

if __name__ == "__main__":
    main()
