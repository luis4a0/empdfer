cmake_minimum_required(VERSION 3.20)

project(empdfer
        DESCRIPTION "Embed images into a PDF"
        VERSION 0.9.0
        LANGUAGES CXX)

option(EMPDFER_USE_PNG "Use libpng" ON)

add_compile_definitions(EMPDFER_VERSION_MAJOR=${PROJECT_VERSION_MAJOR})
add_compile_definitions(EMPDFER_VERSION_MINOR=${PROJECT_VERSION_MINOR})
add_compile_definitions(EMPDFER_VERSION_PATCH=${PROJECT_VERSION_PATCH})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(EMPDFER_SOURCES create_page.cpp file_type.cpp matrix.cpp empdfer.cpp jpeg_file.cpp)

if(EMPDFER_USE_PNG)
    set(EMPDFER_SOURCES ${EMPDFER_SOURCES} png_file.cpp)
endif(EMPDFER_USE_PNG)

add_executable(empdfer ${EMPDFER_SOURCES})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules")

find_package(Paddlefish REQUIRED)
target_include_directories(empdfer PRIVATE ${PADDLEFISH_INCLUDE_DIRS})
target_link_libraries(empdfer ${PADDLEFISH_LIBRARY_RELEASE})
cmake_path(GET PADDLEFISH_LIBRARY_RELEASE PARENT_PATH PADDLEFISH_LIBRARY_PATH)

find_package(JPEG REQUIRED)
target_include_directories(empdfer PRIVATE ${JPEG_INCLUDE_DIRS})
target_link_libraries(empdfer ${JPEG_LIBRARY_RELEASE})
cmake_path(GET JPEG_LIBRARY_RELEASE PARENT_PATH JPEG_LIBRARY_PATH)

if(EMPDFER_USE_PNG)
    find_package(PNG REQUIRED)
    add_compile_definitions(EMPDFER_USE_PNG)
    target_include_directories(empdfer PRIVATE ${PNG_INCLUDE_DIRS})
    target_link_libraries(empdfer ${PNG_LIBRARY_RELEASE})
    cmake_path(GET PNG_LIBRARY_RELEASE PARENT_PATH PNG_LIBRARY_PATH)
endif(EMPDFER_USE_PNG)

if(NOT MSVC)
    add_compile_options(-ansi)
    add_compile_options(-Wall)
    add_compile_options(-pedantic)
endif (NOT MSVC)

if(WIN32)
    add_definitions(-DEMPDFER_WINDOWS)
elseif(APPLE)
    add_definitions(-DEMPDFER_MACOS)
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_definitions(-DEMPDFER_LINUX)
else()
    message(WARNING "Unknown platform '${CMAKE_SYSTEM_NAME}'")
endif()

include_directories(include ${CMAKE_SOURCE_DIR}/include)

if(UNIX)
    set(EMPDFER_RPATH "${PADDLEFISH_LIBRARY_PATH};${JPEG_LIBRARY_PATH}")
    if(EMPDFER_USE_PNG)
        set(EMPDFER_RPATH "${EMPDFER_RPATH};${PNG_LIBRARY_PATH}")
    endif(EMPDFER_USE_PNG)
    set_property(TARGET empdfer PROPERTY INSTALL_RPATH ${EMPDFER_RPATH})
endif(UNIX)

include(GNUInstallDirs)
install(TARGETS empdfer DESTINATION bin)
