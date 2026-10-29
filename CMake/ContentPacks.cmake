# Content packs: thin CMake wrappers around Util/ContentPacks/carla_pack.py.
# See Docs/content_packs.md.
#
#   cmake --build Build --target content-packs-test          run the tool's unit tests
#   cmake --build Build --target pack-<Pack>                 cook + tar Plugins/Packs/<Pack>
#   cmake --build Build --target pack-<Pack>-dry-run         print the UAT command only
#   cmake --build Build --target package-additional-maps     create + cook + tar the AdditionalMaps pack
#   cmake --build Build --target package-additional-maps-dry-run
#
# package-additional-maps builds the towns the default package does not cook
# (the list in CARLA_ADDITIONAL_MAPS_FILE) into one pack, written next to the
# package tarball. Like pack-<Pack> it needs the base release, so run the
# package target first; it does not depend on it to avoid a second cook.
#
# pack-<Pack> targets exist for every directory under Unreal/CarlaUnreal/Plugins/Packs
# that holds a carla-pack.json, plus any names listed in CARLA_CONTENT_PACKS. They
# cook against CARLA_CONTENT_PACKS_BASE (default: the base release the package
# target creates, Unreal/CarlaUnreal/Releases/<CARLA_BASE_RELEASE>), so run the
# package target first, or point CARLA_CONTENT_PACKS_BASE at a
# <release>-release-metadata.tar.gz downloaded with the binary.
#
# Included from the top-level CMakeLists.txt after Options/Common/Util; it only
# uses CARLA_WORKSPACE_PATH, CARLA_VERSION, UE_SYSTEM_NAME, CARLA_BASE_RELEASE and
# CARLA_UNREAL_ENGINE_PATH from there.

set (CARLA_CONTENT_PACKS_TOOL ${CARLA_WORKSPACE_PATH}/Util/ContentPacks/carla_pack.py)
set (CARLA_CONTENT_PACKS_TESTS ${CARLA_WORKSPACE_PATH}/Util/ContentPacks/test_carla_pack.py)
set (CARLA_CONTENT_PACKS_PROJECT ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/CarlaUnreal.uproject)
set (CARLA_CONTENT_PACKS_ROOT ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Plugins/Packs)
set (CARLA_CONTENT_PACKS_OUT ${CMAKE_BINARY_DIR}/Packs)

set (
  CARLA_CONTENT_PACKS
  ""
  CACHE STRING
  "Extra content pack names (semicolon-separated) to create pack-<Name> targets \
for, in addition to the packs found under Unreal/CarlaUnreal/Plugins/Packs."
)

set (
  CARLA_CONTENT_PACKS_BASE
  ""
  CACHE STRING
  "Base release for the pack-<Name> targets: a <release>-release-metadata.tar.gz or \
an extracted Releases/<release> directory. Empty means the release the package \
target writes to Unreal/CarlaUnreal/Releases/<CARLA_BASE_RELEASE>."
)

set (
  CARLA_CONTENT_PACKS_CONFIG
  Development
  CACHE STRING
  "Client configuration the pack cook is staged for (Development or Shipping)."
)

# No find_package(Python3) here: PythonAPI/CMakeLists.txt does the project's
# Python discovery with its own component set and FindPython caches the first
# call. Reuse its result when it already exists, else the shim's default.
if (NOT DEFINED CARLA_CONTENT_PACKS_PYTHON)
  if (DEFINED Python3_EXECUTABLE)
    set (CARLA_CONTENT_PACKS_PYTHON ${Python3_EXECUTABLE})
  else ()
    set (CARLA_CONTENT_PACKS_PYTHON python3)
  endif ()
endif ()

if ("${CARLA_CONTENT_PACKS_BASE}" STREQUAL "")
  if ("${CARLA_BASE_RELEASE}" STREQUAL "")
    set (CARLA_CONTENT_PACKS_BASE_PATH
      ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Releases/carla-${CARLA_VERSION}-${UE_SYSTEM_NAME})
  else ()
    set (CARLA_CONTENT_PACKS_BASE_PATH
      ${CARLA_WORKSPACE_PATH}/Unreal/CarlaUnreal/Releases/${CARLA_BASE_RELEASE})
  endif ()
else ()
  set (CARLA_CONTENT_PACKS_BASE_PATH ${CARLA_CONTENT_PACKS_BASE})
endif ()

carla_add_custom_target (
  content-packs-test
  "Run the carla-pack unit tests."
  COMMAND ${CARLA_CONTENT_PACKS_PYTHON} ${CARLA_CONTENT_PACKS_TESTS}
  USES_TERMINAL
  VERBATIM
)

