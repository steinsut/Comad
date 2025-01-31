# CMakeList.txt : CMake project for Comad, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.13)
cmake_policy(SET CMP0076 NEW)

message("Setting up project build...")

set(PROJECT_NAME "Comad")
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 8)
set(PROJECT_VERSION_PATCH 0)
set(PROJECT_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (${PROJECT_NAME} VERSION ${PROJECT_VERSION} LANGUAGES CXX)

add_subdirectory("src")
add_subdirectory("tests")
