cmake_minimum_required(VERSION 3.1.0)
# Neural net core 
set(Logger_filename "Logger")
set(Logger_Include "inc")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()



MACRO(FILE_DIRECTORIES return_list ending)
    FILE(GLOB_RECURSE new_list ${ending})
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${file_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()


# QT settings
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

include(../QtLocator.cmake)
message("Search for QT5 finished")
find_package(Qt5Widgets REQUIRED)
message("find_package for Qt5Widgets finished")

if (Qt5Widgets_FOUND)
    message(STATUS "Using Qt v ${Qt5Widgets_VERSION}")
else()
    message(STATUS "INFO\n\n\tQt5 not found!\n")
endif()


# end QT settings


# External dependencies
include(FetchContent)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/dependencies")

#  EasyProfiler
set(EASY_GIT_REPO  https://github.com/yse/easy_profiler.git)
set(EASY_GIT_TAG   v2.1.0)
set(BUILD_SHARED_LIBS OFF)
set(EASY_PROFILER_NO_SAMPLES True)
FetchContent_Declare(
 EASY_PROFILER
 GIT_REPOSITORY ${EASY_GIT_REPO}
 GIT_TAG ${EASY_GIT_TAG}
)
message("Download dependency: ${EASY_GIT_REPO}")

FetchContent_MakeAvailable(EASY_PROFILER)
#set(EASY_PROFILER_NO_GUI False)
set_target_properties(easy_profiler PROPERTIES CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set_target_properties(easy_profiler PROPERTIES CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set_target_properties(easy_profiler PROPERTIES CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/${RELATIVE_BUILD_FOLDER}")
set_target_properties(easy_profiler PROPERTIES DEBUG_POSTFIX "-d")

# End external dependencies

include_directories(inc)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

FILE_DIRECTORIES(H_FILES *.h)
FILE_DIRECTORIES(CPP_FILES *.cpp)
FILE_DIRECTORIES(UI_FILES *.ui)

qt5_wrap_cpp(CPP_MOC_FILES ${H_FILES})
qt5_wrap_ui(UIS_HDRS ${UI_FILES})


set(SOURCES
	${CPP_MOC_FILES}
    ${CPP_FILES}
    ${UIS_HDRS}
    ${H_FILES}
)

message("CMAKE_CURRENT_BINARY_DIR = " ${CMAKE_CURRENT_BINARY_DIR})

message("Create library with files:")
 FOREACH(file ${SOURCES})
    message("  File: " ${file})
 ENDFOREACH()



 set(Logger_dependencies 
    Qt5::Widgets)

add_library(Logger_shared SHARED ${SOURCES})
add_library(Logger_static STATIC ${SOURCES})
add_library(Logger_static_profile STATIC ${SOURCES})
 
target_compile_definitions(Logger_shared PRIVATE  LOGGER_LIB)
target_compile_definitions(Logger_static PRIVATE  BUILD_STATIC LOGGER_LIB)
target_compile_definitions(Logger_static_profile PRIVATE  BUILD_STATIC LOGGER_PROFILING LOGGER_LIB)

set_target_properties(easy_profiler PROPERTIES DEBUG_POSTFIX "-d")
set_target_properties(Logger_shared PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${Logger_filename}" DEBUG_POSTFIX "-d")
set_target_properties(Logger_static PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${Logger_filename}-s" DEBUG_POSTFIX "-d")
set_target_properties(Logger_static_profile PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${Logger_filename}-s-p" DEBUG_POSTFIX "-d")


target_link_libraries(Logger_shared ${Logger_dependencies})
target_link_libraries(Logger_static ${Logger_dependencies})
target_link_libraries(Logger_static_profile ${Logger_dependencies} easy_profiler)


target_include_directories(Logger_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${Logger_Include}>
    ${CMAKE_CURRENT_BINARY_DIR}
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
target_include_directories(Logger_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${Logger_Include}>
    ${CMAKE_CURRENT_BINARY_DIR}
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
target_include_directories(Logger_static_profile PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${Logger_Include}>
    ${CMAKE_CURRENT_BINARY_DIR}
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
# Define the install path
message("Installation to path: ${CMAKE_INSTALL_PREFIX}/bin")
message("Installation to path: ${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_FOLDER}")
install(TARGETS Logger_shared DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(TARGETS Logger_shared DESTINATION "${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_FOLDER}")
install(TARGETS Logger_static DESTINATION "${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_FOLDER}")
install(TARGETS Logger_static_profile DESTINATION "${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_FOLDER}")
install(TARGETS easy_profiler DESTINATION "${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_FOLDER}")
