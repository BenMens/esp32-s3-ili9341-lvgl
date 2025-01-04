set(srcs
    main.cpp
    wifi.cpp
    http-server.cpp
    spiffs.cpp
    mqtt-client.cpp
    weather-client.cpp
    lvgl-touch-xpt2046.cpp
    gui/clock-controller.cpp
    gui/wifi-controller.cpp
    gui/home-controller.cpp
    gui/energy-controller.cpp
    gui/energy-meter-controller.cpp
    gui/back-button-controller.cpp
    gui/weather-hour-controller.cpp
    gui/weather-controller.cpp
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
        lwip
        esp_http_server
        esp_http_client
        spiffs
        mqtt
        json
)

spiffs_create_partition_image(spiffs ../spiffs_source FLASH_IN_PROJECT)