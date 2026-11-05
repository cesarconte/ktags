cmake_minimum_required(VERSION 3.22)
project(ktags VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Xml Test)

set(SOURCES
    src/main.cpp
    src/tagengine.h
    src/tagengine.cpp
)

add_executable(ktag ${SOURCES})

target_link_libraries(ktag PRIVATE
    Qt6::Core
    Qt6::Xml
)

target_include_directories(ktag PRIVATE src)

install(TARGETS ktag DESTINATION bin)
install(FILES data/kio/servicemenus/ktags.desktop DESTINATION share/kio/servicemenus)

enable_testing()
add_subdirectory(tests)
