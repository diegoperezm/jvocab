cmake_minimum_required(VERSION 3.16)
project(main C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# External dependencies directory
set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

# Unity testing framework
set(UNITY_ROOT "${CMAKE_SOURCE_DIR}" CACHE PATH "Unity root directory")
set(UNITY_SRC "${UNITY_ROOT}/unity/unity.c")

# Raylib configuration
set(RAYLIB_DIR "${EXTERNAL_DIR}/raylib")
set(RAYLIB_LIB "${RAYLIB_DIR}/raylib/libraylib.a")

# Libxml2 configuration
set(LIBXML2_VERSION "2.12.7")
set(LIBXML2_TARBALL "libxml2-${LIBXML2_VERSION}.tar.xz")
set(LIBXML2_URL "https://download.gnome.org/sources/libxml2/2.12/${LIBXML2_TARBALL}")
set(LIBXML2_SRC_DIR "${EXTERNAL_DIR}/libxml2-${LIBXML2_VERSION}")
set(LIBXML2_LIB "${LIBXML2_SRC_DIR}/.libs/libxml2.a")

# Source files
file(GLOB SRCS "src/*.c")
list(REMOVE_ITEM SRCS "${CMAKE_SOURCE_DIR}/src/main.c" "${CMAKE_SOURCE_DIR}/src/raygui_impl.c")
set(MAIN_SRC "src/main.c")
set(RAYGUI_IMPL_SRC "src/raygui_impl.c")

# Header files
file(GLOB HEADERS "include/*.h")

# Test files
file(GLOB TEST_SRCS "tests/Test_*.c")

# Include directories
set(INCLUDE_DIRS
        ${EXTERNAL_DIR}
        ${RAYLIB_DIR}/src
        include
        src
        ${UNITY_ROOT}/unity
        ${LIBXML2_SRC_DIR}/include
)

# Common compiler flags
set(COMMON_FLAGS
        -Wall -Werror -Wextra -Wpedantic -Wpointer-arith -Wcast-align
        -Wwrite-strings -Wswitch-default -Wunreachable-code -Winit-self
        -Wmissing-field-initializers -Wno-unknown-pragmas -Wstrict-prototypes
        -Wundef -Wold-style-definition -Wno-misleading-indentation
)

# Build type specific flags
set(CMAKE_C_FLAGS_DEBUG "-g3 -DDEBUG -fsanitize=address,undefined -fno-omit-frame-pointer")
set(CMAKE_C_FLAGS_RELEASE "-O2 -DNDEBUG -march=native")
set(CMAKE_C_FLAGS_VALGRIND "-g3 -O0 -DDEBUG")

# Link flags for debug build
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address,undefined")

# Platform-specific libraries
if(APPLE)
    set(PLATFORM_LIBS
            "-framework OpenGL"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework CoreVideo"
            "-framework CoreAudio"
            "-framework AudioToolbox"
    )
else()
    set(PLATFORM_LIBS dl rt X11)
endif()

# External library targets
add_custom_target(fetch_raylib
        COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_DIR}
        COMMAND test -d ${RAYLIB_DIR} || git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git ${RAYLIB_DIR}
        COMMAND cd ${RAYLIB_DIR} && test -f libraylib.a || (cmake -B. -DBUILD_EXAMPLES=OFF -DBUILD_GAMES=OFF && make)
        COMMENT "Fetching and building Raylib"
)

add_custom_target(fetch_libxml2
        COMMAND ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_DIR}
        COMMAND test -f ${EXTERNAL_DIR}/${LIBXML2_TARBALL} || curl -L ${LIBXML2_URL} -o ${EXTERNAL_DIR}/${LIBXML2_TARBALL}
        COMMAND test -d ${LIBXML2_SRC_DIR} || (cd ${EXTERNAL_DIR} && tar xf ${LIBXML2_TARBALL})
        COMMAND test -f ${LIBXML2_LIB} || (cd ${LIBXML2_SRC_DIR} && ./configure --with-minimum --with-html --with-xpath --with-xptr --disable-shared --without-python && make)
        COMMENT "Fetching and building libxml2"
)

# Import external libraries
add_library(raylib_imported STATIC IMPORTED)
set_target_properties(raylib_imported PROPERTIES IMPORTED_LOCATION ${RAYLIB_LIB})
add_dependencies(raylib_imported fetch_raylib)

add_library(libxml2_imported STATIC IMPORTED)
set_target_properties(libxml2_imported PROPERTIES IMPORTED_LOCATION ${LIBXML2_LIB})
add_dependencies(libxml2_imported fetch_libxml2)

# RayGUI implementation library
add_library(raygui_impl STATIC ${RAYGUI_IMPL_SRC})
target_include_directories(raygui_impl SYSTEM PRIVATE ${EXTERNAL_DIR} ${RAYLIB_DIR}/src)
target_compile_options(raygui_impl PRIVATE -w -O2 -Wno-error -Wno-incompatible-pointer-types-discards-qualifiers)
add_dependencies(raygui_impl fetch_raylib)

# Main executable
add_executable(main ${MAIN_SRC} ${SRCS})
target_include_directories(main PRIVATE ${INCLUDE_DIRS})
target_compile_options(main PRIVATE ${COMMON_FLAGS})
target_link_libraries(main
        raylib_imported
        libxml2_imported
        raygui_impl
        m
        pthread
        z
        lzma
        iconv
        ${PLATFORM_LIBS}
)
add_dependencies(main fetch_raylib fetch_libxml2)

