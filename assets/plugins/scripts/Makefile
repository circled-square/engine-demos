
CMAKE_RELEASE_FLAGS=-DCMAKE_BUILD_TYPE=Release -G Ninja -S ./ --log-level=DEBUG


all: 
	make windows
	make linux

linux:
	cmake ${CMAKE_RELEASE_FLAGS} -B build/linux/
	cmake --build build/linux/
	cmake --install build/linux/ --prefix ./
	

windows:
	cmake -DCMAKE_TOOLCHAIN_FILE="../../../cmake/TC-windows.cmake" ${CMAKE_RELEASE_FLAGS} -B build/windows//
	cmake --build build/windows/
	cmake --install build/windows/ --prefix ./

clean:
	rm build/linux/* build/windows/* -fr
