# WinUI 3 项目 - 构建说明

## 当前状态

✅ 所有源代码和项目文件已创建完成
⚠️ 构建需要完整版 Visual Studio IDE

## 找到的 Visual Studio 安装

- **位置**: `E:\VS`
- **类型**: 完整版 Visual Studio 2022
- **IDE**: `E:\VS\Common7\IDE\devenv.exe`
- **VC工具**: `E:\VS\VC\Tools\MSVC\14.50.35717\`

## 推荐构建方式

### 方式 1：使用 Visual Studio IDE（推荐）

1. 双击运行以下任一脚本：
   - `E:\UltraImageViewer\winui\UltraImageViewer.WinUI\open_and_build.bat`
   - 或直接双击 `UltraImageViewer.WinUI.sln`

2. 在 Visual Studio 中：
   - 等待 NuGet 包自动还原
   - 按 `Ctrl+Shift+B` 构建
   - 或按 `F5` 直接运行

### 方式 2：使用命令行构建

打开 **x64 Native Tools Command Prompt for VS 2022**，然后：

```cmd
cd E:\UltraImageViewer\winui\UltraImageViewer.WinUI
msbuild UltraImageViewer.WinUI.sln /p:Configuration=Release /p:Platform=x64
```

### 方式 3：使用构建脚本

```cmd
E:\UltraImageViewer\winui\UltraImageViewer.WinUI\build_vs.bat
```

## 项目文件清单

```
winui/UltraImageViewer.WinUI/
├── UltraImageViewer.WinUI.sln       # VS 解决方案 ← 双击打开
├── open_and_build.bat               # 自动打开 VS
├── build_vs.bat                     # 命令行构建脚本
├── README.md                        # 功能说明
├── BUILD_INSTRUCTIONS.md            # 本文档
└── UltraImageViewer.WinUI/          # 项目目录
    ├── UltraImageViewer.WinUI.vcxproj
    ├── App.xaml / .h / .cpp
    ├── MainWindow.xaml / .h / .cpp
    ├── pch.h / .cpp
    └── app.manifest
```

## 功能清单

| 功能 | 状态 |
|------|------|
| 欢迎页动画 | ✅ |
| Open 多选 | ✅ |
| Recent 持久化 | ✅ |
| 多标签预览 | ✅ |
| 图片缩放 | ✅ |
| 自动适应 | ✅ |
| 快捷键 | ✅ |
| DPI 感知 | ✅ |

## 支持的图片格式

- JPEG (`.jpg`, `.jpeg`)
- PNG (`.png`)
- GIF (`.gif`)
- BMP (`.bmp`)
- WebP (`.webp`)
- TIFF (`.tiff`)

## 快捷键

- `Ctrl+O` - 打开文件
- `Ctrl+W` - 关闭当前标签
- `Ctrl+Tab` - 切换标签
- 鼠标滚轮 - 缩放

## 故障排除

### NuGet 还原失败

打开命令提示符，执行：
```cmd
nuget restore E:\UltraImageViewer\winui\UltraImageViewer.WinUI\UltraImageViewer.WinUI.sln
```

### 找不到 Windows App SDK

Windows App SDK 应该通过 NuGet 自动还原。如果没有，手动安装：
```cmd
nuget install Microsoft.WindowsAppSDK -Version 1.5.240802000
```

### v143 工具集找不到

在 Visual Studio 中：
1. 右键项目 → 属性
2. 配置属性 → 常规
3. 平台工具集 → 选择 "Visual Studio 2022 (v143)"

## 下一步

1. 在 Visual Studio 中打开解决方案
2. 等待 NuGet 包还原
3. 按 F5 运行应用

祝使用愉快！
