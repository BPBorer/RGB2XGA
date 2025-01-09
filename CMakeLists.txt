# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NEVER EDIT THE NEXT LINES for Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 1.5.1)
set(toolchainVersion 13_2_Rel1)
set(picotoolVersion 2.0.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(RGB2XGA C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(RGB2XGA RGB2XGA.cpp )

pico_set_program_name(RGB2XGA "RGB2XGA")
pico_set_program_version(RGB2XGA "0.1")

# Generate PIO header
#pico_generate_pio_header(RGB2XGA ${CMAKE_CURRENT_LIST_DIR}/blink.pio)
pico_generate_pio_header(RGB2XGA ${CMAKE_CURRENT_LIST_DIR}/XGAvSync.pio)
pico_generate_pio_header(RGB2XGA ${CMAKE_CURRENT_LIST_DIR}/XGAhSync.pio)
pico_generate_pio_header(RGB2XGA ${CMAKE_CURRENT_LIST_DIR}/XGAPixs.pio)
pico_generate_pio_header(RGB2XGA ${CMAKE_CURRENT_LIST_DIR}/RGB_PIn.pio)  

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(RGB2XGA 0)
pico_enable_stdio_usb(RGB2XGA 1)

# Add the standard library to the build
target_link_libraries(RGB2XGA
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(RGB2XGA PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/.. # for our common lwipopts or any other standard includes, if required
)

# Add any user requested libraries
target_link_libraries(RGB2XGA 
        hardware_pll
        hardware_clocks
        hardware_dma
        hardware_pio
        )

pico_add_extra_outputs(RGB2XGA)

