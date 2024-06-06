# CMakeLists.txt: Root CMake project file

cmake_minimum_required(VERSION 3.8)

project("flush-engine")

# Ensure C++20 standard is used
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set Vulkan SDK path
if(APPLE)
    set(VULKAN_SDK "/Users/ryanflush/VulkanSDK/1.3.283.0/macOS")
    set(CMAKE_PREFIX_PATH "${VULKAN_SDK}")
    find_path(Vulkan_INCLUDE_DIR NAMES vulkan/vulkan.h HINTS "${VULKAN_SDK}/include")
    find_library(Vulkan_LIBRARY NAMES vulkan HINTS "${VULKAN_SDK}/lib")
    if(Vulkan_INCLUDE_DIR AND Vulkan_LIBRARY)
        set(Vulkan_FOUND TRUE)
        set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIR})
        set(Vulkan_LIBRARIES ${Vulkan_LIBRARY})
    else()
        set(Vulkan_FOUND FALSE)
    endif()
else()
    find_package(Vulkan REQUIRED)
endif()

if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Could not find Vulkan. Please ensure the Vulkan SDK is installed and the environment variables are set correctly.")
endif()

# Create Vulkan::Vulkan target if found manually
if(Vulkan_FOUND AND NOT TARGET Vulkan::Vulkan)
    add_library(Vulkan::Vulkan UNKNOWN IMPORTED)
    set_target_properties(Vulkan::Vulkan PROPERTIES
        IMPORTED_LOCATION ${Vulkan_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES ${Vulkan_INCLUDE_DIRS}
    )
endif()

# Add third-party libraries
add_subdirectory(third_party)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/bin")

# Add source directory
add_subdirectory(src)

# Find GLSL validator
if(APPLE)
    find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/local/bin "${VULKAN_SDK}/bin")
else()
    find_program(GLSL_VALIDATOR glslangValidator HINTS /usr/bin /usr/local/bin $ENV{VULKAN_SDK}/Bin/ $ENV{VULKAN_SDK}/Bin32/)
endif()

# Shader compilation
file(GLOB_RECURSE GLSL_SOURCE_FILES
    "${PROJECT_SOURCE_DIR}/shaders/*.frag"
    "${PROJECT_SOURCE_DIR}/shaders/*.vert"
    "${PROJECT_SOURCE_DIR}/shaders/*.comp"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  message(STATUS "BUILDING SHADER")
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_SOURCE_DIR}/shaders/${FILE_NAME}.spv")
  message(STATUS ${GLSL})
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    Shaders 
    DEPENDS ${SPIRV_BINARY_FILES}
)

