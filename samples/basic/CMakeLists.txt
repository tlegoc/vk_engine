add_executable(basic main.cpp)
add_dependencies(basic Shaders)
configure_file(monkey.obj ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets/monkey.obj COPYONLY)

target_link_libraries(basic vk_engine)