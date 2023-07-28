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


FILE_DIRECTORIES(H_FILES *.h)
FILE_DIRECTORIES(CPP_FILES *.cpp)



set(SOURCES
    ${CPP_FILES}
    ${H_FILES}
)



message("Create library with files:")
 FOREACH(file ${SOURCES})
    message("  File: " ${file})
 ENDFOREACH()

add_library(Logger_shared SHARED ${SOURCES})
add_library(Logger_static STATIC ${SOURCES})
 
target_compile_definitions(Logger_shared PRIVATE  LOGGER_LIB)
target_compile_definitions(Logger_static PRIVATE  BUILD_STATIC LOGGER_LIB)

set_target_properties(Logger_shared PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${Logger_filename}" DEBUG_POSTFIX "-d")
set_target_properties(Logger_static PROPERTIES LINKER_LANGUAGE CXX  OUTPUT_NAME "${Logger_filename}-s" DEBUG_POSTFIX "-d")


target_link_libraries(Logger_shared ${Logger_dependencies})
target_link_libraries(Logger_static ${Logger_dependencies})


target_include_directories(Logger_shared PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
target_include_directories(Logger_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
	# $<INSTALL_INTERFACE:inc>  # <prefix>/inc 
)
# Define the install path
message("Installation to path: ${CMAKE_INSTALL_PREFIX}/bin")
message("Installation to path: ${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_FOLDER}")
install(TARGETS Logger_shared DESTINATION "${CMAKE_INSTALL_PREFIX}/bin")
install(TARGETS Logger_shared DESTINATION "${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_FOLDER}")
install(TARGETS Logger_static DESTINATION "${CMAKE_INSTALL_PREFIX}/${RELATIVE_INSTALL_FOLDER}")
