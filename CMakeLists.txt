cmake_minimum_required(VERSION 3.21)

project(common_lib)

set(LIBRARY_NAME ${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES src/*.cpp )
file(GLOB HEADERS src/*.h )

add_library(${LIBRARY_NAME} STATIC ${SOURCES} ${HEADERS})

if(EXISTS ${CMAKE_SOURCE_DIR}/build/conan/conan_toolchain.cmake)
    include(${CMAKE_SOURCE_DIR}/build/conan/conan_toolchain.cmake)
endif()

#include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/include)
target_include_directories(${LIBRARY_NAME} 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)


#target_include_directories(${LIBRARY_NAME} PRIVATE ${INCLUDE_DIRS})
target_compile_definitions(${LIBRARY_NAME} PRIVATE ${COMMON_DEFINES})
target_link_libraries(${LIBRARY_NAME} PRIVATE ${LINK_LIBRARIES})

# Interface target so consumers can use target_link_libraries(foo PRIVATE sk::common)
add_library(sk_common INTERFACE)
target_include_directories(sk_common INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
add_library(sk::common ALIAS sk_common)

# Only add install/doc/test targets when this is the top-level project,
# not when consumed via FetchContent by another project.
if(PROJECT_IS_TOP_LEVEL)

    install(TARGETS ${LIBRARY_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/concurrent DESTINATION include)
    install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/containers DESTINATION include)

    find_package(Doxygen QUIET)
    if(DOXYGEN_FOUND)
        message(STATUS "Doxygen found, enabling documentation target.")
        set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/Doxyfile)
        set(DOXYGEN_OUT ${CMAKE_BINARY_DIR}/Doxyfile.out)
        add_custom_target(doc
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_IN}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
    else()
        message(STATUS "Doxygen not found, documentation target will not be available.")
    endif()

    include(GoogleTest)
    find_package(GTest QUIET)

    if(GTest_FOUND)
        file(GLOB TEST_SOURCES test/*.cpp)

        add_executable(concurrent_tests ${TEST_SOURCES})
        target_include_directories(concurrent_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${CMAKE_CURRENT_SOURCE_DIR}/src
        )

        target_link_libraries(concurrent_tests
            PRIVATE
                ${LIBRARY_NAME}
                GTest::gtest
                GTest::gtest_main
        )

        if(TARGET GTest::gtest)
            target_include_directories(concurrent_tests PRIVATE ${GTest_INCLUDE_DIRS})
        endif()

        include(GoogleTest)
        gtest_discover_tests(concurrent_tests)

        enable_testing()
    else()
        message(WARNING "GTest not found: unit tests will not be built")
    endif()

endif() # PROJECT_IS_TOP_LEVEL