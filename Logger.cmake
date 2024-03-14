## description: simple library to create log files and UI based logging systems
cmake_minimum_required(VERSION 3.1.0)
include(FetchContent)

# Define the git repository and tag to download from
set(LIB_NAME Logger)
set(GIT_REPO https://github.com/KROIA/Logger.git)
set(GIT_TAG main)

FetchContent_Declare(
    ${LIB_NAME}
    GIT_REPOSITORY ${GIT_REPO}
    GIT_TAG        ${GIT_TAG}
)

set(Logger_NO_SAMPLES True)
set(Logger_NO_UNITTESTS True)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Logger as static library.")
message("Downloading dependency: ${LIB_NAME} from: ${GIT_REPO} tag: ${GIT_TAG}")
FetchContent_MakeAvailable(${LIB_NAME})

# Add this library to the specific profiles of this project
list(APPEND DEPENDENCIES_FOR_SHARED_LIB Logger_static)
list(APPEND DEPENDENCIES_FOR_STATIC_LIB Logger_static)
list(APPEND DEPENDENCIES_FOR_STATIC_PROFILE_LIB Logger_static_profile) # only use for static profiling profile
