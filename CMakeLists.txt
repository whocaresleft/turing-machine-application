cmake_minimum_required(VERSION 3.31)
project(mdt)

set(CMAKE_CXX_STANDARD 17)

# OpenGL3
find_package(OpenGL REQUIRED)

# GLFW
add_subdirectory(third_party/glfw-3.4)

# ImGui
add_compile_definitions(IMGUI_DEFINE_MATH_OPERATORS)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)
set(IMGUI_SOURCES
        ${IMGUI_DIR}/imgui.cpp
        ${IMGUI_DIR}/imgui_draw.cpp
        ${IMGUI_DIR}/imgui_widgets.cpp
        ${IMGUI_DIR}/imgui_tables.cpp
        ${IMGUI_DIR}/imgui_demo.cpp
        ${IMGUI_DIR}/imgui_impl_glfw.cpp
        ${IMGUI_DIR}/imgui_impl_opengl3.cpp
        header/gui_app_helpers.h
)
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
        ${IMGUI_DIR}
)
target_link_libraries(imgui PRIVATE glfw OpenGL::GL)

add_library(imnodes STATIC third_party/imnodes/imnodes.cpp)
target_include_directories(imnodes PUBLIC third_party/imnodes)

add_library(imguifiledialogs STATIC third_party/imguifiledialog/ImGuiFileDialog.cpp)
target_include_directories(imguifiledialogs PUBLIC
        third_party/imguifiledialog
        third_party/imguifiledialog/dirent
        third_party/imguifiledialog/stb
)

add_executable(turing_machine_creator source/gui_app.cpp
        header/definitions.h
        header/alphabet.h
        header/tape.h
        header/helper.h
        header/gui_app_helpers.h
)

add_executable(binary_generator source/generate_binary.cpp)

# JSON, ImGui and GLFW
target_include_directories(turing_machine_creator PRIVATE third_party)

target_link_libraries(turing_machine_creator PRIVATE imgui imnodes imguifiledialogs glfw OpenGL::GL)