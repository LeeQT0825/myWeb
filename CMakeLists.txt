cmake_minimum_required(VERSION 3.0)     # 可用版本号 必写
project(myProject)                      # 项目名称 必写

set(CMAKE_VERBOSE_MAKEFILE ON)          # 显示原始编译信息，用于定位一些连接错误
# CMAKE_CXX_FLAGS———g++编译选项     
# -O3是release版本；-O0是debug版本  O英文字母）
# -g是启用GDB调试
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -g -std=c++11 -Wall -Wno-depracated -Werror -Wno-unused-function")

set(LIB_SCR code/log.cc code/util.cc)

add_library(code SHARED ${LIB_SCR})     # 通过变量LIB-SCR生成库名为libcode.so的动态库

add_executable(test tests/test.cc)      # 生成名为test的可执行文件
add_dependencies(test code)
target_link_libraries(test code)        # 为test可执行文件连接动态库（相当于g++的-l参数）

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)       # EXECUTABLE_OUTPUT_PATH————可执行文件输出的存放路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)          # LIBRARY_OUTPUT_PATH————库文件输出的存放路径