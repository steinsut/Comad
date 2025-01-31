include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(LIBRARY_NAME "Comad" CACHE INTERNAL "")

set(COMAD_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(COMAD_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(COMAD_VERSION_PATCH ${PROJECT_VERSION_PATCH})

#install option
option(COMAD_DISABLE_INSTALL "Prevent CMake from installing Comad." OFF)

#comad options
option(COMAD_SKIP_UNKNOWN_FLAGS "Skips any passed flag that has not been declared by the command. Unknowns are considered an error if off." ON)
option(COMAD_SKIP_DUPE_OPTIONS "Skips any option that has been passed before. Duplicates are considered an error if off." ON)
option(COMAD_SKIP_UNKNOWN_OPTIONS "Skips any passed option that has not been declared by the command. Unknowns are considered an error if off." ON)
option(COMAD_ALLOW_PARTIAL_NUMBER_PARSING "Allow for only a part of a string to be parsed as a number. Such cases are considered an error if off." ON)
option(COMAD_SKIP_INVALID_VALUE_PARSE "Skips any value that cannot be parsed correctly. Such cases are considered an error if off." ON)

set(COMAD_FLAG_PREFIX "-f" CACHE STRING "Prefix used for flags in a command.")
set(COMAD_OPTION_PREFIX "--" CACHE STRING "Prefix used for options in a command.")
set(COMAD_OPTION_SHORT_PREFIX "-" CACHE STRING "Prefix used for short names of options in a command.")

set(COMAD_MAX_CSTR_LENGTH "65536" CACHE STRING "Max length for use in std::memchr for making string views from C strings.")

set(COMAD_NO_INPUT "-1" CACHE STRING "Error code for no input.")
set(COMAD_UNKNOWN_COMMAND "-2" CACHE STRING "Error code for unknown command.")
set(COMAD_UNKNOWN_FLAG "-3" CACHE STRING "Error code for unknown flag.")
set(COMAD_DUPE_OPTION "-4" CACHE STRING "Error code for duplicate option.")
set(COMAD_INVALID_VALUE_PARSE "-5" CACHE STRING "Error code for invalid value parses.")
set(COMAD_INVALID_OPTION_VALUE "-6" CACHE STRING "Error code for invalid option value.")
set(COMAD_UNKNOWN_OPTION "-7" CACHE STRING "Error code for unknown option.")
set(COMAD_MISSING_REQUIRED_OPTIONS "-8" CACHE STRING "Error code for missing required options.")

configure_file("ComadBuildOptions.h.in" "ComadBuildOptions.h")
configure_file("ComadReturnCodes.h.in" "ComadReturnCodes.h")
configure_file("ComadVersion.h.in" "ComadVersion.h")
configure_file("ComadVersion.cpp.in" "ComadVersion.cpp")

add_library(${LIBRARY_NAME} STATIC "${CMAKE_CURRENT_BINARY_DIR}/ComadVersion.cpp"
                                    "Command.cpp"
                                    "CommandHandler.cpp"
                                    "CommandNode.cpp"
                                    "ValueUtils.cpp" 
                                    "CommandLiterals.cpp" )

set_target_properties(${LIBRARY_NAME} PROPERTIES 
                        CXX_STANDARD 20
                        CXX_STANDARD_REQUIRED TRUE
                        CXX_EXTENSIONS OFF)

target_include_directories(${LIBRARY_NAME}
                           PUBLIC
                           "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}>"
                           "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")

write_basic_package_version_file("${CMAKE_CURRENT_BINARY_DIR}/ComadConfigVersion.cmake"
                                  VERSION ${PROJECT_VERSION}
                                  COMPATIBILITY AnyNewerVersion)


configure_package_config_file("${CMAKE_SOURCE_DIR}/cmake/ComadConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/ComadConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/share/Comad")

export(TARGETS Comad
       FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/ComadTargets.cmake"
       NAMESPACE Comad::)

if(NOT ${COMAD_DISABLE_INSTALL})
    install(TARGETS Comad
            EXPORT ComadTargets
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/ComadBuildOptions.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ComadReturnCodes.h"
            "${CMAKE_CURRENT_BINARY_DIR}/ComadVersion.h"
            "Comad.h"
            "Command.h"
            "Command.tcc" 
            "CommandHandler.h"
            "CommandHandler.tcc" 
            "CommandNode.h" 
            "CommandNode.tcc"
            "CommandLiterals.h"
            "CommandLiterals.tcc"
            "StringUtils.h"
            "StringUtils.tcc"
            "Value.h"
            "ValueUtils.h"
            "ValueUtils.tcc"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

    install(EXPORT ComadTargets
            FILE ComadTargets.cmake
            NAMESPACE Comad::
            DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Comad")

    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/ComadConfig.cmake"
            "${CMAKE_CURRENT_BINARY_DIR}/ComadConfigVersion.cmake"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/share/Comad")
endif()