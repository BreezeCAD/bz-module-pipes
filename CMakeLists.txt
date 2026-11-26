cmake_minimum_required(VERSION 3.20)
project(bz_module_pipes LANGUAGES CXX)

set(BREEZECAD_SOURCE_DIR "" CACHE PATH "BreezeCAD source checkout")
if(NOT EXISTS "${BREEZECAD_SOURCE_DIR}/src/modules/bz_module.h"
   OR NOT EXISTS "${BREEZECAD_SOURCE_DIR}/third_party/nlohmann/nlohmann/json.hpp")
    message(FATAL_ERROR "Set BREEZECAD_SOURCE_DIR to a BreezeCAD source checkout")
endif()

add_library(bz_pipes SHARED module.cpp sketch3d.cpp)
target_compile_features(bz_pipes PRIVATE cxx_std_20)
target_include_directories(bz_pipes PRIVATE
    "${BREEZECAD_SOURCE_DIR}/src"
    "${BREEZECAD_SOURCE_DIR}/third_party/nlohmann")
set_target_properties(bz_pipes PROPERTIES CXX_VISIBILITY_PRESET hidden)
if(MSVC)
    target_compile_options(bz_pipes PRIVATE /utf-8)
elseif(MINGW)
    set_target_properties(bz_pipes PROPERTIES PREFIX "")
    target_link_options(bz_pipes PRIVATE -static)
endif()

install(TARGETS bz_pipes RUNTIME DESTINATION . LIBRARY DESTINATION .)
install(FILES module.json DESTINATION .)

include(CTest)
if(BUILD_TESTING)
    add_executable(sketch3d_test tests/sketch3d_test.cpp sketch3d.cpp)
    target_compile_features(sketch3d_test PRIVATE cxx_std_20)
    target_include_directories(sketch3d_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
    add_test(NAME sketch3d COMMAND sketch3d_test)
endif()
