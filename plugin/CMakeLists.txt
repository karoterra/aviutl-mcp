cmake_minimum_required(VERSION 3.31)
project(aviutl_mcp_plugin
    LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json CONFIG REQUIRED)

add_library(mcp SHARED
    src/plugin.cpp
    src/mcp_plugin.h
    src/mcp_plugin.cpp
    src/thread_manager.h
    src/thread_safe_queue.h
    src/pipe_server.h
    src/pipe_server.cpp
    src/pipe_command.h
    src/log.h
    src/log.cpp
    src/string_helper.h
    src/exedit_handler.h
    src/exedit_handler.cpp
)
target_include_directories(mcp PRIVATE
    vendor/aviutl_exedit_sdk
)
target_link_libraries(mcp
    nlohmann_json::nlohmann_json
)
target_compile_options(mcp
    PRIVATE
        "$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>"
        "$<$<CXX_COMPILER_ID:MSVC>:/execution-charset:shift_jis>"
)
set_target_properties(mcp PROPERTIES SUFFIX .auf)
