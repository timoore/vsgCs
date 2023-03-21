include(FindPackageHandleStandardArgs)

# XXX For now we are not going to try too hard to find include-only interface
# packages (e.g. GSL, which is not the GNU Scientific Library!) and
# assume that they will be found in CESIUMNATIVE_INCLUDE_DIR

#=============================================================================
# If the user has provided ``CESIUM_ROOT_DIR``, use it!  Choose items found
# at this location over system locations.
if( EXISTS "$ENV{CESIUM_ROOT_DIR}" )
  file( TO_CMAKE_PATH "$ENV{CESIUM_ROOT_DIR}" CESIUM_ROOT_DIR )
  set( CESIUM_ROOT_DIR "${CESIUM_ROOT_DIR}" CACHE PATH "Prefix for Cesium Native installation." )
endif()


find_path(SPDLOG_INCLUDE_DIR
  NAMES spdlog/logger.h
  HINTS ${CESIUM_ROOT_DIR}/include ${SPDLOG_INCLUDE_DIR}
)

find_library(SPDLOG_LIBRARY
  NAMES spdlog
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(SPDLOG_LIBRARY_DEBUG
  NAMES spdlogd spdlog
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_path(ASYNCPLUSPLUS_INCLUDE_DIR
  NAMES async++/ref_count.h
  HINTS ${CESIUM_ROOT_DIR}/include ${ASYNCPLUSPLUS_INCLUDE_DIR}
)

find_library(ASYNCPLUSPLUS_LIBRARY
  NAMES async++
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(ASYNCPLUSPLUS_LIBRARY_DEBUG
  NAMES async++d async++
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(SQLITE3_LIBRARY
  NAMES sqlite3
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(SQLITE3_LIBRARY_DEBUG
  NAMES sqlite3d sqlite3
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(S2GEOMETRY_LIBRARY
  NAME s2geometry
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(S2GEOMETRY_LIBRARY_DEBUG
  NAME s2geometryd s2geometry
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(MODP_B64_LIBRARY
  NAME modp_b64
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(MODP_B64_LIBRARY_DEBUG
  NAME modp_b64d modp_b64
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(DRACO_LIBRARY
  NAME draco
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(DRACO_LIBRARY_DEBUG
  NAME dracod draco
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(KTX_READ_LIBRARY
  NAME ktx_read
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(KTX_READ_LIBRARY_DEBUG
  NAME ktx_readd ktx_read
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(WEBPDECODER_LIBRARY
  NAME webpdecoder
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(WEBPDECODER_LIBRARY_DEBUG
  NAME webpdecoderd webpdecoder
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(TINYXML2_LIBRARY
  NAME tinyxml2
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(TINYXML2_LIBRARY_DEBUG
  NAME tinyxml2d tinyxml2
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(URIPARSER_LIBRARY
  NAME uriparser
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(URIPARSER_LIBRARY_DEBUG
  NAME uriparserd uriparser
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(TURBOJPEG_LIBRARY
  NAME turbojpeg
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(TURBOJPEG_LIBRARY_DEBUG
  NAME turbojpegd turbojpeg
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)


find_path(CESIUM_INCLUDE_DIR
  NAMES Cesium3DTilesSelection/Tileset.h
  HINTS ${CESIUM_ROOT_DIR}/include ${CESIUM_INCLUDEDIR}
)

find_library(CESIUM_UTILITY_LIBRARY
  NAMES CesiumUtility
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(CESIUM_UTILITY_LIBRARY_DEBUG
  NAMES CesiumUtilityd
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(CESIUM_JSONREADER_LIBRARY
  NAMES CesiumJsonReader
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(CESIUM_JSONREADER_LIBRARY_DEBUG
  NAMES CesiumJsonReaderd CesiumJsonReader
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(CESIUM_GLTFREADER_LIBRARY
  NAMES CesiumGltfReader
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(CESIUM_GLTFREADER_LIBRARY_DEBUG
  NAMES CesiumGltfReaderd CesiumGltfReader
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(CESIUM_GLTF_LIBRARY
  NAMES CesiumGltf
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(CESIUM_GLTF_LIBRARY_DEBUG
  NAMES CesiumGltfd CesiumGltf
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(CESIUM_GEOMETRY_LIBRARY
  NAMES CesiumGeometry
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(CESIUM_GEOMETRY_LIBRARY_DEBUG
  NAMES CesiumGeometryd CesiumGeometry
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(CESIUM_GEOSPATIAL_LIBRARY
  NAMES CesiumGeospatial
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(CESIUM_GEOSPATIAL_LIBRARY_DEBUG
  NAMES CesiumGeospatiald CesiumGeospatial
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(CESIUM_ASYNC_LIBRARY
  NAMES CesiumAsync
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(CESIUM_ASYNC_LIBRARY_DEBUG
  NAMES CesiumAsyncd CesiumAsync
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

find_library(CESIUM_3DTILESSELECTION_LIBRARY
  NAMES Cesium3DTilesSelection
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Release Debug)

find_library(CESIUM_3DTILESSELECTION_LIBRARY_DEBUG
  NAMES Cesium3DTilesSelectiond Cesium3DTilesSelection
  HINTS ${CESIUM_ROOT_DIR}/lib ${CESIUM_LIBDIR}
  PATH_SUFFIXES Debug)

set( CESIUM_INCLUDE_DIRS ${CESIUM_INCLUDE_DIR} )

#=============================================================================
# handle the QUIETLY and REQUIRED arguments and set CESIUM_FOUND to TRUE if all
# listed variables are TRUE
find_package_handle_standard_args(Cesium
  FOUND_VAR
  CESIUM_FOUND
  REQUIRED_VARS
  CESIUM_INCLUDE_DIR
  CESIUM_3DTILESSELECTION_LIBRARY
)

add_library(spdlog UNKNOWN IMPORTED)
add_library(async++ UNKNOWN IMPORTED)
add_library(sqlite3 UNKNOWN IMPORTED)
add_library(s2geometry UNKNOWN IMPORTED)
add_library(modp_b64 UNKNOWN IMPORTED)
# XXX to avoid another installed version of draco
add_library(draco STATIC IMPORTED)
add_library(ktx_read UNKNOWN IMPORTED)
add_library(webpdecoder UNKNOWN IMPORTED)
add_library(tinyxml2 UNKNOWN IMPORTED)
add_library(uriparser UNKNOWN IMPORTED)
add_library(turbojpeg UNKNOWN IMPORTED)

add_library(cesium::Utility UNKNOWN IMPORTED)
add_library(cesium::JsonReader UNKNOWN IMPORTED)
add_library(cesium::GltfReader UNKNOWN IMPORTED)
add_library(cesium::Gltf UNKNOWN IMPORTED)
add_library(cesium::Geometry UNKNOWN IMPORTED)
add_library(cesium::Geospatial UNKNOWN IMPORTED)
add_library(cesium::Async UNKNOWN IMPORTED)
add_library(cesium::3DTilesSelection UNKNOWN IMPORTED)

function(set_debug target debug_location)
  if(debug_location)
    set_target_properties(${target} PROPERTIES IMPORTED_LOCATION_DEBUG ${debug_location})
  endif()
endfunction()

set_target_properties(spdlog PROPERTIES
  IMPORTED_LOCATION "${SPDLOG_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIR}")

set_debug(spdlog "${SPDLOG_LIBRARY_DEBUG}")

set_target_properties(async++ PROPERTIES
  IMPORTED_LOCATION "${ASYNCPLUSPLUS_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${ASYNCPLUSPLUS_INCLUDE_DIR}")

set_debug(async++ "${ASYNCPLUSPLUS_LIBRARY_DEBUG}")

set_target_properties(sqlite3 PROPERTIES
  IMPORTED_LOCATION "${SQLITE3_LIBRARY}")

set_debug(sqlite3 "${SQLITE3_LIBRARY_DEBUG}")

set_target_properties(s2geometry PROPERTIES
  IMPORTED_LOCATION "${S2GEOMETRY_LIBRARY}")

set_debug(s2geometry "${S2GEOMETRY_LIBRARY_DEBUG}")

set_target_properties(modp_b64 PROPERTIES
  IMPORTED_LOCATION "${MODP_B64_LIBRARY}")

set_debug(modp_b64 "${MODP_B64_LIBRARY_DEBUG}")

set_target_properties(draco PROPERTIES
  IMPORTED_LOCATION "${DRACO_LIBRARY}")

set_debug(draco "${DRACO_LIBRARY_DEBUG}")

set_target_properties(ktx_read PROPERTIES
  IMPORTED_LOCATION "${KTX_READ_LIBRARY}")

set_debug(ktx_read "${KTX_READ_LIBRARY_DEBUG}")

set_target_properties(webpdecoder PROPERTIES
  IMPORTED_LOCATION "${WEBPDECODER_LIBRARY}")

set_debug(webpdecoder "${WEBPDECODER_LIBRARY_DEBUG}")

set_target_properties(tinyxml2 PROPERTIES
  IMPORTED_LOCATION "${TINYXML2_LIBRARY}")

set_debug(tinyxml2 "${TINYXML2_LIBRARY_DEBUG}")

set_target_properties(uriparser PROPERTIES
  IMPORTED_LOCATION "${URIPARSER_LIBRARY}")

set_debug(uriparser "${URIPARSER_LIBRARY_DEBUG}")

set_target_properties(turbojpeg PROPERTIES
  IMPORTED_LOCATION "${TURBOJPEG_LIBRARY}")

set_debug(turbojpeg "${TURBOJPEG_LIBRARY_DEBUG}")

set_target_properties(cesium::Utility PROPERTIES
  IMPORTED_LOCATION "${CESIUM_UTILITY_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CESIUM_INCLUDE_DIR}"
  INTERFACE_LINK_LIBRARIES uriparser)

set_debug(cesium::Utility "${CESIUM_UTILITY_LIBRARY_DEBUG}")

target_link_libraries(cesium::JsonReader
  INTERFACE
  cesium::Utility)

set_target_properties(cesium::JsonReader PROPERTIES
  IMPORTED_LOCATION "${CESIUM_JSONREADER_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CESIUM_INCLUDE_DIR}")

set_debug(cesium::JsonReader "${CESIUM_JSONREADER_LIBRARY_DEBUG}")

target_link_libraries(cesium::Async
  INTERFACE
  "cesium::Utility;async++;sqlite3;spdlog")

set_target_properties(cesium::Async PROPERTIES
  IMPORTED_LOCATION "${CESIUM_ASYNC_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CESIUM_INCLUDE_DIR}")

set_debug(cesium::Async "${CESIUM_ASYNC_LIBRARY_DEBUG}")

set_target_properties(cesium::JsonReader PROPERTIES
  IMPORTED_LOCATION "${CESIUM_JSONREADER_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CESIUM_INCLUDE_DIR}")

set_debug(cesium::JsonReader "${CESIUM_JSONREADER_LIBRARY_DEBUG}")

target_link_libraries(cesium::Geometry
  INTERFACE
  cesium::Utility)

set_target_properties(cesium::Geometry PROPERTIES
  IMPORTED_LOCATION "${CESIUM_GEOMETRY_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CESIUM_INCLUDE_DIR}")

set_debug(cesium::Geometry "${CESIUM_GEOMETRY_LIBRARY_DEBUG}")

target_link_libraries(cesium::Geospatial
  INTERFACE
  cesium::Geometry
  cesium::Utility
  s2geometry)

set_target_properties(cesium::Geospatial PROPERTIES
  IMPORTED_LOCATION "${CESIUM_GEOSPATIAL_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CESIUM_INCLUDE_DIR}")

set_debug(cesium::Geospatial "${CESIUM_GEOSPATIAL_LIBRARY_DEBUG}")

target_link_libraries(cesium::GltfReader
  INTERFACE
  cesium::Async
  cesium::Gltf
  cesium::JsonReader
  modp_b64
  draco
  ktx_read
  webpdecoder
  turbojpeg)

set_target_properties(cesium::GltfReader PROPERTIES
  IMPORTED_LOCATION "${CESIUM_GLTFREADER_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CESIUM_INCLUDE_DIR}")

set_debug(cesium::GltfReader "${CESIUM_GLTFREADER_LIBRARY_DEBUG}")

target_link_libraries(cesium::Gltf
  INTERFACE
  cesium::Utility)

set_target_properties(cesium::Gltf PROPERTIES
  IMPORTED_LOCATION "${CESIUM_GLTF_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CESIUM_INCLUDE_DIR}")

set_debug(cesium::Gltf "${CESIUM_GLTF_LIBRARY_DEBUG}")

target_link_libraries(cesium::3DTilesSelection
  INTERFACE
  cesium::Async
  cesium::Geospatial
  cesium::Geometry
  cesium::Gltf
  cesium::GltfReader
  cesium::Utility
  spdlog

  tinyxml2
  uriparser)

target_compile_definitions(cesium::3DTilesSelection
  INTERFACE
            GLM_FORCE_XYZW_ONLY # Disable .rgba and .stpq to make it easier to view values from debugger
            GLM_FORCE_EXPLICIT_CTOR # Disallow implicit conversions between dvec3 <-> dvec4, dvec3 <-> fvec3, etc
            GLM_FORCE_SIZE_T_LENGTH # Make vec.length() and vec[idx] use size_t instead of int
          )
          
set_target_properties(cesium::3DTilesSelection
  PROPERTIES
  IMPORTED_LOCATION "${CESIUM_3DTILESSELECTION_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${CESIUM_INCLUDE_DIR}"
)

set_debug(cesium::3DTilesSelection "${CESIUM_3DTILESSELECTION_LIBRARY_DEBUG}")

