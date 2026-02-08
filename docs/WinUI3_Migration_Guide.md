# UltraImageViewer — WinUI 3 (XAML) 迁移实现指南

## 0. 目标与范围
**目标**
- 用 **WinUI 3 (XAML)** 实现现代 UI：欢迎页动画、Open/Recent、浏览器式标签预览。
- **高分辨率**显示（高 DPI 友好）。
- 图片支持 **自动按比例缩放** + **缩放/放大** + **拖拽平移**。
- Recent 必须 **持久化**。

**非目标（Phase 1）**
- 暂不接入现有 C++ 解码器（先用 `BitmapImage`/`StorageFile`）。
- 暂不做高级滤镜/RAW。

---

## 1. 技术栈
- **WinUI 3 (Windows App SDK)**
- **C++/WinRT**
- 编译工具：Visual Studio 2022+（或你当前 VS）

---

## 2. 目录结构（建议）
```
winui/
  UltraImageViewer.WinUI/
    UltraImageViewer.WinUI.sln
    UltraImageViewer.WinUI/
      UltraImageViewer.WinUI.vcxproj
      App.xaml
      App.xaml.h/.cpp
      MainWindow.xaml
      MainWindow.xaml.h/.cpp
      Assets/
```

---

## 3. 环境准备
### 3.1 必需组件
- Visual Studio “Desktop development with C++”
- Windows App SDK（WinUI 3）
- C++/WinRT（VS 组件或 NuGet）

### 3.2 创建项目
VS 内选择：
- **Blank App, Unpackaged (WinUI 3 in Desktop)**（推荐）
或  
- **Blank App, Packaged (WinUI 3 in Desktop)**

---

## 4. UI 设计（XAML）
### 4.1 MainWindow 布局结构
```xml
<Window ...>
  <Grid>
    <!-- Welcome Page -->
    <Grid x:Name="WelcomeRoot" Visibility="Visible">
      <StackPanel HorizontalAlignment="Center" VerticalAlignment="Center" Width="600" Spacing="16">
        <TextBlock Text="UltraImageViewer" FontSize="32" FontWeight="SemiBold" />
        <TextBlock x:Name="AnimText" FontFamily="Consolas" />
        <Button x:Name="OpenButton" Content="Open" Click="OnOpenClicked"/>
        <TextBlock Text="Recent" FontSize="16" Margin="0,20,0,8"/>
        <ListView x:Name="RecentList" IsItemClickEnabled="True" ItemClick="OnRecentItemClick"/>
      </StackPanel>
    </Grid>

    <!-- Main View -->
    <TabView x:Name="TabView"
             Visibility="Collapsed"
             TabCloseRequested="OnTabCloseRequested">
    </TabView>
  </Grid>
</Window>
```

---

## 5. 功能实现
### 5.1 Open（多选）
使用 `FileOpenPicker`，并绑定窗口句柄：
```cpp
FileOpenPicker picker;
picker.ViewMode(PickerViewMode::Thumbnail);
picker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
picker.FileTypeFilter().Append(L".jpg");
picker.FileTypeFilter().Append(L".png");
// ...
InitializeWithWindow(picker, hwnd);

auto files = co_await picker.PickMultipleFilesAsync();
```

> 关键点：Unpackaged WinUI 3 必须用窗口句柄初始化 picker（`InitializeWithWindow`）。

---

### 5.2 标签式预览（TabView）
每张图一个 `TabViewItem`，内部用 `ScrollViewer`：
```xml
<TabViewItem Header="{Binding Title}">
  <ScrollViewer ZoomMode="Enabled"
                MinZoomFactor="0.1"
                MaxZoomFactor="8.0">
    <Image Source="{Binding ImageSource}" Stretch="Uniform"/>
  </ScrollViewer>
</TabViewItem>
```

---

### 5.3 自动缩放 / 放大 / 平移
默认 Fit-to-window：
```cpp
double scale = min(viewportW / imgW, viewportH / imgH);
scrollViewer.ChangeView(nullptr, nullptr, scale);
```

---

### 5.4 Recent 持久化
存储位置：  
`ApplicationData.Current().LocalFolder()/recent.json`

保存：
```cpp
auto folder = ApplicationData::Current().LocalFolder();
auto file = co_await folder.CreateFileAsync(L"recent.json", CreationCollisionOption::OpenIfExists);

JsonArray arr;
for (auto& path : recentList) arr.Append(JsonValue::CreateStringValue(path));
co_await FileIO::WriteTextAsync(file, arr.Stringify());
```

读取：
```cpp
auto text = co_await FileIO::ReadTextAsync(file);
JsonArray arr = JsonArray::Parse(text);
```

---

### 5.5 欢迎页动画（CLI风格）
使用 `DispatcherTimer`：
```cpp
DispatcherTimer timer;
timer.Interval(std::chrono::milliseconds(80));
timer.Tick([this](auto, auto) {
  static std::vector<std::wstring> frames = { L"[=    ]", L"[==   ]", L"[===  ]", L"[==== ]" };
  AnimText().Text(L"loading " + frames[idx++ % frames.size()]);
});
timer.Start();
```

---

## 6. Recent UX 细节
- Recent 点击 = 打开并新建 Tab
- 若文件不存在，加载时过滤
- 最大保存数量建议 10 条

---

## 7. 交互快捷键
- `Ctrl+O`：打开
- `Ctrl+W`：关闭当前标签
- `Ctrl+Tab`：切换 Tab
- 鼠标滚轮：缩放

---

## 8. DPI / 高分辨率
- WinUI 原生支持 DPI 缩放
- XAML 建议 `UseLayoutRounding="True"`

---

## 9. Phase 2（可选：接入现有 C++ 解码器）
1. 抽出 `core/` 为静态库  
2. WinUI 工程链接该库  
3. 输出 BGRA buffer => `SoftwareBitmap`  

---

## 10. 验收标准
- 欢迎页含动画 + Open + Recent
- Recent 持久化有效
- 多 Tab 切换可用
- 图像可缩放/拖拽平移
- UI 视觉现代且高 DPI 友好
