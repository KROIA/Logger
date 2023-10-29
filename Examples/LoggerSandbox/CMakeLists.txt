
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR FILENAME PROJECT_NAME)   
string(REPLACE " " "_" ProjectId ${PROJECT_NAME})
project(${PROJECT_NAME})


if(LOGGER_PROFILING)
    set(PROJECT_NAME ${PROJECT_NAME}_profile)
endif()

# Macro to search for files with given file ending.
# call:
#   FILE_DIRECTORIES(H_FILES *.h)
#
# all *.h files will be saved in the H_FILES variable
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



include_directories(inc)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Get all source files
FILE_DIRECTORIES(H_FILES *.h)
FILE_DIRECTORIES(CPP_FILES *.cpp)
FILE_DIRECTORIES(UI_FILES *.ui)

if(UI_FILES)
	include(${QT_LOCATOR_CMAKE})
	find_package(Qt5Widgets REQUIRED)
	qt5_wrap_ui(UIS_HDRS ${UI_FILES})
	qt_wrap_cpp(SRC_HDRS ${H_FILES})
endif()

set(Files 
	${H_FILES}
	${CPP_FILES}
	${UIS_HDRS}
	${SRC_HDRS})

add_executable(${PROJECT_NAME} ${Files})



if(LOGGER_PROFILING)
    target_link_libraries(${PROJECT_NAME} Logger_static_profile)
else()
    target_link_libraries(${PROJECT_NAME} Logger_static)
endif()
target_compile_definitions(${PROJECT_NAME} PUBLIC BUILD_STATIC)


install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
