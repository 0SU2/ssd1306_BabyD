#include "ssd1306_babyD.h"

void draw_bitmap(ssd1306_t *ssd1306_config, int x, int y, int width, int height, uint8_t *bitmap)
{
    int size = width * height / 8;
    if (size > SSD1306_LCD_FULL_RES) {
        return;
    }
    int origin_x = x;
    int origin_y = y;
    for (int s = 0; s < size; s++) {
        ssd1306_config->frame_buffer[x + ( y / 8 ) * SSD1306_LCD_W_RES] |= bitmap[s];
        x++;
        if (( x - origin_x ) == width) {
            y+=8;
            x = origin_x;
        }
        if (( y - origin_y ) == height) {
            return;
        }
    }
}

void ssd1306_draw_text(ssd1306_t *ssd1306_config, int x, int y, int size, char *text)
{
    int pos_x = x;
    int pos_y = y;

    // TO-DO: add more font sizes

    for (int i = 0; i< strlen(text); i++) {
        if (pos_x > SSD1306_LCD_W_RES) {
            pos_x = 0;
            pos_y ++;
        }
        if (pos_y > SSD1306_LCD_H_RES) {
            return;
        }
        for (int b = 0; b < 8; b++) {
            ssd1306_config->frame_buffer[pos_x + ( pos_y / 8 ) * SSD1306_LCD_W_RES] |= font8x8_basic_tr[(int)text[i]][b];
            if (pos_x >= SSD1306_LCD_W_RES) {
                pos_x = 0;
                pos_y ++;
            } else pos_x++;
        }
    }
}

void ssd1306_draw_pixel(ssd1306_t *ssd1306_config, int x, int y)
{
    if ((x >= SSD1306_LCD_W_RES) || (y >= SSD1306_LCD_H_RES)) return;
    int pos = x + ( y / 8 ) * SSD1306_LCD_W_RES;
    ssd1306_config->frame_buffer[pos] |= (0x01 << (y & 0x07));
}

void ssd1306_draw_line(ssd1306_t *ssd1306_config, int x_1, int y_1, int x_2, int y_2)
{
    bool steep = abs(y_2 - y_1) > abs(x_2 - x_1);
    if(steep) {
        //swap
        ssd1306_swap_coordenates(x_1, y_1);
        ssd1306_swap_coordenates(x_2, y_2);
    }
    if (x_1 > x_2) {
        // swap
        ssd1306_swap_coordenates(x_1, x_2);
        ssd1306_swap_coordenates(y_1, y_2);
    }
    double dx = x_2 - x_1;
    double dy = abs(y_2 - y_1);
    double err = dx / 2;
    double ystep;
    if (y_1 < y_2) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for(; x_1 <= x_2; x_1++) {
        if(steep) {
            ssd1306_draw_pixel(ssd1306_config, y_1, x_1);
        } else {
            ssd1306_draw_pixel(ssd1306_config, x_1, y_1);
        }
        err = round(err - dy);
        if(err < 0) {
            y_1 += ystep;
            err = round(err + dx);
        }
    }
}

void ssd1306_draw_circle(ssd1306_t *ssd1306_config, int x, int y, int radius)
{
    int cx = 0;
    int cy = -radius;
    int p = -radius;
    while (cx < -cy) {
        if (p > 0) {
            cy+=1;
            p+=(2*(cx+cy)) + 1;
        } else {
            p += (2*cx) + 1;
        }
        ssd1306_draw_pixel(ssd1306_config, x + cx, y + cy );
        ssd1306_draw_pixel(ssd1306_config, x - cx, y + cy );
        ssd1306_draw_pixel(ssd1306_config, x + cx, y - cy );
        ssd1306_draw_pixel(ssd1306_config, x - cx, y - cy );
        ssd1306_draw_pixel(ssd1306_config, x + cy, y + cx );
        ssd1306_draw_pixel(ssd1306_config, x + cy, y - cx );
        ssd1306_draw_pixel(ssd1306_config, x - cy, y + cx );
        ssd1306_draw_pixel(ssd1306_config, x - cy, y - cx );
        cx++;
    }
}

