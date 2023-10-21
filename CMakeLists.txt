cmake_minimum_required(VERSION 3.16)

# project settings
project(imgui_test)
set(CMAKE_CXX_STANDARD 20)
add_definitions("-D_XKEYCHECK_H")
add_definitions(-DPROJECT_DIR="${PROJECT_SOURCE_DIR}")

# paths
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
set(LIB_DIR_D ${PROJECT_SOURCE_DIR}/lib/Debug/lib)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib/Release/lib)

# include
include_directories(${INCLUDE_DIR})

# src files
file(GLOB header_files ${SRC_DIR}/*.h
                       ${SRC_DIR}/imgui_components/*.h)
file(GLOB src_files ${SRC_DIR}/*.cpp
                    ${SRC_DIR}/imgui_components/*.cpp)

# pre processing
if(WIN32)
    # Windows
    message("OS: Windows")

    file(GLOB lib_files_d ${LIB_DIR_D}/*.lib)
    file(GLOB lib_files ${LIB_DIR}/*.lib)
elseif(UNIX AND NOT APPLE)
    # Linux
    message("OS: Linux")

    file(GLOB lib_files_d ${LIB_DIR_D}/*.a)
    file(GLOB lib_files ${LIB_DIR}/*.a)

    # X11
    find_package(X11 REQUIRED)

    # freetype
    find_package(Freetype REQUIRED)

    # message("${FREETYPE_LIBRARIES}")

elseif(APPLE)
    # Apple
    message("OS: Apple")

endif()

# Generate the list of files to link, per flavor.
set (LINK_LIST "")
foreach(x ${lib_files_d})
    list (APPEND LINK_LIST debug ${x})
endforeach()
foreach(x ${lib_files})
    list (APPEND LINK_LIST optimized ${x})
endforeach()
message("${LINK_LIST}")

# add src files to executable
add_executable(${PROJECT_NAME} ${header_files} ${src_files})

# link lib files
target_link_libraries(${PROJECT_NAME} ${LINK_LIST})
if(UNIX AND NOT APPLE)
    # bind X11 libs if linux
    target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES})
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}")

# install paths
install(TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Debug
    RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/Debug)
install(TARGETS ${PROJECT_NAME}
    CONFIGURATIONS Release
    RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/Release)

# post processing
if(WIN32)
    # Windows
    add_custom_command(TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/lib/$<CONFIGURATION>/bin/*.dll" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        VERBATIM)
elseif(UNIX AND NOT APPLE)
    # Linux
    # add_custom_command(TARGET ${PROJECT_NAME}
    #     POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJECT_SOURCE_DIR}/lib/$<CONFIGURATION>/bin/*.so" "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    #     VERBATIM)
elseif(APPLE)
    # Apple
endif()

# LINK : warning LNK4098: 預設的程式庫 'MSVCRT' 與其他使用的程式庫衝突，請使用 /NODEFAULTLIB:library
# if (MSVC)
#     set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
# endif()
