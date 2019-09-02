cmake_minimum_required(VERSION 3.4.0)
set(this SimpleMDSim)
project(${this} LANGUAGES C CXX VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED OM)

# Set path for include modules like package finds
list(APPEND CMAKE_MODULE_PATH "/usr/local/share/cmake-3.13/modules")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake-module")
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DLIBUS_NO_SSL -DUWS_NO_ZLIB -flto -ggdb -O0 -std=c++17")
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DLIBUS_NO_SSL -DUWS_NO_ZLIB -flto -ggdb -O0 -std=c++17")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DLIBUS_NO_SSL -DUWS_NO_ZLIB -flto -ggdb -O0")

# Setup Catch2 (see install instructions)
find_package(Catch2 REQUIRED)
include(CTest)
include(Catch)
enable_testing()

# Setup fmt 
find_package(fmt)
## to use fmt in the link line of a target `target_link_libraries(<your-target> fmt::fmt)`

# Setup JSON
find_package(nlohmann_json 3.2.0 REQUIRED)
# yo use target_link_libraries(foo PRIVATE nlohmann_json::nlohmann_json)

find_program(Docker_EXECUTABLE docker)
if(NOT Docker_EXECUTABLE)
    message(FATAL_ERROR "Cannot find the docker executable!")
endif()

if(DEFINED ENV{"THIRDPARTY"})
    set (DGR_THIRDPARTY_FOLDER $ENV{"THIRDPARTY"})
endif()

if(THIRDPARTY_FOLDER)
    message("Using ${THIRDPARTY} from ENV" )
else()
    message("Variable THIRDPARTY DOES NOT exist, setting default value")
    set(THIRDPARTY_FOLDER "$ENV{HOME}/code/thirdparty")
endif()

include(EnableProfiling)
globally_enable_profiling()

include_directories(${THIRDPARTY_FOLDER}/simplx/include)
message("SIMPLX Include=${THIRDPARTY_FOLDER}/simplx/include")

include_directories(${THIRDPARTY_FOLDER}/uWebSockets/src)
include_directories(${THIRDPARTY_FOLDER}/uWebSockets/uSockets/src)
add_subdirectory(${THIRDPARTY_FOLDER}/uWebSockets ./build)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)

include_directories(${SRC_DIR})

find_package (SQLite3 REQUIRED)
    
file(GLOB testfiles "test/unit-*.cpp")
add_executable(unittests test/testmain.cpp ${testfiles})
target_link_libraries(unittests Catch2::Catch2 fmt ${SQLite3_LIBRARY})
message("CXX_FLAGS=${CMAKE_CXX_FLAGS}")
catch_discover_tests(unittests)

set(web_src ${SRC_DIR}/shortloop.cpp)

add_executable(smdweb ${SRC_DIR}/webapp_main.cpp ${web_src})
target_include_directories(smdweb PUBLIC ${SQLITE3_INCLUDE_DIRS})
target_link_libraries(smdweb fmt uSockets uWebSockets pthread ${SQLite3_LIBRARY})

add_custom_target(buildContainer
    COMMAND ${Docker_EXECUTABLE} build --tag nhorlock/md-simulator:1.0 --file
        ${CMAKE_CURRENT_LIST_DIR}/Dockerfile ${CMAKE_CURRENT_LIST_DIR}
)