add_executable(basic main.cpp)
add_dependencies(basic Shaders)
configure_file(monkey.obj ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/monkey.obj COPYONLY)
configure_file(monkey.mtl ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/monkey.mtl COPYONLY)

target_link_libraries(basic vk_engine)