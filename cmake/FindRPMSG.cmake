#
# Finds the TI TDA4 RPMSG framework includes and libraries
#
#  RPMSG_FOUND     - True if RPMSG framework was found
#  RPMSG::RPMSG    - RPMSG imported target
#

find_path(RPMSG_INCLUDE_DIR NAMES ti_rpmsg_char.h)

find_library(RPMSG_LIBRARY ti_rpmsg_char)

set(RPMSG_INCLUDE_DIRS
  ${RPMSG_INCLUDE_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args("RPMSG" DEFAULT_MSG RPMSG_INCLUDE_DIRS RPMSG_LIBRARY)

if(RPMSG_FOUND)
  if (NOT TARGET RPMSG::RPMSG)
    add_library(RPMSG::RPMSG UNKNOWN IMPORTED)

    set_target_properties(RPMSG::RPMSG PROPERTIES
      IMPORTED_LOCATION "${RPMSG_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${RPMSG_INCLUDE_DIRS}"
    )
  endif()
endif()

mark_as_advanced(RPMSG_INCLUDE_DIRS RPMSG_LIBRARY)
