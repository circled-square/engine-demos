# a simple makefile for running cmake, compiling and running the project
# without using an IDE or having to remember all the flags. it's more like a
# script actually, since i do not actually use Make's features


# add -fsanitize=address to check memory errors
CXXFLAGS="-Wall -Werror -Wpedantic -Wnon-virtual-dtor"
GENERATOR="Ninja" # set to "Unix Makefiles" if ninja is unavailable

CMAKE_CLANG_TIDY_FLAG=-DCMAKE_CXX_CLANG_TIDY="clang-tidy;-checks=cppcoreguidelines-\*,readability-\*"
CMAKE_FLAGS=-G ${GENERATOR} -S ./
CMAKE_DEBUG_FLAGS=-DCMAKE_BUILD_TYPE=Debug ${CMAKE_FLAGS} --log-level=DEBUG
CMAKE_RELEASE_FLAGS=-DCMAKE_BUILD_TYPE=Release ${CMAKE_FLAGS} --log-level=TRACE

all:
	make configure_debug
	make compile_debug


configure_debug:
	cmake ${CMAKE_DEBUG_FLAGS} -B build/debug

compile_debug:
	cmake --build build/debug/


configure_release:
	cmake ${CMAKE_RELEASE_FLAGS} -B build/release

compile_release:
	cmake --build build/release/


configure_windows:
	cmake -DCMAKE_TOOLCHAIN_FILE=./cmake/TC-windows.cmake ${CMAKE_RELEASE_FLAGS} -B build/release_windows

compile_windows:
	cmake --build build/release_windows/ #-DCMAKE_TOOLCHAIN_FILE=./cmake/TC-windows.cmake


configure_debug_clangtidy:
	cmake ${CMAKE_CLANG_TIDY_FLAG} ${CMAKE_DEBUG_FLAGS} -B build/debug_clangtidy

compile_debug_clangtidy:
	cmake --build build/debug_clangtidy


run:
	make compile_debug
	build/debug/src/main

run_tests:
	CTEST_OUTPUT_ON_FAILURE=1 ninja -C build/debug run_engine_tests
	CTEST_OUTPUT_ON_FAILURE=1 ninja -C build/release run_engine_tests


clean:
	# instead of doing rm build/* -fr we do this to avoid removing dependencies
	rm -rf build/

	rm imgui.ini -f
