FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG        0.9.9.8 #suggest using a tag so the library doesn't update whenever new commits are pushed to a branch
)
FetchContent_MakeAvailable(glm)