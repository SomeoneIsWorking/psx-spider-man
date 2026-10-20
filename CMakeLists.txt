cmake_minimum_required(VERSION 3.21)
project(spider_ports CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PSXPORT_DIR "${CMAKE_SOURCE_DIR}/external/psxport" CACHE PATH
    "psxport framework source directory")
if(NOT EXISTS "${PSXPORT_DIR}/cmake/psxport.cmake")
  message(FATAL_ERROR
    "PSXPORT_DIR=${PSXPORT_DIR} has no cmake/psxport.cmake; run ./run.sh once to fetch it")
endif()
message(STATUS "psxport framework: ${PSXPORT_DIR}")

find_package(ZLIB REQUIRED)
find_package(zstd CONFIG QUIET)
if(NOT TARGET zstd::libzstd_shared)
  find_library(ZSTD_LIBRARY zstd REQUIRED)
  add_library(zstd::libzstd_shared SHARED IMPORTED)
  set_target_properties(zstd::libzstd_shared PROPERTIES IMPORTED_LOCATION "${ZSTD_LIBRARY}")
endif()
find_package(Threads REQUIRED)

add_subdirectory(${PSXPORT_DIR}/vendor/beetle-psx/deps/libchdr
                 ${CMAKE_BINARY_DIR}/libchdr_build EXCLUDE_FROM_ALL)
include(${PSXPORT_DIR}/cmake/psxport.cmake)
include(cmake/title_manifest.cmake)

# A target that sees the shared owners under game/ and the titles under titles/.
function(spider_configure_runtime_target TARGET)
  target_include_directories(${TARGET} PRIVATE game titles)
  target_link_libraries(${TARGET} PRIVATE psxport)
  set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endfunction()

add_library(spider_shared STATIC
  game/execution/guest_execution.cpp
  game/runtime/runtime.cpp)
spider_configure_runtime_target(spider_shared)

set(SPIDERMAN1_SOURCES
  titles/spiderman1/runtime.cpp
  titles/spiderman1/session.cpp
  titles/spiderman1/frame_driver.cpp
  titles/spiderman1/level_warp.cpp
  titles/spiderman1/stream_driver.cpp
  titles/spiderman1/cd_initialization.cpp
  titles/spiderman1/field_clock.cpp)

set(SPIDERMAN2_SOURCES
  titles/spiderman2/frame_driver.cpp
  titles/spiderman2/runtime.cpp)

# Both titles' manifest facts, as SPIDER1_TITLE_* and SPIDER2_TITLE_*; only the catalog reads them.
function(spider_add_catalog_manifests TARGET)
  foreach(title IN ITEMS spiderman1:SPIDER1 spiderman2:SPIDER2)
    string(REPLACE ":" ";" title "${title}")
    list(GET title 0 id)
    list(GET title 1 prefix)
    spider_read_title(${id} ${prefix})
    target_compile_definitions(${TARGET} PRIVATE
      ${prefix}_TITLE_ID="${${prefix}_ID}"
      ${prefix}_TITLE_LABEL="${${prefix}_LABEL}"
      ${prefix}_TITLE_SERIAL="${${prefix}_SERIAL}"
      ${prefix}_TITLE_DISC_ENV="${${prefix}_DISCENV}"
      ${prefix}_TITLE_DISC_EXECUTABLE="${${prefix}_DISCEXECUTABLE}"
      ${prefix}_TITLE_EXECUTABLE_SIZE=${${prefix}_FILESIZE}
      ${prefix}_TITLE_EXECUTABLE_SHA256="${${prefix}_EXECUTABLESHA256}"
      ${prefix}_TITLE_HEADER_ENTRY=${${prefix}_HEADER_ENTRY}u
      ${prefix}_TITLE_HEADER_GP=${${prefix}_HEADER_GP}u
      ${prefix}_TITLE_HEADER_TEXT_ADDRESS=${${prefix}_HEADER_TEXTADDRESS}u
      ${prefix}_TITLE_HEADER_TEXT_SIZE=${${prefix}_HEADER_TEXTSIZE}u
      ${prefix}_TITLE_HEADER_STACK_ADDRESS=${${prefix}_HEADER_STACKADDRESS}u
      ${prefix}_TITLE_HEADER_STACK_OFFSET=${${prefix}_HEADER_STACKOFFSET}u)
  endforeach()
endfunction()

# Each title's own sources.
add_library(spiderman1_title OBJECT ${SPIDERMAN1_SOURCES})
add_library(spiderman2_title OBJECT ${SPIDERMAN2_SOURCES})
spider_configure_runtime_target(spiderman1_title)
spider_configure_runtime_target(spiderman2_title)

psxport_add_product(spiderman_port
  CATALOG spider::createCatalog
  HEADER product/catalog.h
  SOURCES game/product/catalog.cpp
  LIBRARIES spiderman1_title spiderman2_title spider_shared)
spider_configure_runtime_target(spiderman_port)
spider_add_catalog_manifests(spiderman_port)

# Render owners no product target links yet; built as one library so the contracts stay compiling.
add_library(spider_render_contracts STATIC
  game/render/frame_envelope.cpp
  game/render/gpu_env.cpp
  game/render/mesh_animated_vertex.cpp
  game/render/mesh_asset_cook.cpp
  game/render/mesh_face_format.cpp
  game/render/mesh_pose_contract.cpp
  game/render/mesh_transform.cpp
  game/render/mips_fixed_point.cpp
  game/render/scene_id.cpp)
target_include_directories(spider_render_contracts PRIVATE game/render)
target_link_libraries(spider_render_contracts PRIVATE psxport)
