# a simple makefile for running cmake, compiling and running the project
# without using an IDE or having to remember all the flags. it's more like a
# set of aliases actually, since i do not actually use Make's features (which
# would be useless since cmake already checks whether what you're asking to build is already built)


# add -fsanitize=address to check memory errors
CXXFLAGS="-Wall -Werror -Wpedantic -Wnon-virtual-dtor"
GENERATOR="Ninja" # set to "Unix Makefiles" if ninja is unavailable

CMAKE_FLAGS=-DCMAKE_INSTALL_MESSAGE=LAZY -G ${GENERATOR} -S ./

# flags for configuration of the various targets
CMAKE_debug_FLAGS=-DCMAKE_BUILD_TYPE=Debug ${CMAKE_FLAGS} --log-level=DEBUG
CMAKE_release_FLAGS=-DCMAKE_BUILD_TYPE=Release ${CMAKE_FLAGS} --log-level=TRACE
CMAKE_debug_windows_FLAGS=-DCMAKE_TOOLCHAIN_FILE=./cmake/TC-windows.cmake ${CMAKE_debug_FLAGS}
CMAKE_release_windows_FLAGS=-DCMAKE_TOOLCHAIN_FILE=./cmake/TC-windows.cmake ${CMAKE_release_FLAGS}
CMAKE_clangtidy_FLAGS=-DCMAKE_CXX_CLANG_TIDY="clang-tidy;-checks=cppcoreguidelines-\*,readability-\*" ${CMAKE_debug_FLAGS}

#build everything;
all: build_debug build_release build_debug_windows build_release_windows build_clangtidy

build_%:
	#configure $*
	cmake ${CMAKE_$*_FLAGS} -B build/$*
	#build $*
	cmake --build build/$*
	#install $*
	cmake --install build/$* --prefix install_dir/$*/

run_%: 
	make build_$*
	install_dir/$*/main*

run_tests: build_debug build_release
	CTEST_OUTPUT_ON_FAILURE=1 ninja -C build/debug run_engine_tests
	CTEST_OUTPUT_ON_FAILURE=1 ninja -C build/release run_engine_tests

clean:
	rm -rf build/* install_dir/*
	rm imgui.ini -f
