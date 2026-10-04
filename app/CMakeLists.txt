cmake_minimum_required(VERSION 3.20.0)

# Register this application directory as a board root so the custom board in
# <app>/boards/our_board can be discovered without --board-dir.
# See Zephyr RTOS course, Lecture 5: Custom Boards.
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED)
project(my_zephyr_app)

target_sources(app PRIVATE src/main.cpp)
