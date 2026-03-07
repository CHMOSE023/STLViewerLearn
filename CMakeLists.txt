cmake_minimum_required(VERSION 3.10) 

project(STLViewerLearn)

# 设置 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -------------------------------
# 设置输出目录
# -------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)   # .exe 和 .dll
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)   # Linux .so
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)   # .lib 导入库

# Debug / Release 子目录也统一
foreach(CONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach()
 
# -------------------------------
# Windows 动态库自动导出符号，生成配套 .lib 导入库
# -------------------------------
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# -------------------------------
# 设置 include 目录
# -------------------------------
include_directories(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GeomCalc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/GeomKernel"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/glContext"
)

# -------------------------------
# 收集源文件
# -------------------------------
file(GLOB GEOMCALC_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/GeomCalc/*.cpp")
file(GLOB GEOMKERNEL_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/GeomKernel/*.cpp")
file(GLOB GLCONTEXT_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/glContext/*.cpp")
file(GLOB STLVIEWER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/STLViewer/*.cpp")

# -------------------------------
# 构建动态库
# -------------------------------
add_library(GeomCalc SHARED ${GEOMCALC_SOURCES})
add_library(GeomKernel SHARED ${GEOMKERNEL_SOURCES})
add_library(glContext SHARED ${GLCONTEXT_SOURCES})

# -------------------------------
# 构建可执行文件
# -------------------------------
add_executable(STLViewer ${STLVIEWER_SOURCES})

# -------------------------------
# 链接库
# -------------------------------
target_link_libraries(STLViewer GeomCalc GeomKernel glContext)
 