void ssd1306_draw_parallel_line(ssd1306_t *ssd1306_config, int x, int y, int width)
{
    (void)ssd1306_config;
    (void)x;
    (void)y;
    (void)width;
}

void ssd1306_draw_h_line(ssd1306_t *ssd1306_config, int x, int y, int w)
{
    ssd1306_draw_line(ssd1306_config, x, y, x + w - 1, y);
}

void ssd1306_draw_v_line(ssd1306_t *ssd1306_config, int x, int y, int h)
{
    ssd1306_draw_line(ssd1306_config, x, y, x, y + h - 1);
}

void ssd1306_draw_fill_rectangle(ssd1306_t *ssd1306_config, int x, int y, int width, int height)
{
    for (int i = x; i <= x+width; i++)
        ssd1306_draw_v_line(ssd1306_config, i, y, height);
}

void ssd1306_draw_rectangle(ssd1306_t *ssd1306_config, int x, int y, int width, int height)
{
    ssd1306_draw_h_line(ssd1306_config, x, y, width);
    ssd1306_draw_h_line(ssd1306_config, x, y + height - 1, width);
    ssd1306_draw_v_line(ssd1306_config, x, y, height);
    ssd1306_draw_v_line(ssd1306_config, x + width - 1, y, height);
}

void ssd1306_clear(ssd1306_t *ssd1306_config)
{
    for (int i = 0 ; i <= (SSD1306_LCD_FULL_RES); i++)
        ssd1306_config->frame_buffer[i] = 0x00;
}

void ssd1306_fill_buff(ssd1306_t *ssd1306_config)
{
    for (int i = 0 ; i <= (SSD1306_LCD_FULL_RES); i++)
        ssd1306_config->frame_buffer[i] |= (1 << (i & 7));
}

esp_err_t ssd1306_switch(ssd1306_t *ssd1306_c, bool state)
{
    esp_err_t err;
    uint8_t buffer_state[] = {
        OLED_CONTROL_BYTE_CMD_SINGLE,
        state ? OLED_CMD_DISPLAY_ON : OLED_CMD_DISPLAY_OFF,
    };
    err = i2c_master_transmit(ssd1306_c->dev_handle_t, buffer_state, sizeof(buffer_state), SSD1306_TIMEOUT / portTICK_PERIOD_MS);
    return err;
}

void ssd1306_send_buff(ssd1306_t *ssd1306_config)
{
    esp_err_t err;
    uint8_t write_buffer[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_SET_COLUMN_RANGE,
        (0x00), // >= 0x00
        (0x7F), // <= 0x7F
        OLED_CMD_SET_PAGE_RANGE,
        (0x00), // lower start column
        (0x07), // upper start column
    };
    err = i2c_master_transmit(ssd1306_config->dev_handle_t, write_buffer, sizeof(write_buffer), SSD1306_TIMEOUT / portTICK_PERIOD_MS);
    if(err != ESP_OK) {
        ESP_LOGI("DRAW", "Fail to send buff to screen!");
        return;
    }

    uint8_t out_buf[SSD1306_LCD_FULL_RES + 1];
    //uint8_t out_buf[SSD1306_LCD_W_RES + 1];
    out_buf[0] = OLED_CONTROL_BYTE_DATA_STREAM;
    //for (uint8_t i = 0; i <= 0x07; i++) {
    //    memcpy(&out_buf[1], &ssd1306_config->frame_buffer[i * ssd1306_config->width + 1], ssd1306_config->width);
    //    err = i2c_master_transmit(ssd1306_config->dev_handle_t, out_buf, sizeof(out_buf), SSD1306_TIMEOUT / portTICK_PERIOD_MS);
    //    if (err != ESP_OK) {
    //        ESP_LOGI("DRAW", "Fail to draw pixel in screen!");
    //        return;
    //    }
    //    vTaskDelay(SSD1306_TIMEOUT / portTICK_PERIOD_MS);
    //}
    memcpy(&out_buf[1], &ssd1306_config->frame_buffer, SSD1306_LCD_FULL_RES);
    err = i2c_master_transmit(ssd1306_config->dev_handle_t, out_buf, sizeof(out_buf), SSD1306_TIMEOUT);
    if (err != ESP_OK) {
        ESP_LOGI("DRAW", "Fail to draw pixel in screen!");
        return;
    }
}

