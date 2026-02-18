add_subdirectory(engine/)

if(${WIN32})
    add_library(win_runtime_libs INTERFACE)

    set(WIN_RUNTIME_LIBS win_runtime_libs/libgcc_s_seh-1.dll win_runtime_libs/libstdc++-6.dll win_runtime_libs/libwinpthread-1.dll) # for some reason i need to make a separate variable for this, otherwise it does not work
    set_target_properties(win_runtime_libs PROPERTIES
        LINKER_LANGUAGE CXX
        RESOURCE "${WIN_RUNTIME_LIBS}"
    )

    install(TARGETS win_runtime_libs RESOURCE DESTINATION .)
    target_link_libraries(engine PUBLIC win_runtime_libs)
endif()



