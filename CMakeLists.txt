#########################################################################

# CMAKE MINIMAL VERSION
cmake_minimum_required (VERSION 3.8)

# PROJECT PROPERTIES
project ("CMM language" LANGUAGES CXX)

#########################################################################

# ASSEMBLY OPTIONS

if(NOT CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

add_compile_options(
    -Wall
    -Wpedantic

    -Wno-int-to-pointer-cast
    -Wno-variadic-macros
    -Wno-unused-variable
    -Wno-unused-parameter
    -Wno-unused-function
    -Wno-sign-compare
    -Wno-misleading-indentation
    -Wno-unused-but-set-variable
)

#########################################################################

set(COMPILER_LIB_SRC "compiler_lib_src")

# nCPU

set(NCPU_DIR nCPU)
add_subdirectory(${COMPILER_LIB_SRC}/${NCPU_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${NCPU_DIR})

# TEXT LIBRARY

set(TEXT_LIB_PATH)
set(TEXT_LIB_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${COMPILER_LIB_SRC}/libtext)

find_library(TEXT_LIB_PATH NAMES text PATHS ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/${NCPU_DIR}/libtext)

if (NOT TEXT_LIB_PATH)
    set(TEXT_LIB_DIR ${CMAKE_CURRENT_BINARY_DIR}/libtext)
    add_subdirectory(${TEXT_LIB_SRC_DIR} ${TEXT_LIB_DIR})
    set(TEXT_LIB_PATH text)
endif()

# TREE

set(TREE_LIB_NAME "tree")
set(TREE_LIB_PATH ${TREE_LIB_NAME})
set(TREE_LIB_SRC_DIR ${TREE_LIB_NAME})

set(TREE_LIB_SRC
    tree/tree.cpp
)

add_library(${TREE_LIB_NAME} STATIC)
target_include_directories(${TREE_LIB_NAME} PRIVATE ${TREE_LIB_NAME})
target_sources(${TREE_LIB_NAME} PRIVATE ${TREE_LIB_SRC})

# CMM FRONTEND

set(LANG_FRONTEND_NAME "cmm_frontend")

set(LANG_FRONTEND_SRC
    frontend/main.cpp
    frontend/parsing.cpp
    frontend/tree_recording.cpp
)

add_executable(${LANG_FRONTEND_NAME} ${LANG_FRONTEND_SRC})

target_include_directories(${LANG_FRONTEND_NAME} PRIVATE ${TEXT_LIB_SRC_DIR})
target_include_directories(${LANG_FRONTEND_NAME} PRIVATE ${TREE_LIB_SRC_DIR})
target_link_libraries(${LANG_FRONTEND_NAME} ${TEXT_LIB_PATH})
target_link_libraries(${LANG_FRONTEND_NAME} ${TREE_LIB_PATH})

# LANG OPTIMIZER

set(LANG_OPTIMIZER_NAME "lang_optimizer")

set(LANG_OPTIMIZER_SRC
    optimizer/main.cpp
    optimizer/derivative.cpp
    optimizer/optimizer.cpp
    optimizer/tree_reading.cpp
    optimizer/tree_recording.cpp
)

add_executable(${LANG_OPTIMIZER_NAME} ${LANG_OPTIMIZER_SRC})

target_include_directories(${LANG_OPTIMIZER_NAME} PRIVATE ${TEXT_LIB_SRC_DIR})
target_include_directories(${LANG_OPTIMIZER_NAME} PRIVATE ${TREE_LIB_SRC_DIR})
target_link_libraries(${LANG_OPTIMIZER_NAME} ${TEXT_LIB_PATH})
target_link_libraries(${LANG_OPTIMIZER_NAME} ${TREE_LIB_PATH})

# LANG REVERSED FRONTEND

set(CMM_REV_FRONTEND_NAME "cmm_frontend_rev")

set(CMM_REV_FRONTEND_SRC
    frontend_reversed/main.cpp
    frontend_reversed/frontend_rev.cpp
    frontend_reversed/tree_reading.cpp
)

add_executable(${CMM_REV_FRONTEND_NAME} ${CMM_REV_FRONTEND_SRC})

target_include_directories(${CMM_REV_FRONTEND_NAME} PRIVATE ${TEXT_LIB_SRC_DIR})
target_include_directories(${CMM_REV_FRONTEND_NAME} PRIVATE ${TREE_LIB_SRC_DIR})
target_link_libraries(${CMM_REV_FRONTEND_NAME} ${TEXT_LIB_PATH})
target_link_libraries(${CMM_REV_FRONTEND_NAME} ${TREE_LIB_PATH})

# LANG BACKEND

set(LANG_BACKEND_NAME "lang_backend")

set(LANG_BACKEND_SRC
    backend/main.cpp
    backend/backend.cpp
    backend/tree_reading.cpp
)

add_executable(${LANG_BACKEND_NAME} ${LANG_BACKEND_SRC})

target_include_directories(${LANG_BACKEND_NAME} PRIVATE ${TEXT_LIB_SRC_DIR})
target_include_directories(${LANG_BACKEND_NAME} PRIVATE ${TREE_LIB_SRC_DIR})
target_link_libraries(${LANG_BACKEND_NAME} ${TEXT_LIB_PATH})
target_link_libraries(${LANG_BACKEND_NAME} ${TREE_LIB_PATH})

#########################################################################
