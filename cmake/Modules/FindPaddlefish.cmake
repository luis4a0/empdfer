# Find Paddlefish library. Based on CMake's FindJPEG.cmake.

find_path(PADDLEFISH_INCLUDE_DIR paddlefish/paddlefish.h)

set(paddlefish_names ${PADDLEFISH_NAMES} paddlefish paddlefish-static libpaddlefish libpaddlefish-static)
foreach(name ${paddlefish_names})
  list(APPEND paddlefish_names_debug "${name}d")
endforeach()

if(NOT PADDLEFISH_LIBRARY)
  find_library(PADDLEFISH_LIBRARY_RELEASE NAMES ${paddlefish_names} NAMES_PER_DIR)
  find_library(PADDLEFISH_LIBRARY_DEBUG NAMES ${paddlefish_names_debug} NAMES_PER_DIR)
  include(SelectLibraryConfigurations)
  select_library_configurations(PADDLEFISH)
  mark_as_advanced(PADDLEFISH_LIBRARY_RELEASE PADDLEFISH_LIBRARY_DEBUG)
endif()
unset(paddlefish_names)
unset(paddlefish_names_debug)

if(PADDLEFISH_INCLUDE_DIR)
  set(_PADDLEFISH_CONFIG_HEADERS "${PADDLEFISH_INCLUDE_DIR}/paddlefish.h")
  foreach (_PADDLEFISH_CONFIG_HEADER IN LISTS _PADDLEFISH_CONFIG_HEADERS)
    if (NOT EXISTS "${_PADDLEFISH_CONFIG_HEADER}")
      continue ()
    endif ()
    file(STRINGS "${_PADDLEFISH_CONFIG_HEADER}"
      paddlefish_lib_version REGEX "^#define[\t ]+PADDLEFISH_LIB_VERSION[\t ]+.*")

    if (NOT paddlefish_lib_version)
      continue ()
    endif ()

    string(REGEX REPLACE "^#define[\t ]+PADDLEFISH_LIB_VERSION[\t ]+([0-9]+).*"
      "\\1" PADDLEFISH_VERSION "${paddlefish_lib_version}")
    break ()
  endforeach ()
  unset(paddlefish_lib_version)
  unset(_PADDLEFISH_CONFIG_HEADER)
  unset(_PADDLEFISH_CONFIG_HEADERS)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Paddlefish
  REQUIRED_VARS PADDLEFISH_LIBRARY PADDLEFISH_INCLUDE_DIR
  VERSION_VAR PADDLEFISH_VERSION)

if(PADDLEFISH_FOUND)
  set(PADDLEFISH_LIBRARIES ${PADDLEFISH_LIBRARY})
  set(PADDLEFISH_INCLUDE_DIRS "${PADDLEFISH_INCLUDE_DIR}")

  if(NOT TARGET PADDLEFISH::PADDLEFISH)
    add_library(PADDLEFISH::PADDLEFISH UNKNOWN IMPORTED)
    if(PADDLEFISH_INCLUDE_DIRS)
      set_target_properties(PADDLEFISH::PADDLEFISH PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${PADDLEFISH_INCLUDE_DIRS}")
    endif()
    if(EXISTS "${PADDLEFISH_LIBRARY}")
      set_target_properties(PADDLEFISH::PADDLEFISH PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        IMPORTED_LOCATION "${PADDLEFISH_LIBRARY}")
    endif()
    if(EXISTS "${PADDLEFISH_LIBRARY_RELEASE}")
      set_property(TARGET PADDLEFISH::PADDLEFISH APPEND PROPERTY
        IMPORTED_CONFIGURATIONS RELEASE)
      set_target_properties(PADDLEFISH::PADDLEFISH PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
        IMPORTED_LOCATION_RELEASE "${PADDLEFISH_LIBRARY_RELEASE}")
    endif()
    if(EXISTS "${PADDLEFISH_LIBRARY_DEBUG}")
      set_property(TARGET PADDLEFISH::PADDLEFISH APPEND PROPERTY
        IMPORTED_CONFIGURATIONS DEBUG)
      set_target_properties(PADDLEFISH::PADDLEFISH PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX"
        IMPORTED_LOCATION_DEBUG "${PADDLEFISH_LIBRARY_DEBUG}")
    endif()
  endif()
endif()

mark_as_advanced(PADDLEFISH_LIBRARY PADDLEFISH_INCLUDE_DIR)
