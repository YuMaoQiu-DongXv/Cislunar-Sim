# Cislunar-Sim Desktop · 地月系轨道 3D 模拟器（桌面版）

[Cislunar-Sim](../README.md) 的 Windows 桌面版，使用 **Qt 6（Widgets）+ OpenGL 3.3** 原生重写，输出单一可执行程序。

在地球 + 月球双体引力场（限制性三体问题）中，用**四阶龙格-库塔（RK4）数值积分**实时计算并可视化航天器的位置、速度与轨迹。

## 功能特性

- **3D 交互场景**：旋转 / 缩放 / 平移，真实比例天体 + 星空背景 + 轨道平面网格 + 月球轨道圆
- **两种初始轨道输入**（滑轨拖动 + 数值输入，实时预览 t=0 的位置与速度方向）
  - 位置 + 速度（坐标、速度大小、方位角 / 仰角）
  - 轨道根数（半长轴 a、偏心率 e、倾角 i、升交点赤经 Ω、近地点幅角 ω、真近点角 ν）
- **实时数据面板**：模拟时间、速度、距地面 / 月面高度、地球 / 月球引力加速度
- **轨迹显示切换**：最近 30 天（默认）/ 始终显示
- **速度可视化**：轨迹按速度着色（红 = 慢 → 蓝 = 快）、速度竖线（长度 = 速度），可四选一
- **预设方案**：保存 / 载入任意初始轨道参数（本地持久化）
- **模拟速度可调**：1× 至 1 天/秒
- **开普勒轨道预览（仅地球引力）**：「初始轨道设置」预览状态下，3D 视图同步显示绿色开普勒轨道（椭圆 / 抛物线 / 双曲线），与白色航天器起始点、青色速度箭头一起预览；点「开始模拟」后自动隐藏，复选框可随时开关

## 技术栈

Qt 6 · OpenGL 3.3 · 四阶龙格-库塔（RK4）· 限制性三体问题

## 构建

### 环境要求

- **Windows** + Visual Studio 2022（MSVC 编译器）+ Windows 10/11 SDK
- **Qt 6.7.x**（`msvc2019_64` 套件，与 MSVC 2022 ABI 兼容）
- **CMake ≥ 3.21** + **Ninja**

### 配置与编译

在已初始化 MSVC 环境的命令行（或 VS 的 "x64 Native Tools"）中：

```bash
cmake -S desktop -B desktop/build -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="D:/Qt/6.7.2/msvc2019_64"
cmake --build desktop/build
```

生成的可执行程序位于 `desktop/build/CislunarSim.exe`。

### 打包发布

```bash
windeployqt --release --no-translations --compiler-runtime desktop/build/CislunarSim.exe
```

`windeployqt` 会把 Qt 运行库复制到 exe 同级目录，之后整个目录即可拷贝分发。

## 目录结构

```
desktop/
├── CMakeLists.txt          # CMake 构建配置
├── src/
│   ├── main.cpp            # 入口
│   ├── physics.{h,cpp}     # 地月双体引力 + RK4 积分 + 轨道根数转换 + 开普勒轨道解析解
│   ├── renderer.{h,cpp}    # QOpenGLWidget + OpenGL 3.3 渲染（含开普勒绿色轨道）
│   └── mainwindow.{h,cpp}  # Qt Widgets 控制面板 + 模拟主循环
└── README.md
```

## 关联项目

- Web 版（浏览器直接运行）：[../README.md](../README.md)

## 许可证

[MIT](../LICENSE) © 2026 一只羽毛球儿
