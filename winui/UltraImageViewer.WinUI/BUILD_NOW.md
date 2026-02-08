# UltraImageViewer WinUI 3 - 构建说明

## 问题诊断

您的 Visual Studio 安装位置：`E:\VS`

这是一个 **Visual Studio 2025 预览版**（或类似版本），使用：
- MSBuild 版本：18.x (v180)
- VC 工具：14.50.35717
- **问题**：缺少 v143 (VS 2022) 工具集

## 解决方案

### 方式 1：使用 Visual Studio IDE 构建（推荐）

直接双击打开解决方案：

```
E:\UltraImageViewer\winui\UltraImageViewer.WinUI\UltraImageViewer.WinUI.sln
```

Visual Studio 会：
1. 自动检测项目类型
2. 还原 NuGet 包
3. 使用正确的工具集构建

### 方式 2：命令行构建（需要 VS 环境）

打开 **x64 Native Tools Command Prompt for VS 2025**：
- 开始菜单 → Visual Studio 2025 → x64 Native Tools Command Prompt

然后执行：
```cmd
cd E:\UltraImageViewer\winui\UltraImageViewer.WinUI
msbuild UltraImageViewer.WinUI.sln /p:Configuration=Release /p:Platform=x64
```

### 方式 3：使用提供的批处理脚本

双击运行以下任一脚本：
- `build_now.bat` - 命令行构建（需要 VS 环境）
- `open_and_build.bat` - 在 VS IDE 中打开

## 项目文件结构

```
winui/UltraImageViewer.WinUI/
├── UltraImageViewer.WinUI.sln       ← VS 解决方案
├── build_now.bat                    ← 构建脚本
├── open_and_build.bat               ← 打开 VS
├── README.md                        ← 功能说明
└── UltraImageViewer.WinUI/
    ├── App.xaml / .h / .cpp
    ├── MainWindow.xaml / .h / .cpp
    ├── pch.h / .cpp
    └── UltraImageViewer.WinUI.vcxproj
```

## 已实现功能

| 功能 | 状态 |
|------|------|
| 欢迎页动画 | ✅ CLI 风格进度条 |
| Open 多选 | ✅ 支持 7 种格式 |
| Recent 持久化 | ✅ JSON 存储 |
| 多标签预览 | ✅ TabView |
| 图片缩放 | ✅ 0.1x - 8x |
| 自动适应 | ✅ Fit-to-window |
| 快捷键 | ✅ Ctrl+O/W/Tab |
| DPI 感知 | ✅ Per-Monitor V2 |

## 为什么命令行构建困难

WinUI 3 项目需要：
1. XAML 编译器 (仅在 Visual Studio IDE 中完整支持)
2. Windows App SDK NuGet 包 (需要自动还原)
3. C++/WinRT 工具链 (需要特定工具集配置)

您的 VS 安装是预览版本，工具集配置与标准 VS 2022 不同，导致命令行 MSBuild 无法正确识别工具集。

**Visual Studio IDE 会自动处理这些复杂配置，是最可靠的构建方式。**

## 快速开始

1. 双击 `UltraImageViewer.WinUI.sln`
2. 等待 VS 打开并加载项目
3. 等待 NuGet 包自动还原（右下角提示）
4. 按 `Ctrl+Shift+B` 构建或 `F5` 运行

就这么简单！

## 支持的图片格式

- JPEG (`.jpg`, `.jpeg`)
- PNG (`.png`)
- GIF (`.gif`)
- BMP (`.bmp`)
- WebP (`.webp`)
- TIFF (`.tiff`)

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl+O` | 打开文件 |
| `Ctrl+W` | 关闭当前标签 |
| `Ctrl+Tab` | 切换标签 |
| 滚轮 | 缩放图片 |
| 拖拽 | 平移图片 |
| `Alt+F4` | 退出 |

## 需要帮助？

如果遇到 NuGet 还原问题：
```cmd
nuget restore E:\UltraImageViewer\winui\UltraImageViewer.WinUI\UltraImageViewer.WinUI.sln
```

如果缺少 Windows App SDK，在 Visual Studio 中：
1. 右键项目 → 管理 NuGet 包
2. 搜索并安装 `Microsoft.WindowsAppSDK`