# Create build type specific executables
add_custom_target(debug_build
        COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_SOURCE_DIR}/debug
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_SOURCE_DIR}/debug
        COMMENT "Building debug version"
)

add_custom_target(release_build
        COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_SOURCE_DIR}/release
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_SOURCE_DIR}/release
        COMMENT "Building release version"
)

add_custom_target(valgrind_build
        COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Valgrind -S ${CMAKE_SOURCE_DIR} -B ${CMAKE_SOURCE_DIR}/valgrind
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_SOURCE_DIR}/valgrind
        COMMENT "Building valgrind version"
)

# Testing
if(TEST_SRCS)
    enable_testing()

    foreach(test_file ${TEST_SRCS})
        get_filename_component(test_name ${test_file} NAME_WE)
        string(REGEX REPLACE "Test_" "" test_name ${test_name})

        # Find corresponding source file
        set(test_src_file "src/${test_name}.c")
        if(EXISTS "${CMAKE_SOURCE_DIR}/${test_src_file}")
            add_executable(test_${test_name}
                    ${test_file}
                    ${test_src_file}
                    ${UNITY_SRC}
            )
            target_include_directories(test_${test_name} PRIVATE ${INCLUDE_DIRS})
            target_compile_options(test_${test_name} PRIVATE ${COMMON_FLAGS} -DTESTING)
            target_link_libraries(test_${test_name} m pthread ${PLATFORM_LIBS})

            add_test(NAME ${test_name} COMMAND test_${test_name})
        endif()
    endforeach()

    # Custom test target
    add_custom_target(run_tests
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMENT "Running all tests"
    )
endif()

# Valgrind target
add_custom_target(valgrind_run
        COMMAND valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all
        --track-origins=yes --track-fds=yes --trace-children=yes
        --num-callers=50 ${CMAKE_BINARY_DIR}/main
        DEPENDS main
        COMMENT "Running valgrind on main executable"
)

# Code formatting and analysis
find_program(CLANG_FORMAT clang-format)
if(CLANG_FORMAT)
    add_custom_target(format
            COMMAND ${CLANG_FORMAT} -i ${SRCS} ${MAIN_SRC}
            COMMENT "Formatting source code"
    )
endif()

find_program(CLANG_TIDY clang-tidy)
if(CLANG_TIDY)
    add_custom_target(tidy
            COMMAND ${CLANG_TIDY} -header-filter=.* ${SRCS} -- ${COMMON_FLAGS} -I${CMAKE_SOURCE_DIR}/include
            COMMENT "Running clang-tidy"
    )
endif()

find_program(CPPCHECK cppcheck)
if(CPPCHECK)
    add_custom_target(cppcheck
            COMMAND ${CPPCHECK} --std=c99 --enable=all --suppress=missingIncludeSystem
            --inconclusive --suppressions-list=${CMAKE_SOURCE_DIR}/cppcheck.suppressions
            --inline-suppr --error-exitcode=1 --library=posix
            --rule-file=${CMAKE_SOURCE_DIR}/cppcheck-rules.xml
            -I${CMAKE_SOURCE_DIR}/include ${SRCS}
            COMMENT "Running cppcheck"
    )
endif()

add_custom_target(check)
if(CLANG_TIDY)
    add_dependencies(check tidy)
endif()
if(CPPCHECK)
    add_dependencies(check cppcheck)
endif()

# CTags target
find_program(CTAGS ctags)
if(CTAGS)
    add_custom_target(ctags
            COMMAND ${CTAGS} -R --languages=C --extras=+q --c-kinds=+fps --fields=+iaS
            -f tags ${RAYLIB_DIR}/src/ ./src
            COMMAND ${CTAGS} -a --language-force=C --extras=+q --c-kinds=+fps --fields=+iaS
            -f tags ${EXTERNAL_DIR}/raygui.h || echo "Warning: raygui.h skipped"
            COMMENT "Generating ctags"
    )
endif()

# Clean targets
add_custom_target(clean_all
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/debug
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/release
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_SOURCE_DIR}/valgrind
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${RAYLIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${LIBXML2_SRC_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove ${EXTERNAL_DIR}/${LIBXML2_TARBALL}
        COMMENT "Cleaning all build artifacts and external dependencies"
)

# Show help target
add_custom_target(show_help
        COMMAND ${CMAKE_COMMAND} -E echo "Available targets:"
        COMMAND ${CMAKE_COMMAND} -E echo "  main (default) - Build main executable"
        COMMAND ${CMAKE_COMMAND} -E echo "  debug_build - Build debug version"
        COMMAND ${CMAKE_COMMAND} -E echo "  release_build - Build release version"
        COMMAND ${CMAKE_COMMAND} -E echo "  valgrind_build - Build valgrind version"
        COMMAND ${CMAKE_COMMAND} -E echo "  run_tests - Run all tests"
        COMMAND ${CMAKE_COMMAND} -E echo "  valgrind_run - Run with valgrind"
        COMMAND ${CMAKE_COMMAND} -E echo "  format - Format source code"
        COMMAND ${CMAKE_COMMAND} -E echo "  tidy - Run clang-tidy"
        COMMAND ${CMAKE_COMMAND} -E echo "  cppcheck - Run cppcheck"
        COMMAND ${CMAKE_COMMAND} -E echo "  check - Run tidy and cppcheck"
        COMMAND ${CMAKE_COMMAND} -E echo "  ctags - Generate ctags"
        COMMAND ${CMAKE_COMMAND} -E echo "  clean_all - Clean everything"
        COMMAND ${CMAKE_COMMAND} -E echo "  show_help - Show this help"
        COMMENT "Showing available targets"
)
