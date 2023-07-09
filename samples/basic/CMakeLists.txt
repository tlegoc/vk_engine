add_executable(basic main.cpp)
add_dependencies(basic Shaders)

target_link_libraries(basic vk_engine)