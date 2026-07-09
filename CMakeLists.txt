cmake_minimum_required(VERSION 3.20)

project(problem_solution_generator VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} QUIET COMPONENTS Test)

add_executable(problem_solution_generator
    src/appsettings.cpp
    src/appsettings.h
    src/htmlrenderer.cpp
    src/htmlrenderer.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/openai_client.cpp
    src/openai_client.h
    src/promptbuilder.cpp
    src/promptbuilder.h
    src/settingsdialog.cpp
    src/settingsdialog.h
    src/solutiondraft.cpp
    src/solutiondraft.h
)

target_link_libraries(problem_solution_generator PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
)

if (MSVC)
    target_compile_options(problem_solution_generator PRIVATE /W4 /permissive- /utf-8)
else()
    target_compile_options(problem_solution_generator PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

if (TARGET Qt${QT_VERSION_MAJOR}::Test)
    add_executable(problem_solution_generator_tests
        tests/tst_render_and_parse.cpp
        src/htmlrenderer.cpp
        src/responseparser.cpp
        src/solutiondraft.cpp
    )

    target_link_libraries(problem_solution_generator_tests PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Test
    )

    if (MSVC)
        target_compile_options(problem_solution_generator_tests PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(problem_solution_generator_tests PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    add_test(NAME problem_solution_generator_tests COMMAND problem_solution_generator_tests)
endif()
