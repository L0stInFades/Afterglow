# UltraImageViewer WinUI 3

基于 WinUI 3 (Windows App SDK) 和 C++/WinRT 的现代图片查看器。

## 功能特性

- **欢迎页动画** - CLI 风格的加载动画
- **打开图片** - 支持多选打开图片文件（7种格式）
- **Recent 持久化** - 自动保存最近打开的图片（最多10条）
- **多标签预览** - 浏览器式标签页切换
- **图片缩放** - 支持鼠标滚轮缩放（0.1x - 8x）
- **拖拽平移** - 缩放后可拖拽平移查看
- **快捷键** - Ctrl+O 打开、Ctrl+W 关闭、Ctrl+Tab 切换
- **DPI 感知** - 高分辨率友好

## 支持的图片格式

| 格式 | 扩展名 |
|------|--------|
| JPEG | `.jpg`, `.jpeg` |
| PNG | `.png` |
| GIF | `.gif` |
| BMP | `.bmp` |
| WebP | `.webp` |
| TIFF | `.tiff` |

## 构建要求

- **Visual Studio 2022** (v17.0+)
  - 工作负载：使用 C++ 的桌面开发
- **Windows 10 SDK** (10.0.17763.0 或更高)
- **NuGet** (用于包还原)
- **Windows App SDK** 1.5+ (自动通过 NuGet 安装)

## 快速开始

### 方法 1：使用构建脚本（推荐）

```bash
cd E:\UltraImageViewer\winui\UltraImageViewer.WinUI
build.bat
```

### 方法 2：Visual Studio IDE

1. 双击打开 `UltraImageViewer.WinUI.sln`
2. 等待 NuGet 包自动还原
3. 选择 `x64` 平台和 `Debug/Release` 配置
4. 按 `F5` 或点击"本地 Windows 调试器"

### 方法 3：命令行构建

```bash
# 还原 NuGet 包
nuget restore UltraImageViewer.WinUI.sln

# 构建 Release 版本
msbuild UltraImageViewer.WinUI.sln /p:Configuration=Release /p:Platform=x64

# 运行
UltraImageViewer.WinUI\Release\x64\UltraImageViewer.WinUI.exe
```

## 项目结构

```
winui/UltraImageViewer.WinUI/
├── UltraImageViewer.WinUI.sln     # Visual Studio 解决方案
├── nuget.config                    # NuGet 配置
├── build.bat                       # 快速构建脚本
├── README.md                       # 本文档
└── UltraImageViewer.WinUI/         # 项目目录
    ├── UltraImageViewer.WinUI.vcxproj
    ├── App.xaml                    # 应用入口定义
    ├── App.xaml.h/.cpp             # 应用逻辑
    ├── MainWindow.xaml             # 主窗口 UI
    ├── MainWindow.xaml.h/.cpp      # 主窗口逻辑
    ├── pch.h/.cpp                  # 预编译头
    └── app.manifest                # DPI 感知配置
```

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl+O` | 打开文件对话框 |
| `Ctrl+W` | 关闭当前标签 |
| `Ctrl+Tab` | 切换到下一个标签 |
| 鼠标滚轮 | 缩放图片 |
| 拖拽 | 平移图片（缩放后） |
| `Alt+F4` | 退出应用 |

## 数据存储

Recent 文件列表保存在：
```
%LocalAppData\[PackageFamily]\LocalState\recent.json
```

## 已知限制

1. **XAML 编译** - 首次构建时，XAML 编译器会生成 `.g.cpp` 和 `.g.h` 文件
2. **Unpackaged 应用** - 当前配置为非打包应用，需要使用 `InitializeWithWindow` 初始化 picker
3. **格式支持** - Phase 1 仅支持常见格式，RAW 支持计划在 Phase 2

## 故障排除

### NuGet 还原失败

```bash
# 下载并安装 NuGet
https://www.nuget.org/downloads

# 手动还原
nuget restore UltraImageViewer.WinUI.sln
```

### XAML 编译错误

确保项目属性中启用了 XAML 编译：
- 右键项目 → 属性 → 通用属性 → XAML UI 设计器
- 将"XAML 编译工具"设置为"True"

### 运行时错误

确保已安装 Windows App SDK 运行时：
- [Windows App SDK Runtime下载](https://aka.ms/windowsappsdk/1.5/stable)

## Phase 2 计划

- [ ] 接入现有 C++ 解码器
- [ ] 支持 RAW 格式
- [ ] 添加滤镜功能（亮度、对比度、饱和度）
- [ ] 性能优化（大图片处理）
- [ ] 添加缩略图预览
- [ ] 支持图片元数据显示

## 许可证

遵循 UltraImageViewer 主项目的许可证。
