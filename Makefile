# a simple makefile for running cmake, compiling and running the project
# without using an IDE or having to remember all the flags

all: build/debug/src/main
build/debug/src/main: configure_debug compile_debug
build/release/src/main: configure_release compile_release

# add -fsanitize=address to check memory errors
CXXFLAGS="-Wall -Werror -Wpedantic -Wnon-virtual-dtor"
GENERATOR="Ninja" # set to "Unix Makefiles" if ninja is unavailable

CMAKE_CLANG_TIDY_FLAG=-DCMAKE_CXX_CLANG_TIDY="clang-tidy;-checks=cppcoreguidelines-\*,readability-\*"
CMAKE_FLAGS=-G ${GENERATOR} -S ./
CMAKE_DEBUG_FLAGS=-DCMAKE_BUILD_TYPE=Debug ${CMAKE_FLAGS} -B build/debug
CMAKE_RELEASE_FLAGS=-DCMAKE_BUILD_TYPE=Release ${CMAKE_FLAGS} -B build/release

configure_debug: src/* src/**
	cmake ${CMAKE_DEBUG_FLAGS}

configure_release: src/* src/**
	cmake ${CMAKE_RELEASE_FLAGS}

compile_debug: src/* src/**/*
	ninja -C build/debug/

compile_release: src/* src/**/*
	ninja -C build/release/

compile_with_clang_tidy: src/* src/**/*
	cmake ${CMAKE_CLANG_TIDY_FLAG} ${CMAKE_DEBUG_FLAGS} -B build/debug_clangtidy
	make -C build/debug_clangtidy


run: build/debug/src/main
	build/debug/src/main

run_release: build/release/src/main
	build/release/src/main

clean_deps:
	rm -fr build/debug/_deps

clean:
	# instead of doing rm build/* -fr we do this to avoid removing dependencies
	rm -f build/debug/CMakeCache.txt
	rm -f build/debug/build.ninja
	rm -f build/debug/cmake_install.cmake
	rm -fr build/debug/deps/
	rm -fr build/debug/CMakeFiles/
	rm -fr build/debug/src/
	rm -f build/release/CMakeCache.txt
	rm -f build/release/build.ninja
	rm -f build/release/cmake_install.cmake
	rm -fr build/release/deps/
	rm -fr build/release/CMakeFiles/
	rm -fr build/release/src/
	rm -fr build/debug_clangtidy/

	rm imgui.ini -f
