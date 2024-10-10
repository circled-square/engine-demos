# a simple makefile for running cmake, compiling and running the project
# without using an IDE or having to remember all the flags. it's more like a
# script actually, since i do not actually use Make's features


# add -fsanitize=address to check memory errors
CXXFLAGS="-Wall -Werror -Wpedantic -Wnon-virtual-dtor"
GENERATOR="Ninja" # set to "Unix Makefiles" if ninja is unavailable

CMAKE_CLANG_TIDY_FLAG=-DCMAKE_CXX_CLANG_TIDY="clang-tidy;-checks=cppcoreguidelines-\*,readability-\*"
CMAKE_FLAGS=-G ${GENERATOR} -S ./
CMAKE_DEBUG_FLAGS=-DCMAKE_BUILD_TYPE=Debug ${CMAKE_FLAGS} -B build/debug
CMAKE_RELEASE_FLAGS=-DCMAKE_BUILD_TYPE=Release ${CMAKE_FLAGS} -B build/release

all:
	make configure_debug
	make compile_debug

configure_debug:
	cmake ${CMAKE_DEBUG_FLAGS}

configure_release:
	cmake ${CMAKE_RELEASE_FLAGS}

compile_debug:
	ninja -C build/debug/

compile_release:
	ninja -C build/release/

compile_with_clang_tidy: src/* src/**/*
	cmake ${CMAKE_CLANG_TIDY_FLAG} ${CMAKE_DEBUG_FLAGS} -B build/debug_clangtidy
	ninja -C build/debug_clangtidy


run:
	make all
	build/debug/src/main

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