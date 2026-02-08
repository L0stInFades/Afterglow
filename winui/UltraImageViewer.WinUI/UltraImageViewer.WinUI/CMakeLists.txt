cmake_minimum_required(VERSION 3.20)

project(UltraImageViewer.WinUI
    VERSION 1.0.0
    DESCRIPTION "UltraImageViewer WinUI 3 Application"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINRT_EXECUTABLE ON)

# Windows App SDK path - may need adjustment
set(WASDK_PATH "C:\\Program Files\\Microsoft SDKs\\Windows App SDK" CACHE PATH "Windows App SDK path")

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Source files
set(XAML_FILES
    App.xaml
    MainWindow.xaml
)

set(SOURCE_FILES
    App.xaml.cpp
    MainWindow.xaml.cpp
    pch.cpp
)

set(HEADER_FILES
    App.xaml.h
    MainWindow.xaml.h
    pch.h
)

# Create executable
add_executable(UltraImageViewer.WinUI WIN32
    ${XAML_FILES}
    ${SOURCE_FILES}
    ${HEADER_FILES}
    app.manifest
)

# Precompiled header
target_precompile_headers(UltraImageViewer.WinUI
    PUBLIC
        pch.h
)

# Compiler flags
target_compile_options(UltraImageViewer.WinUI PRIVATE
    /await
    /bigobj
    /ZW
    /EHsc
)

# Linker settings
set_target_properties(UltraImageViewer.WinUI PROPERTIES
    VS_GLOBAL_UseWinUI TRUE
)

# Link libraries
target_link_libraries(UltraImageViewer.WinUI PRIVATE
    windowsapp
)

# Windows App SDK NuGet packages would need to be restored first
# For now, this provides a basic CMake structure

# Output directory
set_target_properties(UltraImageViewer.WinUI PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
