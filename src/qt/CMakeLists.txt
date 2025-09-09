cmake_minimum_required(VERSION 3.20)

project(NCUTAutoLoginQt LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 COMPONENTS Widgets Network REQUIRED QUIET)
if (NOT Qt6_FOUND)
  find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
  set(QT_PREFIX Qt5)
else()
  set(QT_PREFIX Qt6)
endif()

set(SRC
  src/main.cpp
  src/MainWindow.cpp
  src/Monitor.cpp
  src/Network.cpp
  src/Logger.cpp
  src/Config.cpp
)

if (WIN32)
  list(APPEND SRC src/CryptoWin.cpp)
endif()

add_executable(NCUTAutoLoginQt WIN32 ${SRC})

target_include_directories(NCUTAutoLoginQt PRIVATE src)

target_link_libraries(NCUTAutoLoginQt PRIVATE
  ${QT_PREFIX}::Widgets
  ${QT_PREFIX}::Network
)

if (WIN32)
  target_link_libraries(NCUTAutoLoginQt PRIVATE crypt32)
endif()

install(TARGETS NCUTAutoLoginQt RUNTIME DESTINATION bin)

