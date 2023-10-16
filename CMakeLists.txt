cmake_minimum_required(VERSION 3.16)

project(imgui_test)
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(LIB_DIR_D ${PROJECT_SOURCE_DIR}/lib/Debug/lib)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib/Release/lib)

set(CMAKE_CXX_STANDARD 20)

include_directories(${INCLUDE_DIR})

add_definitions("-D_XKEYCHECK_H")
add_definitions(-DPROJECT_DIR="${PROJECT_SOURCE_DIR}")

file(GLOB header_files ${SRC_DIR}/*.h
                       ${SRC_DIR}/imgui_components/*.h)
file(GLOB src_files ${SRC_DIR}/*.cpp
                    ${SRC_DIR}/imgui_components/*.cpp)

file(GLOB lib_files_d ${LIB_DIR_D}/*.lib)
file(GLOB lib_files ${LIB_DIR}/*.lib)
# Generate the list of files to link, per flavor.
set (LINK_LIST "")
foreach(x ${lib_files_d})
    list (APPEND LINK_LIST debug ${x})
endforeach()
foreach(x ${lib_files})
    list (APPEND LINK_LIST optimized ${x})
endforeach()

add_executable(${PROJECT_NAME} ${header_files} ${src_files})

# target_link_libraries(${PROJECT_NAME} optimized ${lib_files})
# target_link_libraries(${PROJECT_NAME} debug ${lib_files_d})
target_link_libraries(${PROJECT_NAME} ${LINK_LIST})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${PROJECT_SOURCE_DIR}/lib/$<CONFIGURATION>/bin" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    VERBATIM)

# LINK : warning LNK4098: 預設的程式庫 'MSVCRT' 與其他使用的程式庫衝突，請使用 /NODEFAULTLIB:library
# if (MSVC)
#     set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
# endif()
