cmake_minimum_required(VERSION 3.20.0)

# Register this application directory as a board root so the custom board in
# <app>/boards/our_board can be discovered without --board-dir.
# See Zephyr RTOS course, Lecture 5: Custom Boards.
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# Pick up the out-of-tree LED sensor driver module that ships with this app.
# See Zephyr RTOS course, Lecture 6: Zephyr Driver Development.
list(APPEND EXTRA_ZEPHYR_MODULES ${CMAKE_CURRENT_SOURCE_DIR}/drivers/my_driver_module)

find_package(Zephyr REQUIRED)
project(my_zephyr_app)

target_sources(app PRIVATE src/main.c)
