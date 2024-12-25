set(srcs
    main.cpp
    wifi.cpp
    lvgl-touch-xpt2046.cpp
    gui/clock-controller.cpp
    gui/wifi-controller.cpp
    gui/home-controller.cpp
    gui/energy-controller.cpp
    gui/back-button-controller.cpp
    lvgl-mvc/events.cpp
    lvgl-mvc/view-controller.cpp
    lvgl-mvc/navigation.cpp
    model/wifi-model.cpp
)

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "."
    REQUIRES
        esp_hw_support
        esp_event
        esp_lcd
        esp_adc
        neopixel
        display
        lvgl
        esp_wifi
        nvs_flash
        wifi_provisioning
)