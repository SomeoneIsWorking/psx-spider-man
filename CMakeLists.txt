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

set(SPIDER_COMMON_SOURCES game/core/guest_execution.cpp)

set(SPIDER1_FRAME_SOURCES
  titles/spiderman1/spider1_bootstrap_turn.cpp
  titles/spiderman1/spider1_stream_driver.cpp
  titles/spiderman1/spider1_movie_execution.cpp
  titles/spiderman1/spider1_frame_driver.cpp
  # Mode driver and the retail mode functions it composes.
  titles/spiderman1/spider1_cd_initialization.cpp
  titles/spiderman1/spider1_field_clock.cpp
  titles/spiderman1/spider1_gpu_reset.cpp
  titles/spiderman1/spider1_host_stepped_fiber.cpp
  titles/spiderman1/spider1_mode_driver.cpp
  titles/spiderman1/spider1_alternate_mode.cpp
  titles/spiderman1/spider1_invalid_selector_input.cpp
  titles/spiderman1/spider1_menu_mode.cpp
  titles/spiderman1/spider1_mode_frame_boundary.cpp
  titles/spiderman1/spider1_transition_wipe.cpp)

# Linked into the product: the title runtime returns its policy and registers two overrides.
set(SPIDER1_WIDESCREEN_SOURCES titles/spiderman1/spider1_widescreen.cpp)

set(ENTER_ELECTRO_SOURCES
  titles/spiderman2/enter_electro_frame_driver.cpp
  titles/spiderman2/enter_electro_runtime.cpp)

# A runtime target that sees game/core and the headers of the named titles only.
function(spider_configure_runtime_target TARGET)
  cmake_parse_arguments(PARSE_ARGV 1 ARG "" "" "TITLES")
  target_include_directories(${TARGET} PRIVATE game/core)
  foreach(title IN LISTS ARG_TITLES)
    target_include_directories(${TARGET} PRIVATE titles/${title})
  endforeach()
  target_link_libraries(${TARGET} PRIVATE psxport)
  set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endfunction()

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
add_library(spider1_title OBJECT
  titles/spiderman1/spider1_runtime.cpp ${SPIDER1_FRAME_SOURCES} ${SPIDER1_WIDESCREEN_SOURCES})
add_library(enter_electro_title OBJECT ${ENTER_ELECTRO_SOURCES})
spider_configure_runtime_target(spider1_title TITLES spiderman1)
spider_configure_runtime_target(enter_electro_title TITLES spiderman2)

psxport_add_product(spiderman_port
  CATALOG spider::createCatalog
  HEADER spider_catalog.h
  SOURCES ${SPIDER_COMMON_SOURCES} game/core/spider_catalog.cpp
  LIBRARIES spider1_title enter_electro_title)
