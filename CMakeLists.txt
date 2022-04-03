cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

# ---- Project ----
# Note: update this to your new project's name and version
project(
  qoimage
  VERSION 1.0
  LANGUAGES CXX
)

# ---- Include guards ----

include(${CMAKE_BINARY_DIR}/conan_paths.cmake)
if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set (BUILD_TEST ON)
# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info

if(${BUILD_TESTS})
	find_package(ut REQUIRED)
endif()
find_package(fmt REQUIRED)
# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
set(BUILD_TEST ${BUILD_TESTS})
# ---- Create library ----
# add_executable(qoimage src/impls/tests.cpp src/impls/catch_main.cpp)
add_library(qoimage STATIC src/pixel.cpp src/qoimage.hpp src/qoimage.cpp)
# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(qoimage INTERFACE) set_target_properties(qoimage PROPERTIES
# INTERFACE_COMPILE_FEATURES cxx_std_17)

target_compile_features(qoimage PUBLIC cxx_std_20)
target_compile_options(qoimage PRIVATE -ggdb  -fsanitize=address -fsanitize=undefined -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused )

target_include_directories(
	qoimage PUBLIC $<BUILD_INTERFACE: ${fmt_INCLUDE_DIRS} >
)

target_link_libraries(qoimage PRIVATE -lasan -lubsan fmt::fmt)


if(${BUILD_TEST})
	add_executable(tests src/tests/test.cpp)
	target_compile_features(tests PUBLIC cxx_std_20)
	target_compile_options(tests PRIVATE -ggdb  -fsanitize=address -fsanitize=undefined -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused )

	target_include_directories(
		tests PUBLIC $<BUILD_INTERFACE: ${fmt_INCLUDE_DIRS} ${ut_INCLUDE_DIRS} >
	)

	target_link_libraries(tests PRIVATE -lasan -lubsan fmt::fmt qoimage boost::ut )
else()
	add_executable(qoi src/tests/main.cpp )
	target_compile_features(qoi PUBLIC cxx_std_20)
	target_compile_options(qoi PRIVATE -ggdb  -fsanitize=address -fsanitize=undefined -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef -Wno-unused )

	target_include_directories(
		qoi PUBLIC $<BUILD_INTERFACE: ${fmt_INCLUDE_DIRS}  >
	)

	target_link_libraries(qoi PRIVATE -lasan -lubsan fmt::fmt  qoimage)
endif()

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths

