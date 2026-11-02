# TCP 通讯协议 v1（visionapp ↔ MES/测试机模拟端）

- 传输：TCP，上位机为 Server，默认端口 8899
- 编码：UTF-8 JSON，每条报文以 `\n` 结尾（防止粘包歧义）
- 心跳：双方 2s 间隔互发 heartbeat，10s 未收到判离线

## 报文

### 1) 心跳（双向）
```json
{"type":"heartbeat","ts":"2026-11-09T10:00:00"}
```

### 2) 远程触发（MES/模拟端 → 上位机）
```json
{"type":"trigger","job":"chip_a","seq":12345}
```
- `seq`：流水号，结果报文原样带回

### 3) 结果上报（上位机 → MES）
```json
{
  "type":"result",
  "seq":12345,
  "verdict":"OK",
  "defects":[
    {"type":1,"x":110.5,"y":80.2,"w":30.1,"h":6.4,"area":132.7}
  ],
  "measure":{"mark_offset_mm":0.021,"width_mm":14.982},
  "mark":{"ocr":"ACME-7411","dmc":true},
  "elapsed_ms":86.4
}
```

### 4) 复判指令（MES → 上位机，对应 RETRY 流程）
```json
{"type":"recheck","seq":12345,"recipe":"chip_a_relax"}
```

## 判级语义
- `OK`：放行
- `NG`：剔除（缺陷列表随结果上报，供 SPC 分析）
- `RETRY`：算法置信度不足，请求重拍/复判

## 错误处理
| 场景 | 行为 |
|---|---|
| JSON 解析失败 | 回 `{"type":"error","code":"bad_json"}`，不断连 |
| seq 重复 | 丢弃并计数（日志） |
| 客户端 10s 无心跳 | 判离线，重连后重新握手 |