spider_configure_runtime_target(spiderman_port TITLES spiderman1 spiderman2)
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

  add_executable(spider1_widescreen_test
    tests/spider1_widescreen_test.cpp
    titles/spiderman1/spider1_widescreen.cpp)
  target_include_directories(spider1_widescreen_test PRIVATE
    game/core titles/spiderman1 ${PSXPORT_DIR}/tests)
  target_link_libraries(spider1_widescreen_test PRIVATE psxport)
  add_test(NAME spider1_widescreen COMMAND spider1_widescreen_test)

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

  add_executable(spider1_mode_transition_test tests/spider1_mode_transition_test.cpp)
  target_include_directories(spider1_mode_transition_test PRIVATE titles/spiderman1)
  add_test(NAME spider1_mode_transition COMMAND spider1_mode_transition_test)

  add_executable(spider1_field_schedule_test tests/spider1_field_schedule_test.cpp)
  target_include_directories(spider1_field_schedule_test PRIVATE titles/spiderman1)
  add_test(NAME spider1_field_schedule COMMAND spider1_field_schedule_test)

  # Mode driver decisions, tested without a Core; links the wipe to test the shipping darkening rule.
  add_executable(spider1_mode_rules_test
    tests/spider1_mode_rules_test.cpp titles/spiderman1/spider1_transition_wipe.cpp)
  target_include_directories(spider1_mode_rules_test PRIVATE titles/spiderman1 ${PSXPORT_DIR}/tests)
  target_link_libraries(spider1_mode_rules_test PRIVATE psxport)
  add_test(NAME spider1_mode_rules COMMAND spider1_mode_rules_test)

  # Guest-boundary owners against a real Core with native-override probes that record call order.
  add_executable(spider1_guest_boundary_owners_test
    tests/spider1_guest_boundary_owners_test.cpp
    titles/spiderman1/spider1_cd_initialization.cpp
    titles/spiderman1/spider1_field_clock.cpp
    titles/spiderman1/spider1_gpu_reset.cpp
    titles/spiderman1/spider1_host_stepped_fiber.cpp
    titles/spiderman1/spider1_invalid_selector_input.cpp
    titles/spiderman1/spider1_mode_frame_boundary.cpp)
  target_include_directories(spider1_guest_boundary_owners_test
    PRIVATE game/core titles/spiderman1 ${PSXPORT_DIR}/tests)
  target_link_libraries(spider1_guest_boundary_owners_test PRIVATE psxport)
  add_test(NAME spider1_guest_boundary_owners COMMAND spider1_guest_boundary_owners_test)

  add_test(NAME title_catalog
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/test_title_catalog.py)
  add_test(NAME executable_provision
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/test_provision.py)
  add_test(NAME source_policy
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/source_policy.py)
  # CD-ROM interrupt ownership contract; its baseline assertion is expected to fail once issue 0026 lands (docs/issues/0026).
  add_executable(cd_irq2_delivery_test tests/cd_irq2_delivery_test.cpp)
  target_link_libraries(cd_irq2_delivery_test PRIVATE psxport)
  add_test(NAME cd_irq2_delivery COMMAND cd_irq2_delivery_test)
  set(SPIDER_HEADLESS_TEST_ENVIRONMENT
    "PSXPORT_NOWINDOW=1;PSXPORT_NOAUDIO=1;PSXPORT_NOPACE=1;PSXPORT_ASSET_DIR=${PSXPORT_DIR}")
  add_executable(spider1_runtime_services_test
    ${SPIDER_COMMON_SOURCES} tests/spider1_runtime_services_test.cpp)
  spider_configure_runtime_target(spider1_runtime_services_test TITLES spiderman1)
  target_link_libraries(spider1_runtime_services_test PRIVATE spider1_title)
  target_include_directories(spider1_runtime_services_test PRIVATE ${PSXPORT_DIR}/tests)
  add_test(NAME spider1_runtime_services COMMAND spider1_runtime_services_test)
  set_tests_properties(spider1_runtime_services PROPERTIES ENVIRONMENT "${SPIDER_HEADLESS_TEST_ENVIRONMENT}")
  add_executable(enter_electro_frame_driver_test
    ${SPIDER_COMMON_SOURCES} tests/enter_electro_frame_driver_test.cpp)
  spider_configure_runtime_target(enter_electro_frame_driver_test TITLES spiderman2)
  target_link_libraries(enter_electro_frame_driver_test PRIVATE enter_electro_title)
  target_include_directories(enter_electro_frame_driver_test PRIVATE ${PSXPORT_DIR}/tests)
  add_test(NAME enter_electro_frame_driver COMMAND enter_electro_frame_driver_test)
  set_tests_properties(enter_electro_frame_driver PROPERTIES
    ENVIRONMENT "${SPIDER_HEADLESS_TEST_ENVIRONMENT}")
  add_executable(spider_catalog_test
    ${SPIDER_COMMON_SOURCES} tests/spider_catalog_test.cpp game/core/spider_catalog.cpp)
  spider_configure_runtime_target(spider_catalog_test TITLES spiderman1 spiderman2)
  spider_add_catalog_manifests(spider_catalog_test)
  target_link_libraries(spider_catalog_test PRIVATE spider1_title enter_electro_title)
  target_include_directories(spider_catalog_test PRIVATE ${PSXPORT_DIR}/tests)
  add_test(NAME spider_catalog COMMAND spider_catalog_test WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
  add_test(NAME launcher_help
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tests/test_help_contract.py
            --executable $<TARGET_FILE:spiderman_port>)

  # One binary links both titles, so no external name may be defined by both from their own sources.
  psxport_add_title_link_isolation_test(spider_title_link_isolation
    FIRST spider1_title FIRST_ROOT "${CMAKE_SOURCE_DIR}/titles/spiderman1"
    SECOND enter_electro_title SECOND_ROOT "${CMAKE_SOURCE_DIR}/titles/spiderman2")

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
            --root ${CMAKE_SOURCE_DIR} --compile-commands ${CMAKE_BINARY_DIR}
            # Shrink-only caps; lower them whenever the file shrinks.
            --cap titles/spiderman1/spider1_frame_driver.cpp=437
            --cap titles/spiderman1/spider1_mode_driver.cpp=521)
endif()
