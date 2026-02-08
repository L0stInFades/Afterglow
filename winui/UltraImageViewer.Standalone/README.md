# UltraImageViewer - 独立版本

## 🎉 编译成功！

**位置**: `E:\UltraImageViewer\winui\UltraImageViewer.Standalone\UltraImageViewer.exe`
**大小**: 186 KB
**类型**: 64位 Windows GUI 应用程序

## 功能特性

- ✅ **图片显示** - 支持 JPG, PNG, GIF, BMP, WebP, TIFF
- ✅ **缩放** - 鼠标滚轮缩放，或按 +/- 键
- ✅ **拖拽平移** - 缩放后可拖拽查看
- ✅ **Recent 记忆** - 自动保存最近打开的图片（最多10条）
- ✅ **拖放支持** - 直接拖图片文件到窗口打开
- ✅ **快捷键** - Ctrl+O 打开，0 键适应窗口
- ✅ **暗色主题** - 护眼深色界面

## 快捷键

| 快捷键 | 功能 |
|--------|------|
| `Ctrl+O` | 打开文件 |
| `+` / `滚轮上` | 放大 |
| `-` / `滚轮下` | 缩小 |
| `0` | 适应窗口 |
| 拖拽 | 平移图片 |

## 使用方法

1. 双击 `UltraImageViewer.exe` 运行
2. 按 `Ctrl+O` 打开图片，或直接拖拽图片文件到窗口
3. 使用滚轮缩放，拖拽移动

## 系统要求

- Windows 7 或更高版本
- 64位系统

## 文件说明

```
UltraImageViewer.Standalone/
├── UltraImageViewer.exe    ← 可执行文件（直接运行）
├── UltraImageViewer.cpp    ← 源代码
└── make.bat                ← 编译脚本
```

## Recent 文件存储位置

```
%LocalAppData%\UltraImageViewer_Recent.txt
```

## 重新编译

如果需要修改代码或重新编译：

```batch
双击运行 make.bat
```

或在 **x64 Native Tools Command Prompt** 中：

```cmd
cd E:\UltraImageViewer\winui\UltraImageViewer.Standalone
cl UltraImageViewer.cpp /EHsc /O2 /Fe:UltraImageViewer.exe user32.lib gdi32.lib gdiplus.lib shlwapi.lib shell32.lib ole32.lib comdlg32.lib /link /SUBSYSTEM:WINDOWS
```

## 技术栈

- 纯 Win32 API
- GDI+ 图形库
- 标准 C++ (无外部依赖)
