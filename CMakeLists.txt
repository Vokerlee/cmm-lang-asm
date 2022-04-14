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

# TEXT LIBRARY

set(TEXT_LIB_NAME "text")

set(TEXT_LIB_DIR libtext)
add_subdirectory(${COMPILER_LIB_SRC}/${TEXT_LIB_DIR} ${TEXT_LIB_DIR})

# TREE

set(TREE_LIB_NAME "tree")

set(TREE_LIB_SRC
    tree/tree.cpp
)

add_library(${TREE_LIB_NAME} STATIC)
target_include_directories(${TREE_LIB_NAME} PRIVATE tree)
target_sources(${TREE_LIB_NAME} PRIVATE ${TREE_LIB_SRC})

# CMM FRONTEND

set(LANG_FRONTEND_NAME "cmm_frontend")

set(LANG_FRONTEND_SRC
    frontend/main.cpp
    frontend/parsing.cpp
    frontend/tree_recording.cpp
)

add_executable(${LANG_FRONTEND_NAME} ${LANG_FRONTEND_SRC})

target_include_directories(${LANG_FRONTEND_NAME} PRIVATE ${COMPILER_LIB_SRC}/${TEXT_LIB_DIR})
target_link_libraries(${LANG_FRONTEND_NAME} ${TEXT_LIB_NAME})
target_include_directories(${LANG_FRONTEND_NAME} PRIVATE tree)
target_link_libraries(${LANG_FRONTEND_NAME} ${TREE_LIB_NAME})

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

target_include_directories(${LANG_OPTIMIZER_NAME} PRIVATE ${COMPILER_LIB_SRC}/${TEXT_LIB_DIR})
target_link_libraries(${LANG_OPTIMIZER_NAME} ${TEXT_LIB_NAME})
target_include_directories(${LANG_OPTIMIZER_NAME} PRIVATE tree)
target_link_libraries(${LANG_OPTIMIZER_NAME} ${TREE_LIB_NAME})

#########################################################################
