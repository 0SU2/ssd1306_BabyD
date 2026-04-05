#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "esp_log.h"

#include "driver/i2c_master.h"
#include "driver/i2c_types.h"

#include "freertos/idf_additions.h"
#include "freertos/task.h"

#include "font.h"

// Program types
#define BLINK_LED                      14
#define ADC_PIN                        33
#define BUTTON                         12
#define SSD1306_I2C_ADD                0x3C
#define SSD1306_LCD_PIXEL_CLOCK_HZ    (400 * 1000)
#define SSD1306_PIN_NUM_RST           -1
#define SSD1306_LCD_H_RES              64
#define SSD1306_LCD_W_RES              128
#define SSD1306_LCD_FULL_RES           (SSD1306_LCD_H_RES * SSD1306_LCD_W_RES / 8)
#define SSD1306_TIMEOUT                100
#define SSD1306_FPS                    60

// Bit number used to represent command and parameter
#define SSD1306_LCD_CMD_BITS           8
#define SSD1306_LCD_PARAM_BITS         8

// Control byte
#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14
#define OLED_CMD_COM_SCAN_DIRECTION_NORMAL 0xC0
#define OLED_CMD_SEGMENT_REMAP_LEFT_TO_RIGHT 0xA0

#define ssd1306_swap_coordenates(x,y) { uint8_t t = x; x = y; y = t; }

typedef struct {
    i2c_master_dev_handle_t dev_handle_t;
    int height, width;
    uint8_t page;
    uint8_t frame_buffer[SSD1306_LCD_FULL_RES];
} ssd1306_t;

void ssd1306_draw_text(ssd1306_t *ssd1306_config, int x, int y, int size, char *text);

void draw_bitmap(ssd1306_t *ssd1306_config, int x, int y, int width, int height, uint8_t *bitmap);

void ssd1306_draw_pixel(ssd1306_t *ssd1306_config, int x, int y);

void ssd1306_draw_parallel_line(ssd1306_t *ssd1306_config, int x, int y, int width);

void ssd1306_draw_line(ssd1306_t *ssd1306_config, int x_1, int y_1, int x_2, int y_2);

void ssd1306_draw_h_line(ssd1306_t *ssd1306_config, int x, int y, int w);

void ssd1306_draw_v_line(ssd1306_t *ssd1306_config, int x, int y, int h);

void ssd1306_draw_circle(ssd1306_t *ssd1306_config, int x, int y, int radius);

void ssd1306_draw_fill_rectangle(ssd1306_t *ssd1306_config, int x, int y, int width, int height);

void ssd1306_draw_rectangle(ssd1306_t *ssd1306_config, int x, int y, int width, int height);

void ssd1306_clear(ssd1306_t *ssd1306_config);

void ssd1306_fill_buff(ssd1306_t *ssd1306_config);

esp_err_t ssd1306_switch(ssd1306_t *ssd1306_c, bool state);

void ssd1306_send_buff(ssd1306_t *ssd1306_config);

void ssd1306_send_buff_to_page(ssd1306_t *ssd1306_config, uint8_t column, uint8_t page);

esp_err_t ssd1306_init(i2c_master_bus_handle_t *bus_handle_connect ,ssd1306_t *ssd1306_config, i2c_device_config_t *dev_config);
