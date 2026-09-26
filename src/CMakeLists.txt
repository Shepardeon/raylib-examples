include(FetchContent)

file(GLOB_RECURSE examples_src CONFIGURE_DEPENDS "*.h" "*.c")

# Paramètres pour raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(CUSTOMIZE_BUILD OFF CACHE BOOL "" FORCE)

FetchContent_Declare(raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 6.0
  GIT_SHALLOW TRUE
  GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

add_executable(examples ${examples_src})

target_include_directories(examples
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(examples
  PRIVATE
    raylib
)

install(TARGETS examples
  RUNTIME DESTINATION bin
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/
  DESTINATION include
)