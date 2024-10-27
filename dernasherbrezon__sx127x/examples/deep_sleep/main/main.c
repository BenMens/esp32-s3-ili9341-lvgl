#include <driver/gpio.h>
#include <driver/rtc_io.h>
#include <driver/spi_common.h>
#include <driver/spi_master.h>
#include <esp_intr_alloc.h>
#include <esp_log.h>
#include <esp_sleep.h>
#include <freertos/task.h>
#include <sx127x.h>
#include <inttypes.h>
#include <ssd1306.h>
#include <stdio.h>
#include <rom/gpio.h>
//#include <hal/gpio_hal.h>
#include <driver/rtc_io.h>

#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define CONFIG_PIN_OLED_SDL 22
#define CONFIG_PIN_OLED_SDA 21

#define I2C_MASTER_NUM I2C_NUM_1    /*!&lt; I2C port number for master dev */
#define I2C_MASTER_FREQ_HZ 100000   /*!&lt; I2C master clock frequency */

sx127x *device = NULL;
static const char *TAG = "sx127x";

void pin_mode(int pin) {
  gpio_config_t conf = {
      .pin_bit_mask = (1ULL << pin),                 /*!< GPIO pin: set with bit mask, each bit maps to a GPIO */
      .mode = GPIO_MODE_DISABLE,                   /*!< GPIO mode: set input/output mode                     */
      .pull_up_en = GPIO_PULLUP_DISABLE,           /*!< GPIO pull-up                                         */
      .pull_down_en = GPIO_PULLDOWN_DISABLE,       /*!< GPIO pull-down                                       */
      .intr_type = GPIO_INTR_DISABLE  /*!< GPIO interrupt type - previously set                 */
  };
  conf.mode = GPIO_MODE_INPUT;
  ESP_ERROR_CHECK(gpio_config(&conf));
}

void app_main() {
  ESP_LOGI(TAG, "starting up");
//  gpio_hold_dis((gpio_num_t) 18);
//  gpio_hold_dis((gpio_num_t) 21);
//
  spi_bus_config_t config = {
      .mosi_io_num = 27,
      .miso_io_num = 19,
      .sclk_io_num = 5,
      .quadwp_io_num = -1,
      .quadhd_io_num = -1,
      .max_transfer_sz = 0,
  };
  ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &config, 1));
  spi_device_interface_config_t dev_cfg = {
      .clock_speed_hz = 3E6,
      .spics_io_num = 18,
      .queue_size = 16,
      .command_bits = 0,
      .address_bits = 8,
      .dummy_bits = 0,
      .mode = 0};
  spi_device_handle_t spi_device;
  ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &dev_cfg, &spi_device));
  ESP_ERROR_CHECK(sx127x_create(spi_device, &device));
  ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_SLEEP, SX127x_MODULATION_LORA, device));
  ESP_ERROR_CHECK(sx127x_set_opmod(SX127x_MODE_SLEEP, SX127x_MODULATION_LORA, device));

  uint8_t regs[0x80];
  sx127x_dump_registers(regs, device);
  printf("%d, %d\n", regs[0x40], regs[0x41]);

  sx127x_destroy(device);

  ESP_ERROR_CHECK(spi_bus_remove_device(spi_device));
  ESP_ERROR_CHECK(spi_bus_free(VSPI_HOST));

  gpio_matrix_out(18, 0x100, false, false);
  gpio_matrix_out(27, 0x100, false, false);
  gpio_matrix_in(19, 0x100, false);
  gpio_matrix_out(5, 0x100, false, false);

//  i2c_config_t conf;
//  conf.mode = I2C_MODE_MASTER;
//  conf.sda_io_num = (gpio_num_t) CONFIG_PIN_OLED_SDA;
//  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
//  conf.scl_io_num = (gpio_num_t) CONFIG_PIN_OLED_SDL;
//  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
//  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
//  conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;
//
//  ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
//  ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
//
//  ssd1306_handle_t ssd1306_dev = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);
//  ssd1306_delete(ssd1306_dev);

//  gpio_set_direction((gpio_num_t) 21, GPIO_MODE_OUTPUT);
//  gpio_set_level((gpio_num_t) 21, 1);
//  gpio_hold_en((gpio_num_t) 21);

//  gpio_set_direction((gpio_num_t) 18, GPIO_MODE_INPUT_OUTPUT);
//  gpio_set_pull_mode((gpio_num_t) 18, GPIO_PULLDOWN_ONLY);
//  gpio_set_level((gpio_num_t) 18, 1);
//  gpio_hold_en((gpio_num_t) 18);


//  printf("state: %d\n", gpio_get_level((gpio_num_t) 18));
//  gpio_set_pull_mode((gpio_num_t) 18, GPIO_PULLDOWN_ONLY);
//  gpio_set_level((gpio_num_t) 4, 0);
//  gpio_set_level((gpio_num_t) 5, 0);
//  gpio_set_level((gpio_num_t) 14, 0);
//  gpio_set_level((gpio_num_t) 15, 0);
//  gpio_set_level((gpio_num_t) 16, 0);
//  gpio_set_level((gpio_num_t) 17, 0);
//  gpio_set_level((gpio_num_t) 18, 1);
//
//  gpio_set_level((gpio_num_t) 19, 0);
//  gpio_set_level((gpio_num_t) 26, 0);
//  gpio_set_level((gpio_num_t) 27, 0);
//  gpio_hold_en((gpio_num_t) 18);

//  gpio_config();

//  pin_mode(4);
//  pin_mode(5);
//  pin_mode(14);
//  pin_mode(15);
//  pin_mode(16);
//  pin_mode(17);
//  pin_mode(18);

//  gpio_set_direction((gpio_num_t) 4, GPIO_MODE_INPUT);
//  gpio_set_direction((gpio_num_t) 5, GPIO_MODE_INPUT);

  rtc_gpio_set_direction((gpio_num_t)14, RTC_GPIO_MODE_INPUT_OUTPUT);
  rtc_gpio_set_level((gpio_num_t)14, 0);

//  gpio_set_direction((gpio_num_t) 14, GPIO_MODE_INPUT);
//  gpio_set_pull_mode((gpio_num_t) 14, GPIO_FLOATING);
//  printf("state: %d\n", gpio_get_level((gpio_num_t) 14));
//  gpio_hold_en((gpio_num_t) 14);
//  gpio_set_direction((gpio_num_t) 15, GPIO_MODE_INPUT);
//  gpio_set_direction((gpio_num_t) 16, GPIO_MODE_INPUT);
//  gpio_set_direction((gpio_num_t) 17, GPIO_MODE_INPUT);
//  gpio_set_direction((gpio_num_t) 18, GPIO_MODE_INPUT);
//  gpio_set_direction((gpio_num_t) 19, GPIO_MODE_INPUT);
//  gpio_set_direction((gpio_num_t) 26, GPIO_MODE_INPUT);
//  gpio_set_direction((gpio_num_t) 27, GPIO_MODE_INPUT);

//  gpio_deep_sleep_hold_en();

  ESP_LOGI(TAG, "final state");
//  gpio_dump_io_configuration(stdout, SOC_GPIO_VALID_GPIO_MASK);
//  gpio_
//  gpio_deep_sleep_hold_en();

  esp_sleep_enable_timer_wakeup(6 * 1000 * (uint64_t) 1000);
  esp_deep_sleep_start();
}
