#include "pch.h"
#include "MainWindow.xaml.h"

#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Input;
using namespace Microsoft::UI::Xaml::Media;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Windows::Storage;
using namespace Windows::Storage::Pickers;
using namespace Windows::Data::Json;
using namespace Windows::Foundation;

namespace winrt::UltraImageViewer::WinUI::implementation
{
    MainWindow::MainWindow()
    {
        InitializeComponent();

        // Get the window
        m_window = XamlRoot().try_as<Window>();
        if (m_window == nullptr)
        {
            // Try to get window through a different method
            m_window = Window();
        }

        // Get window handle for InitializeWithWindow
        auto windowNative = this->m_inner.as<IWindowNative>();
        windowNative->get_WindowHandle(&m_hwnd);

        Title(L"UltraImageViewer");
        InitializeAnimation();
        InitializeKeyboardShortcuts();
        LoadRecentFiles();
    }

    void MainWindow::InitializeAnimation()
    {
        m_animationTimer = DispatcherTimer();
        m_animationTimer.Interval(std::chrono::milliseconds(80));
        m_animationTimer.Tick([this](IInspectable const&, IInspectable const&)
        {
            AnimText().Text(L"loading " + m_animFrames[m_animFrame % m_animFrames.size()]);
            m_animFrame++;

            // Stop animation after one full cycle
            if (m_animFrame > static_cast<int>(m_animFrames.size()))
            {
                m_animationTimer.Stop();
                AnimText().Text(L"");
            }
        });
        m_animationTimer.Start();
    }

    void MainWindow::InitializeKeyboardShortcuts()
    {
        this->KeyUp([this](IInspectable const&, KeyEventArgs const& args)
        {
            auto key = args.Key();

            // Check if Control is pressed using GetKeyState
            auto keyState = GetKeyState(VK_CONTROL);
            bool ctrlPressed = (keyState & 0x8000) != 0;

            // Ctrl+O: Open
            if (key == Windows::System::VirtualKey::O && ctrlPressed)
            {
                OpenFilesAsync();
            }

            // Ctrl+W: Close current tab
            if (key == Windows::System::VirtualKey::W && ctrlPressed)
            {
                if (TabView().SelectedTab() != nullptr)
                {
                    TabView().TabItems().Remove(TabView().SelectedTab());
                    if (TabView().TabItems().Size() == 0)
                    {
                        SwitchToWelcomeView();
                    }
                }
            }

            // Ctrl+Tab: Switch to next tab
            if (key == Windows::System::VirtualKey::Tab && ctrlPressed)
            {
                auto items = TabView().TabItems();
                if (items.Size() > 0)
                {
                    auto selectedIndex = TabView().SelectedIndex();
                    selectedIndex = (selectedIndex + 1) % items.Size();
                    TabView().SelectedIndex(selectedIndex);
                }
            }
        });
    }

    void MainWindow::LoadRecentFiles()
    {
        auto folder = Windows::Storage::ApplicationData::Current().LocalFolder();

        folder.TryGetItemAsync(L"recent.json").Completed(
            [this](IAsyncOperation<IStorageItem> const& op, AsyncStatus status)
        {
            if (status == AsyncStatus::Completed)
            {
                auto item = op.GetResults();
                if (item != nullptr)
                {
                    auto file = item.as<StorageFile>();
                    FileIO::ReadTextAsync(file).Completed(
                        [this](IAsyncOperation<hstring> const& readOp, AsyncStatus readStatus)
                    {
                        if (readStatus == AsyncStatus::Completed)
                        {
                            try
                            {
                                auto jsonText = readOp.GetResults();
                                auto jsonArray = JsonArray::Parse(jsonText);

                                m_recentFiles.clear();
                                for (auto jsonValue : jsonArray)
                                {
                                    m_recentFiles.push_back(jsonValue.GetString());
                                }

                                // Update UI - filter files that still exist
                                RecentList().Items().Clear();
                                for (auto& path : m_recentFiles)
                                {
                                    try
                                    {
                                        if (fs::exists(path))
                                        {
                                            RecentList().Items().Append(box_value(path));
                                        }
                                    }
                                    catch (...)
                                    {
                                        // Skip invalid paths
                                    }
                                }
                            }
                            catch (...)
                            {
                                // Invalid JSON, start with empty list
                            }
                        }
                    });
                }
            }
        });
    }

    fire_and_forget MainWindow::SaveRecentAsync()
    {
        auto folder = Windows::Storage::ApplicationData::Current().LocalFolder();
        auto file = co_await folder.CreateFileAsync(L"recent.json", CreationCollisionOption::ReplaceExisting);

        JsonArray arr;
        for (auto& path : m_recentFiles)
        {
            arr.Append(JsonValue::CreateStringValue(path.c_str()));
        }

        co_await FileIO::WriteTextAsync(file, arr.Stringify());
    }

    void MainWindow::OnOpenClicked(IInspectable const&, RoutedEventArgs const&)
    {
        OpenFilesAsync();
    }

