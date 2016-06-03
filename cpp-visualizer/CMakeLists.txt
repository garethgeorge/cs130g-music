cmake_minimum_required(VERSION 2.6)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

project (SfmlTest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("${CMAKE_SOURCE_DIR}")
link_directories("${PROJECT_BINARY_DIR}")

# set(EXECUTABLE_NAME "sfmltest")
add_executable("wavplayer" wavplayer/wavplayer.cpp)
add_executable("micvis" micvisualizer/main.cpp)

# Detect and add SFML
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
#Find any version 2.X of SFML
#See the FindSFML.cmake file for additional details and instructions
find_package(SFML 2 REQUIRED system window graphics network audio)
if(SFML_FOUND)
  include_directories(${SFML_INCLUDE_DIR})
  target_link_libraries(wavplayer ${SFML_LIBRARIES})
  target_link_libraries(micvis ${SFML_LIBRARIES})
endif()

# Linking with boost
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.34.0 REQUIRED system filesystem program_options)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(wavplayer ${Boost_LIBRARIES})
  target_link_libraries(micvis ${Boost_LIBRARIES})
endif()
