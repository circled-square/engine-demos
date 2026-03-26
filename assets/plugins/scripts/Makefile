
CMAKE_RELEASE_FLAGS=-DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_MESSAGE=LAZY -G Ninja -S ./ --log-level=DEBUG
CMAKE_DEBUG_FLAGS=-DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_MESSAGE=LAZY -G Ninja -S ./ --log-level=DEBUG


all: 
	make linux
	make windows
	make linux_debug

linux:
	cmake ${CMAKE_RELEASE_FLAGS} -B build/linux/
	cmake --build build/linux/
	cmake --install build/linux/ --prefix ./
	
linux_debug:
	cmake ${CMAKE_DEBUG_FLAGS} -B build/linux_debug/
	cmake --build build/linux_debug/
	cmake --install build/linux_debug/ --prefix ./
	
windows:
	cmake -DCMAKE_TOOLCHAIN_FILE="../../../cmake/TC-windows.cmake" ${CMAKE_RELEASE_FLAGS} -B build/windows/
	cmake --build build/windows/
	cmake --install build/windows/ --prefix ./

clean:
	rm build/linux/* build/windows/* build/linux_debug/* -fr
	
