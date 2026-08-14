cmake_minimum_required(VERSION 3.21)
set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
project(Vulcano LANGUAGES CXX)

# Project folders layout
set(MAIN_DIR .)
set(SRC_DIR src)
set(INCLUDE_DIR include)

# Create folder for dependencies
set(LIBS_DIR "${CMAKE_BINARY_DIR}/libs_include")
file(MAKE_DIRECTORY "${LIBS_DIR}")

# Define single-header dependencies URLs
set(DEP_FILENAMES
  "json.hpp"
  "plusaes.hpp"
  "stb_image.h"
  "stb_image_write.h"
  "tiny_gltf.h"
  "tiny_obj_loader.h"
  "sdefl.h"
  "sinfl.h"
)
set(DEP_URLS
  "https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp"
  "https://raw.githubusercontent.com/kkAyataka/plusaes/master/include/plusaes/plusaes.hpp"
  "https://raw.githubusercontent.com/nothings/stb/master/stb_image.h"
  "https://raw.githubusercontent.com/nothings/stb/master/stb_image_write.h"
  "https://raw.githubusercontent.com/syoyo/tinygltf/v2.8.23/tiny_gltf.h"
  "https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/v2.0.0rc10/tiny_obj_loader.h"
  "https://raw.githubusercontent.com/vurtun/lib/master/sdefl.h"
  "https://raw.githubusercontent.com/vurtun/lib/master/sinfl.h"
)

# DOWNLOAD EACH DEPENDENCIES
# dep_count = len(dep_filenames)
# dep_max = dep_count - 1
# for index in range(dep_max)
  # filename = dep_filenames[index]
  # url = dep_urls[index]
  # dest = "{libs_dir}/{filename}"
  # if ! exists(dest)
    # print(...)
    # download_status = download(url, dest)
    # status_code = download_status[0]
    # if status_code != 0
      # print(...)
list(LENGTH DEP_FILENAMES DEP_COUNT)
math(EXPR DEP_MAX "${DEP_COUNT} - 1")
foreach(INDEX RANGE ${DEP_MAX})
  list(GET DEP_FILENAMES ${INDEX} FILENAME)
  list(GET DEP_URLS ${INDEX} URL)
  set(DEST "${LIBS_DIR}/${FILENAME}")
  if(NOT EXISTS "${DEST}")
    message(STATUS "Downloading ${FILENAME} from ${URL}...")
    file(DOWNLOAD "${URL}" "${DEST}" STATUS DOWNLOAD_STATUS)
    list(GET DOWNLOAD_STATUS 0 STATUS_CODE)
    if(NOT STATUS_CODE EQUAL 0)
      message(FATAL_ERROR "Failed to download ${FILENAME}: ${DOWNLOAD_STATUS}")
    endif()
  endif()
endforeach()

# Find all source files (src/*.cpp) and header files (include/**/*.hpp)
file(GLOB SRC_FILES CONFIGURE_DEPENDS "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE INC_FILES CONFIGURE_DEPENDS "${INCLUDE_DIR}/*.hpp" "${INCLUDE_DIR}/*.h")

# Compile executable/target (named as project) including all source and header files
add_executable(${PROJECT_NAME} ${SRC_FILES} ${INC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE "${INCLUDE_DIR}" "${LIBS_DIR}")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

# Avoid generating debug symbols for libraries to increase compilation speed
set_source_files_properties(${SRC_DIR}/Libs.cpp PROPERTIES COMPILE_OPTIONS "-g0")

# Link Vulkan to executable
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

# Dependencies: GLFW, GLM
include(FetchContent)

# GLFW
FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
  EXCLUDE_FROM_ALL
)

# Deactivate non-essential components to speed up the build
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

# Download
FetchContent_MakeAvailable(glfw)

# GLM
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
  EXCLUDE_FROM_ALL

)

# Deactivate non-essential components to speed up the build
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)

# Download
FetchContent_MakeAvailable(glm)

# Link GLFW and GLM
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glm::glm)

# Shader compilation (glslc)
set(SHADER_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/spv")
set(SHADER_OUT_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders")

# Find shader files
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS "${SHADER_SRC_DIR}/*.vert" "${SHADER_SRC_DIR}/*.frag" "${SHADER_SRC_DIR}/*.comp")

# COMPILE EACH SHADER
# spirv_outputs = []
# spirv_copied_stamps = []
# for shader in shader_files
  # shader_rel_path = relative_path(shader_src_dir, shader)
  # shader_rel_dir = dirname(shader_rel_path)
  # shader_file_name = basename(shader)
  # spirv = "{shader_build_dir}/{shader_rel_path}.spv"
  # copy_stamp = "{cmake_current_binary_dir}/shader_copy_stamps/{shader_rel_path}.stamp"
  # final_shader_dir = "{shader_out_dir}/{shader_rel_dir}"
  # mkdir("{shader_build_dir}/{shader_rel_dir}")
  # compile(shader, spirv)
  # print(...)
  # mkdir(final_shader_dir)
  # cp(spirv, final_shader_dir)
  # mkdir("{cmake_current_binary_dir}/shader_copy_stamps/{shader_rel_dir}")
  # touch(copy_stamp)
  # spirv_outputs.append(spirv)
  # spirv_copied_stamps.append(copy_stamp)
set(SPIRV_OUTPUTS "")
set(SPIRV_COPIED_STAMPS "")
foreach(SHADER IN LISTS SHADER_FILES)
  file(RELATIVE_PATH SHADER_REL_PATH "${SHADER_SRC_DIR}" "${SHADER}")
  get_filename_component(SHADER_REL_DIR "${SHADER_REL_PATH}" DIRECTORY)
  get_filename_component(SHADER_FILE_NAME "${SHADER}" NAME)
  set(SPIRV "${SHADER_BUILD_DIR}/${SHADER_REL_PATH}.spv")
  set(COPY_STAMP "${CMAKE_CURRENT_BINARY_DIR}/shader_copy_stamps/${SHADER_REL_PATH}.stamp")
  set(FINAL_SHADER_DIR "${SHADER_OUT_DIR}/${SHADER_REL_DIR}")
  add_custom_command(
          OUTPUT "${SPIRV}"
          COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_BUILD_DIR}/${SHADER_REL_DIR}"
          COMMAND "${Vulkan_GLSLC_EXECUTABLE}" -o "${SPIRV}" "${SHADER}"
          DEPENDS "${SHADER}"
          COMMENT "Building shader: ${SHADER_REL_PATH}"
          VERBATIM
  )
  add_custom_command(
          OUTPUT "${COPY_STAMP}"
          COMMAND ${CMAKE_COMMAND} -E make_directory "${FINAL_SHADER_DIR}"
          COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SPIRV}" "${FINAL_SHADER_DIR}/"
          COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/shader_copy_stamps/${SHADER_REL_DIR}"
          COMMAND ${CMAKE_COMMAND} -E touch "${COPY_STAMP}"
          DEPENDS "${SPIRV}"
          COMMENT ""
          VERBATIM
  )
  list(APPEND SPIRV_OUTPUTS "${SPIRV}")
  list(APPEND SPIRV_COPIED_STAMPS "${COPY_STAMP}")
endforeach()

# Make sure shaders are always compiled before main executable
add_custom_target(shaders ALL DEPENDS ${SPIRV_COPIED_STAMPS})
add_dependencies(${PROJECT_NAME} shaders)

# Copy assets
set(ASSET_SRC_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSET_DST_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets")
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSET_SRC_DIR}" "${ASSET_DST_DIR}"
)

