cmake_minimum_required(VERSION 3.20)

project(NCUTAutoLogin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(NCUTAutoLogin WIN32
    src/main.cpp
    src/AppWindow.cpp
    src/Monitor.cpp
    src/Network.cpp
    src/Logger.cpp
    src/Config.cpp
    src/I18N.cpp
)

target_include_directories(NCUTAutoLogin PRIVATE src)

if (MSVC)
    target_compile_definitions(NCUTAutoLogin PRIVATE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_compile_options(NCUTAutoLogin PRIVATE /W4 /permissive-)
else()
    target_compile_options(NCUTAutoLogin PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(NCUTAutoLogin PRIVATE
    ws2_32
    winhttp
    comctl32
    shlwapi
    shell32
    advapi32
)

if (MINGW)
    target_link_options(NCUTAutoLogin PRIVATE -municode)
endif()

add_executable(NCUTAutoLoginSvc
    src/ServiceMain.cpp
    src/Monitor.cpp
    src/Network.cpp
    src/Logger.cpp
    src/Config.cpp
)

target_include_directories(NCUTAutoLoginSvc PRIVATE src)
target_link_libraries(NCUTAutoLoginSvc PRIVATE
    ws2_32
    winhttp
    advapi32
)
if (MINGW)
    target_link_options(NCUTAutoLoginSvc PRIVATE -municode)
endif()

install(TARGETS NCUTAutoLoginSvc RUNTIME DESTINATION bin)

install(TARGETS NCUTAutoLogin RUNTIME DESTINATION bin)
