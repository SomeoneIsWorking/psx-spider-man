cmake_minimum_required(VERSION 3.21)
project(spider_ports CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PSXPORT_DIR "${CMAKE_SOURCE_DIR}/external/psxport" CACHE PATH
    "psxport framework source directory")
if(NOT EXISTS "${PSXPORT_DIR}/cmake/psxport.cmake")
  message(FATAL_ERROR
    "PSXPORT_DIR=${PSXPORT_DIR} has no cmake/psxport.cmake; run tools/psxport_fetch.py")
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

# Each title's own sources, so the link-isolation test can tell the two trees apart.
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
  game/render/scene_id.cpp)
target_include_directories(spider_render_contracts PRIVATE game/render)
target_link_libraries(spider_render_contracts PRIVATE psxport)

include(CTest)
if(BUILD_TESTING)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  add_executable(mesh_transform_contract_test
    tests/mesh_transform_contract_test.cpp
    game/render/mesh_transform.cpp)
  target_include_directories(mesh_transform_contract_test PRIVATE game/render)
  add_test(NAME mesh_transform_contract COMMAND mesh_transform_contract_test)

  add_executable(mesh_animated_vertex_test
    tests/mesh_animated_vertex_test.cpp
    game/render/mesh_animated_vertex.cpp
    game/render/mips_fixed_point.cpp)
  target_include_directories(mesh_animated_vertex_test PRIVATE game/render)
  add_test(NAME mesh_animated_vertex COMMAND mesh_animated_vertex_test)

  add_executable(mesh_pose_contract_test
    tests/mesh_pose_contract_test.cpp
    game/render/mesh_pose_contract.cpp
    game/render/mips_fixed_point.cpp)
  target_include_directories(mesh_pose_contract_test PRIVATE game/render)
  add_test(NAME mesh_pose_contract COMMAND mesh_pose_contract_test)

  add_executable(mesh_asset_cook_test
    tests/mesh_asset_cook_test.cpp
    game/render/mesh_asset_cook.cpp)
  target_include_directories(mesh_asset_cook_test PRIVATE game/render)
  add_test(NAME mesh_asset_cook COMMAND mesh_asset_cook_test)

  add_executable(mesh_face_format_test
    tests/mesh_face_format_test.cpp
    game/render/mesh_face_format.cpp)
  target_include_directories(mesh_face_format_test PRIVATE game/render)
  add_test(NAME mesh_face_format COMMAND mesh_face_format_test)

  # A test of one title's owners through the shipping sources, with psxport's test support.
  function(spider_add_title_test NAME SOURCE)
    add_executable(${NAME}_test ${SOURCE} ${ARGN})
    spider_configure_runtime_target(${NAME}_test)
    target_include_directories(${NAME}_test PRIVATE ${PSXPORT_DIR}/tests)
    add_test(NAME ${NAME} COMMAND ${NAME}_test)
  endfunction()

  # Field clock and CdInit against a bare Core.
  spider_add_title_test(spiderman1_guest_boundary_owners
    tests/spiderman1/guest_boundary_owners_test.cpp
    titles/spiderman1/cd_initialization.cpp
    titles/spiderman1/field_clock.cpp)
  target_link_libraries(spiderman1_guest_boundary_owners_test PRIVATE psxport_test_support)

  set(SPIDER_HEADLESS_TEST_ENVIRONMENT
    "PSXPORT_NOWINDOW=1;PSXPORT_NOAUDIO=1;PSXPORT_NOPACE=1;PSXPORT_ASSET_DIR=${PSXPORT_DIR}")
  spider_add_title_test(spiderman1_runtime tests/spiderman1/runtime_test.cpp)
  target_link_libraries(spiderman1_runtime_test PRIVATE spiderman1_title spider_shared)
  set_tests_properties(spiderman1_runtime PROPERTIES ENVIRONMENT "${SPIDER_HEADLESS_TEST_ENVIRONMENT}")
  spider_add_title_test(spiderman2_frame_driver tests/spiderman2/frame_driver_test.cpp)
  target_link_libraries(spiderman2_frame_driver_test PRIVATE spiderman2_title spider_shared)
  set_tests_properties(spiderman2_frame_driver PROPERTIES ENVIRONMENT "${SPIDER_HEADLESS_TEST_ENVIRONMENT}")
  spider_add_title_test(catalog tests/catalog_test.cpp game/product/catalog.cpp)
  spider_add_catalog_manifests(catalog_test)
  target_link_libraries(catalog_test PRIVATE spiderman1_title spiderman2_title spider_shared)
  set_tests_properties(catalog PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

  add_test(NAME title_catalog
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/test_title_catalog.py)
  add_test(NAME executable_provision
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/test_provision.py)
  add_test(NAME launcher_help
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/test_help_contract.py
            --executable $<TARGET_FILE:spiderman_port>)

  # One binary links both titles, so no external name may be defined by both from their own sources.
  psxport_add_title_link_isolation_test(spider_title_link_isolation
    FIRST spiderman1_title FIRST_ROOT "${CMAKE_SOURCE_DIR}/titles/spiderman1"
    SECOND spiderman2_title SECOND_ROOT "${CMAKE_SOURCE_DIR}/titles/spiderman2")

  # The shipped picker with both titles live at the end of the run, not merely once. Local-only: it
  # needs the provisioned executables, and skips where neither is provisioned (CI).
  add_test(NAME spider_picker_both_panels_live COMMAND spiderman_port WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
  set_tests_properties(spider_picker_both_panels_live PROPERTIES
    ENVIRONMENT "PSXPORT_DEBUG=picker;PSXPORT_NATIVE_FRAMES=1500;PSXPORT_NOAUDIO=1;PSXPORT_ASSET_DIR=${PSXPORT_DIR}"
    PASS_REGULAR_EXPRESSION "with no choice; panels: \\[0\\]\\*spiderman1 demo \\[1\\] spiderman2 demo"
    SKIP_REGULAR_EXPRESSION "no provisioned title"
    # A settled panel that drops back to booting has lost its picture.
    FAIL_REGULAR_EXPRESSION "\\[watchdog\\] FAULT;spiderman1 demo.*spiderman1 booting;spiderman2 demo.*spiderman2 booting"
    TIMEOUT 300)

  add_test(NAME cpp_policy
    COMMAND ${Python3_EXECUTABLE} ${PSXPORT_DIR}/tools/check_cpp_style.py
            --root ${CMAKE_SOURCE_DIR} --compile-commands ${CMAKE_BINARY_DIR})
endif()
