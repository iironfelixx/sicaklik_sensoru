#define STM32F1  // İşlemcinin F1 serisi olduğunu kütüphaneye bildiriyoruz
#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

// I2C ayarları
#define SSD1306_USE_I2C
#define SSD1306_I2C_PORT        hi2c1
#define SSD1306_I2C_ADDR        (0x3C << 1) // 0x78 yapar

// Ekran boyutları
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

// Kullanılacak fontları aktif et
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18

#endif /* __SSD1306_CONF_H__ */