void ssd1306_send_buff_to_page(ssd1306_t *ssd1306_config, uint8_t column, uint8_t page)
{
    esp_err_t err;
    uint8_t write_buffer[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_SET_COLUMN_RANGE,
        (column), // >= 0x00
        (column), // <= 0x7F
        OLED_CMD_SET_PAGE_RANGE,
        (page), // lower start column
        (0x07), // upper start column
    };
    err = i2c_master_transmit(ssd1306_config->dev_handle_t, write_buffer, sizeof(write_buffer), portTICK_PERIOD_MS);
    if(err != ESP_OK) {
        ESP_LOGI("DRAW", "Fail to send buff to screen!");
        return;
    }

    uint8_t out_buf[ssd1306_config->width+1];
    out_buf[0] = OLED_CONTROL_BYTE_DATA_STREAM;
    memcpy(&out_buf[1], &ssd1306_config->frame_buffer[column * ssd1306_config->width + 1], ssd1306_config->width);
    err = i2c_master_transmit(ssd1306_config->dev_handle_t, out_buf, sizeof(out_buf), SSD1306_TIMEOUT / portTICK_PERIOD_MS);
    if (err != ESP_OK) {
        ESP_LOGI("DRAW", "Fail to draw pixel in screen!");
        return;
    }
    // for (uint8_t i = 0; i <= 0x07; i++) {
    //     memcpy(&out_buf[1], &ssd1306_config->frame_buffer[i * ssd1306_config->width + 1], ssd1306_config->width);
    //     err = i2c_master_transmit(ssd1306_config->dev_handle_t, out_buf, sizeof(out_buf), SSD1306_TIMEOUT /portTICK_PERIOD_MS);
    //     if (err != ESP_OK) {
    //         ESP_LOGI("DRAW", "Fail to draw pixel in screen!");
    //         return;
    //     }
    //     vTaskDelay(portTICK_PERIOD_MS);
    // }
}

esp_err_t ssd1306_init(i2c_master_bus_handle_t *bus_handle_connect ,ssd1306_t *ssd1306_config, i2c_device_config_t *dev_config)
{
    esp_err_t err = i2c_master_bus_add_device(*bus_handle_connect, dev_config, &ssd1306_config->dev_handle_t);
    if (err != ESP_OK)
    {
        ESP_LOGI("HANDLER", "FAILED, check configuration.");
        return err;
    }

    uint8_t out_buf[] = {
        OLED_CONTROL_BYTE_CMD_STREAM,
        OLED_CMD_DISPLAY_OFF,
        OLED_CMD_SET_MUX_RATIO, 0x3F,
        OLED_CMD_SET_DISPLAY_OFFSET, 0x00,
        OLED_CMD_SET_DISPLAY_START_LINE,
        OLED_CMD_SET_SEGMENT_REMAP,
        OLED_CMD_SET_COM_SCAN_MODE,
        OLED_CMD_SET_COM_PIN_MAP, 0x12,
        OLED_CMD_SET_CONTRAST, 0x7F,
        OLED_CMD_DISPLAY_RAM,
        OLED_CMD_DISPLAY_NORMAL,
        OLED_CMD_SET_DISPLAY_CLK_DIV, 0x80,
        OLED_CMD_SET_CHARGE_PUMP, 0x14,
        OLED_CMD_SET_VCOMH_DESELCT, 0x30,
        OLED_CMD_SET_MEMORY_ADDR_MODE, 0x00,
        OLED_CMD_DISPLAY_ON,
    };

    err = i2c_master_transmit(ssd1306_config->dev_handle_t, out_buf, sizeof(out_buf), SSD1306_TIMEOUT / portTICK_PERIOD_MS);
    if (err != ESP_OK)
    {
        ESP_LOGI("HANDLER", "FAILED, check transmit.");
        return err;
    }
    memset(ssd1306_config->frame_buffer, 0x00, sizeof((ssd1306_config->frame_buffer)));
    return ESP_OK;
}
