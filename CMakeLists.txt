cmake_minimum_required(VERSION 3.20)

set(PROJECT_NAME CanFTPApp)

project(${PROJECT_NAME} VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(${CMAKE_SOURCE_DIR}/CANFileTransferProtocol)
add_subdirectory(${CMAKE_SOURCE_DIR}/libs/yaml-cpp)

add_executable(${PROJECT_NAME})

file(GLOB_RECURSE SOURCES ./sources/*.cpp)
file(GLOB_RECURSE HEADERS ./sources/*.hpp)
file(GLOB_RECURSE SUBDIRECTORIES LIST_DIRECTORIES TRUE ./sources/*)

FOREACH(SUBDIRECTORY ${SUBDIRECTORIES})
    IF(IS_DIRECTORY ${SUBDIRECTORY})
        target_include_directories(${PROJECT_NAME}
            PUBLIC
                ${SUBDIRECTORY}
        )
    ENDIF()
ENDFOREACH()

target_sources(${PROJECT_NAME}
    PRIVATE
        ${SOURCES}
    PUBLIC
        ${HEADERS}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CANFileTransferProtocol
        yaml-cpp::yaml-cpp
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}
)