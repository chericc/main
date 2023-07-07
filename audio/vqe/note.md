# VQE

VQE: Voice Quality Enhancement.

音质增强；

## 主要模块

### AEC

Acoustic Echo Cancellation.

回声抵消；

远端语音数据在本地扬声器播放同时本地麦克风采集声音的场景下，抵消本地麦克风采集到的本地扬声器播放的声音；

### NR

Noise Reduction.

噪声消除；

去除不需要的噪音，比如语音场景下会过滤除语音之外的其它声音；

### DRC

Dynamic Range Control.

动态压缩控制；

控制输出增益，保证声音不至于过大或过小；（输出是有限制的）

### PEQ

Parameter Equalizer.

参量均衡器；

调节音频数据中各个频段的增益；

### HDR

High Dynamic Rage.

高动态范围；

输入增益控制，保证声音不至于过大或国小；（输入是有限制的）

### HPF

High-Pass Filter.

高通滤波；

高通低阻，去除低频声音；

### AGC

Auto Gain Control.

自动增益控制；

在声音输入音量大小变化时，将输出音量控制在比较一致的范围内；

### RES

Resample.

重采样；

可作为各个处理模块之间的数据兼容；