function (carla_add_content_pack_target PACK_NAME)
  set (COMMON_ARGS
    ${CARLA_CONTENT_PACKS_TOOL}
    build ${PACK_NAME}
    --project ${CARLA_CONTENT_PACKS_PROJECT}
    --base ${CARLA_CONTENT_PACKS_BASE_PATH}
    --config ${CARLA_CONTENT_PACKS_CONFIG}
    --platform ${UE_SYSTEM_NAME}
    --out ${CARLA_CONTENT_PACKS_OUT}
  )
  if (NOT "${CARLA_UNREAL_ENGINE_PATH}" STREQUAL "")
    list (APPEND COMMON_ARGS --engine ${CARLA_UNREAL_ENGINE_PATH})
  endif ()
  carla_add_custom_target (
    pack-${PACK_NAME}
    "Cook content pack ${PACK_NAME} as DLC against the base release and tar it."
    COMMAND ${CARLA_CONTENT_PACKS_PYTHON} ${COMMON_ARGS}
    USES_TERMINAL
    VERBATIM
  )
  carla_add_custom_target (
    pack-${PACK_NAME}-dry-run
    "Print the UAT command 'pack-${PACK_NAME}' would run."
    COMMAND ${CARLA_CONTENT_PACKS_PYTHON} ${COMMON_ARGS} --dry-run
    USES_TERMINAL
    VERBATIM
  )
endfunction ()

set (CARLA_CONTENT_PACK_NAMES ${CARLA_CONTENT_PACKS})
# CONFIGURE_DEPENDS: a freshly `carla-pack init`-ed pack gets its target on the next build.
file (GLOB CARLA_CONTENT_PACK_MANIFESTS CONFIGURE_DEPENDS ${CARLA_CONTENT_PACKS_ROOT}/*/carla-pack.json)
foreach (MANIFEST ${CARLA_CONTENT_PACK_MANIFESTS})
  cmake_path (GET MANIFEST PARENT_PATH PACK_DIR)
  cmake_path (GET PACK_DIR FILENAME PACK_NAME)
  list (APPEND CARLA_CONTENT_PACK_NAMES ${PACK_NAME})
endforeach ()
if (CARLA_CONTENT_PACK_NAMES)
  list (REMOVE_DUPLICATES CARLA_CONTENT_PACK_NAMES)
endif ()
foreach (PACK_NAME ${CARLA_CONTENT_PACK_NAMES})
  carla_add_content_pack_target (${PACK_NAME})
endforeach ()

set (
  CARLA_ADDITIONAL_MAPS_FILE
  ${CARLA_WORKSPACE_PATH}/Util/ContentPacks/AdditionalMaps.json
  CACHE FILEPATH
  "JSON list of the maps package-additional-maps puts in the pack \
([\"Town01_Opt\", ...] or {\"maps\": [...]}); read at build time."
)

set (
  CARLA_ADDITIONAL_MAPS_PACK
  AdditionalMaps
  CACHE STRING
  "Name of the pack package-additional-maps creates (also its /<Name>/ mount point)."
)

set (CARLA_ADDITIONAL_MAPS_ARGS
  ${CARLA_CONTENT_PACKS_TOOL}
  create ${CARLA_ADDITIONAL_MAPS_PACK}
  --maps-file ${CARLA_ADDITIONAL_MAPS_FILE}
  --project ${CARLA_CONTENT_PACKS_PROJECT}
  --base ${CARLA_CONTENT_PACKS_BASE_PATH}
  # The default package (the one deployed) is Shipping.
  --config Shipping
  --platform ${UE_SYSTEM_NAME}
  --pack-version ${CARLA_VERSION}
  --description "CARLA towns not shipped in the default package"
  --out ${CMAKE_BINARY_DIR}/Package
)
if (NOT "${CARLA_UNREAL_ENGINE_PATH}" STREQUAL "")
  list (APPEND CARLA_ADDITIONAL_MAPS_ARGS --engine ${CARLA_UNREAL_ENGINE_PATH})
endif ()

carla_add_custom_target (
  package-additional-maps
  "Cook the maps of CARLA_ADDITIONAL_MAPS_FILE as the ${CARLA_ADDITIONAL_MAPS_PACK} content pack."
  COMMAND ${CARLA_CONTENT_PACKS_PYTHON} ${CARLA_ADDITIONAL_MAPS_ARGS}
  USES_TERMINAL
  VERBATIM
)

carla_add_custom_target (
  package-additional-maps-dry-run
  "Create the ${CARLA_ADDITIONAL_MAPS_PACK} pack sources and print the UAT command only."
  COMMAND ${CARLA_CONTENT_PACKS_PYTHON} ${CARLA_ADDITIONAL_MAPS_ARGS} --dry-run
  USES_TERMINAL
  VERBATIM
)
