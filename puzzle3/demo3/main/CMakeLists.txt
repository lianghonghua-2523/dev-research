idf_component_register(SRCS "hello_world_main.c"
                            "../software/wifi/Wifi.c"
                            "../software/uart/my_uart.c"

                    INCLUDE_DIRS "."
                            "../software/wifi"
                            "../software/uart"
                            "../software"
                    )
