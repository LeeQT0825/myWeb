cmake_minimum_required(VERSION 3.0)     # 可用版本号 必写
project(myProject)                      # 项目名称 必写

include_directories(.)
include_directories(/usr/local/lib)

link_directories(/usr/local/lib)

set(CMAKE_VERBOSE_MAKEFILE ON)          # 显示原始编译信息，用于定位一些连接错误
# CMAKE_CXX_FLAGS———g++编译选项     
# -O3是release版本；-O0是debug版本  O英文字母）
# -g是启用GDB调试
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -g -std=c++11 -Wall -Wno-depracated -Werror -Wno-unused-function")

set(LIB_SCR code/log.cc 
            code/util.cc
            code/config.cc
            code/mthread.cc
            code/mmutex.cc
            code/fiber.cc
            code/scheduler.cc
            code/iomanager.cc
            code/timer.cc
            code/hook.cc
            code/fdmanager.cc
            code/address.cc
            code/socket.cc
            code/bytearray.cc
            code/tcp_server.cc
            code/stream.cc
            code/http/http.cc
            code/http/http_parser.cc
            code/http/httpserver_parser.cc
            code/http/httpclient_parser.cc
            code/http/http_session.cc
            code/http/http_server.cc
            code/http/servlet.cc
            code/streams/socket_stream.cc)

# ragelmaker(code/http/httpserver_parser.rl LIB_SCR ${CMAKE_CURRENT_SOURCE_DIR}/code/http)
# ragelmaker(code/http/httpclient_parser.rl LIB_SCR ${CMAKE_CURRENT_SOURCE_DIR}/code/http)

add_library(code SHARED ${LIB_SCR})     # 通过变量LIB-SCR生成库名为libcode.so的动态库
set(LIBLIB  code
            pthread
            yaml-cpp
            dl)

# 测试log的可执行文件
add_executable(test tests/test.cc)      # 生成名为test的可执行文件
add_dependencies(test code)
target_link_libraries(test ${LIBLIB})        # 为test可执行文件连接动态库（相当于g++的-l参数）

# 测试config的可执行文件
add_executable(test_config tests/test_config.cc)      
add_dependencies(test_config code)
target_link_libraries(test_config ${LIBLIB})        # /usr/local/lib/ 中准标准库也要手动添加（可执行文件）
 
# 测试thread的可执行文件
add_executable(test_thread tests/test_thread.cc)      
add_dependencies(test_thread code)
target_link_libraries(test_thread ${LIBLIB})

# 测试assert的可执行文件
add_executable(test_assert tests/test_assert.cc)      
add_dependencies(test_assert code)
target_link_libraries(test_assert ${LIBLIB})

# 测试fiber的可执行文件
add_executable(test_fiber tests/test_fiber.cc)      
add_dependencies(test_fiber code)
target_link_libraries(test_fiber ${LIBLIB})

# 测试Scheduler的可执行文件
add_executable(test_scheduler tests/test_scheduler.cc)      
add_dependencies(test_scheduler code)
target_link_libraries(test_scheduler ${LIBLIB})

# 测试IOManager的可执行文件
add_executable(test_iomanager tests/test_iomanager.cc)      
add_dependencies(test_iomanager code)
target_link_libraries(test_iomanager ${LIBLIB})

# 测试Socket API的可执行文件
add_executable(test_socket tests/test_socket.cc)      
add_dependencies(test_socket code)
target_link_libraries(test_socket ${LIBLIB})

# 测试序列化的可执行文件
add_executable(test_bytearray tests/test_bytearray.cc)      
add_dependencies(test_bytearray code)
target_link_libraries(test_bytearray ${LIBLIB})

# 测试Http相关的可执行文件
add_executable(test_http tests/test_http.cc)      
add_dependencies(test_http code)
target_link_libraries(test_http ${LIBLIB})

# 测试TCP_Server相关的可执行文件
add_executable(test_tcp_server tests/test_tcp_server.cc)      
add_dependencies(test_tcp_server code)
target_link_libraries(test_tcp_server ${LIBLIB})

# 测试Echo_Server相关的可执行文件
add_executable(echo_server examples/echo_server.cc)      
add_dependencies(echo_server code)
target_link_libraries(echo_server ${LIBLIB})
add_executable(echo_client examples/echo_client.cc)      
add_dependencies(echo_client code)
target_link_libraries(echo_client ${LIBLIB})

# 测试Echo_Server相关的可执行文件
add_executable(echo_server_2 examples/echo_server_2.cc)      
add_dependencies(echo_server_2 code)
target_link_libraries(echo_server_2 ${LIBLIB})

# 测试Servlet相关的可执行文件
add_executable(test_http_server tests/test_http_server.cc)      
add_dependencies(test_http_server code)
target_link_libraries(test_http_server ${LIBLIB})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)       # EXECUTABLE_OUTPUT_PATH————可执行文件输出的存放路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)          # LIBRARY_OUTPUT_PATH————库文件输出的存放路径