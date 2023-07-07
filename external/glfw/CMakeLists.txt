FetchContent_Declare(
        glfw3
        # URL https://github.com/glfw/glfw/releases/download/3.3.8/glfw-3.3.8.zip
        # URL_HASH SHA256=4d025083cc4a3dd1f91ab9b9ba4f5807193823e565a5bcf4be202669d9911ea6
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3-stable
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable( glfw3 )