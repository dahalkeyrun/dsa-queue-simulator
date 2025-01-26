cmake_minimum_required(VERSION 3.15)
project(TrafficSimulator)

# SDL2 configuration (for simulator only)
set(SDL2_DIR ${CMAKE_SOURCE_DIR}/SDLTemplate/lib)
include_directories(${SDL2_DIR}/include/SDL2)

# Simulator (SDL GUI app)
add_executable(simulator simulator.c)
if(WIN32)
    target_link_libraries(simulator mingw32 SDL2main SDL2)
    target_link_directories(simulator PRIVATE ${SDL2_DIR})
endif()

# Traffic Generator (Pure Console App)
add_executable(traffic_generator traffic_generator.c)
if(WIN32)
    # For MinGW: Force console subsystem and entry point
    target_link_options(traffic_generator PRIVATE 
        "-Wl,-subsystem,console" 
        "-Wl,-entry,mainCRTStartup"
    )
endif()
