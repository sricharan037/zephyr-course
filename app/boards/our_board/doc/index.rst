.. zephyr:board:: our_board

Overview
********

OUR BOARD is a custom board created with the "Copy/Rename" method described in
the Zephyr RTOS course, Lecture 5: Custom Boards.

It was derived by copying the in-tree ``esp32_devkitc`` board (ESP32-DevKitC,
ESP32 SoC) and renaming its files to ``our_board``. The hardware is therefore
identical to the ESP32-DevKitC; only the board identity and the default
configuration were changed.

ESP32 is a series of low cost, low power system on a chip microcontrollers
with integrated Wi-Fi & dual-mode Bluetooth. ESP32 is created and developed by
Espressif Systems.

Supported Features
==================

.. zephyr:board-supported-hw::

Building & Flashing
*******************

Build the :zephyr:code-sample:`hello_world` sample for this board with:

.. zephyr-app-commands::
   :zephyr-app: samples/hello_world
   :board: our_board/esp32/procpu
   :goals: build flash

References
**********

.. target-notes::

.. _`ESP32-DevKitC`: https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/index.html