    fire_and_forget MainWindow::OpenFilesAsync()
    {
        auto strong_this = get_strong();  // Keep object alive during coroutine

        FileOpenPicker picker;
        picker.ViewMode(PickerViewMode::Thumbnail);
        picker.SuggestedStartLocation(PickerLocationId::PicturesLibrary);
        picker.FileTypeFilter().Append(L".jpg");
        picker.FileTypeFilter().Append(L".jpeg");
        picker.FileTypeFilter().Append(L".png");
        picker.FileTypeFilter().Append(L".gif");
        picker.FileTypeFilter().Append(L".bmp");
        picker.FileTypeFilter().Append(L".webp");
        picker.FileTypeFilter().Append(L".tiff");

        // Initialize with window handle for unpackaged app
        auto init = InitializeWithWindow::Initialize(picker, m_hwnd);

        auto files = co_await picker.PickMultipleFilesAsync();

        if (files.Size() > 0)
        {
            SwitchToMainView();

            for (auto file : files)
            {
                AddImageTab(file);

                // Add to recent
                auto path = std::wstring(file.Path());
                auto it = std::find(m_recentFiles.begin(), m_recentFiles.end(), path);
                if (it == m_recentFiles.end())
                {
                    m_recentFiles.insert(m_recentFiles.begin(), path);
                    if (m_recentFiles.size() > MAX_RECENT)
                    {
                        m_recentFiles.pop_back();
                    }
                    co_await SaveRecentAsync();
                }
            }
        }
    }

    fire_and_forget MainWindow::OpenFileAsync(StorageFile const& file)
    {
        auto strong_this = get_strong();
        co_await winrt::resume_background();
        SwitchToMainView();
        AddImageTab(file);
    }

    void MainWindow::OnRecentItemClick(IInspectable const{}, ItemClickEventArgs const& args)
    {
        auto path = unbox_value<hstring>(args.ClickedItem());
        StorageFile::GetFileFromPathAsync(path).Completed(
            [this](IAsyncOperation<StorageFile> const& op, AsyncStatus status)
        {
            if (status == AsyncStatus::Completed)
            {
                auto file = op.GetResults();
                SwitchToMainView();
                AddImageTab(file);
            }
        });
    }

    void MainWindow::AddImageTab(StorageFile const& file)
    {
        auto tabViewItem = TabViewItem();
        tabViewItem.Header(box_value(file.Name()));

        // Create scroll viewer with image
        auto scrollViewer = ScrollViewer();
        scrollViewer.ZoomMode(ZoomMode::Enabled);
        scrollViewer.MinZoomFactor(0.1f);
        scrollViewer.MaxZoomFactor(8.0f);
        scrollViewer.HorizontalScrollMode(ScrollMode::Auto);
        scrollViewer.VerticalScrollMode(ScrollMode::Auto);
        scrollViewer.HorizontalScrollBarVisibility(ScrollBarVisibility::Auto);
        scrollViewer.VerticalScrollBarVisibility(ScrollBarVisibility::Auto);

        auto image = Image();
        image.Stretch(Stretch::Uniform);
        image.StretchDirection(StretchDirection::Both);

        // Load image from file
        auto bitmapImage = BitmapImage(Uri(file.Path()));
        image.Source(bitmapImage);

        // Register image opened event for initial fit
        image.ImageOpened([this, scrollViewer, image](IInspectable const&, RoutedEventArgs const&)
        {
            FitImageToViewport(scrollViewer, image);
        });

        scrollViewer.Content(image);
        tabViewItem.Content(scrollViewer);
        TabView().TabItems().Append(tabViewItem);
        TabView().SelectedItem(tabViewItem);
    }

    fire_and_forget MainWindow::FitImageToViewport(ScrollViewer const& sv, Image const& img)
    {
        auto strong_this = get_strong();

        // Wait a bit for layout
        co_await resume_after(std::chrono::milliseconds(50));

        if (img.Source() == nullptr) co_return;

        auto bitmap = img.Source().try_as<BitmapImage>();
        if (bitmap == nullptr) co_return;

        // Get viewport size
        auto viewportWidth = sv.ActualWidth();
        auto viewportHeight = sv.ActualHeight();

        // Get image size
        auto pixelWidth = bitmap.PixelWidth();
        auto pixelHeight = bitmap.PixelHeight();

        if (pixelWidth == 0 || pixelHeight == 0) co_return;

        // Calculate scale to fit
        double scaleX = static_cast<double>(viewportWidth) / pixelWidth;
        double scaleY = static_cast<double>(viewportHeight) / pixelHeight;
        double scale = std::min(scaleX, scaleY);

        // Ensure we don't zoom in more than 100% for initial fit
        if (scale > 1.0) scale = 1.0;

        sv.ChangeView(nullptr, nullptr, static_cast<float>(scale));
    }

    void MainWindow::OnTabCloseRequested(IInspectable const{}, TabViewTabCloseRequestedEventArgs const& args)
    {
        auto tab = args.Tab();
        TabView().TabItems().Remove(tab);

        if (TabView().TabItems().Size() == 0)
        {
            SwitchToWelcomeView();
        }
    }

    void MainWindow::SwitchToMainView()
    {
        WelcomeRoot().Visibility(Visibility::Collapsed);
        TabView().Visibility(Visibility::Visible);
    }

    void MainWindow::SwitchToWelcomeView()
    {
        TabView().Visibility(Visibility::Collapsed);
        WelcomeRoot().Visibility(Visibility::Visible);
        LoadRecentFiles(); // Refresh recent list
    }
}
