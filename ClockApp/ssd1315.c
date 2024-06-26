#include <ssd1315.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"


#define SSD1315_SLAVE_ADDRESS 0x3C
#define ROW_COUNT 4
#define COLUMN_COUNT 16
#define DISPLAY_ON_LEN 2
#define NORMAL_DISPLAY_LEN 2
#define ENTIRE_DISPLAY_LEN 2
#define VCOMSEL_LEN 3
#define PRECHARGE_LEN 3
#define CONTRAST_LEN 3
#define COM_PIN_HW_CONFIG_LEN 3
#define SCAN_DIR_LEN 2
#define SEG_REMAP_LEN 2
#define MEM_ADDR_MODE_LEN 3
#define CHARGE_PUMP_LEN 3
#define START_LINE_LEN 2
#define OFFSET_LEN 3
#define MULTIPLEX_LEN 3
#define CLOCK_DIV_LEN 3
#define DISPLAY_OFF_LEN 2
#define COLUMN_HIGH_LEN 2
#define COLUMN_LOW_LEN 2
#define PAGE_START_LEN 2


// Font for the SSD1315
static const uint8_t cui8_ssd1315Font[][8] =
{
  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x5F,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x07,0x00,0x07,0x00,0x00,0x00},
  {0x00,0x14,0x7F,0x14,0x7F,0x14,0x00,0x00},
  {0x00,0x24,0x2A,0x7F,0x2A,0x12,0x00,0x00},
  {0x00,0x23,0x13,0x08,0x64,0x62,0x00,0x00},
  {0x00,0x36,0x49,0x55,0x22,0x50,0x00,0x00},
  {0x00,0x00,0x05,0x03,0x00,0x00,0x00,0x00},
  {0x00,0x1C,0x22,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x41,0x22,0x1C,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x2A,0x1C,0x2A,0x08,0x00,0x00},
  {0x00,0x08,0x08,0x3E,0x08,0x08,0x00,0x00},
  {0x00,0xA0,0x60,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x08,0x08,0x08,0x08,0x00,0x00},
  {0x00,0x60,0x60,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x10,0x08,0x04,0x02,0x00,0x00},
  {0x00,0x3E,0x51,0x49,0x45,0x3E,0x00,0x00},
  {0x00,0x00,0x42,0x7F,0x40,0x00,0x00,0x00},
  {0x00,0x62,0x51,0x49,0x49,0x46,0x00,0x00},
  {0x00,0x22,0x41,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x18,0x14,0x12,0x7F,0x10,0x00,0x00},
  {0x00,0x27,0x45,0x45,0x45,0x39,0x00,0x00},
  {0x00,0x3C,0x4A,0x49,0x49,0x30,0x00,0x00},
  {0x00,0x01,0x71,0x09,0x05,0x03,0x00,0x00},
  {0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x06,0x49,0x49,0x29,0x1E,0x00,0x00},
  {0x00,0x00,0x36,0x36,0x00,0x00,0x00,0x00},
  {0x00,0x00,0xAC,0x6C,0x00,0x00,0x00,0x00},
  {0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x00},
  {0x00,0x14,0x14,0x14,0x14,0x14,0x00,0x00},
  {0x00,0x41,0x22,0x14,0x08,0x00,0x00,0x00},
  {0x00,0x02,0x01,0x51,0x09,0x06,0x00,0x00},
  {0x00,0x32,0x49,0x79,0x41,0x3E,0x00,0x00},
  {0x00,0x7E,0x09,0x09,0x09,0x7E,0x00,0x00},
  {0x00,0x7F,0x49,0x49,0x49,0x36,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x41,0x22,0x00,0x00},
  {0x00,0x7F,0x41,0x41,0x22,0x1C,0x00,0x00},
  {0x00,0x7F,0x49,0x49,0x49,0x41,0x00,0x00},
  {0x00,0x7F,0x09,0x09,0x09,0x01,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x51,0x72,0x00,0x00},
  {0x00,0x7F,0x08,0x08,0x08,0x7F,0x00,0x00},
  {0x00,0x41,0x7F,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x40,0x41,0x3F,0x01,0x00,0x00},
  {0x00,0x7F,0x08,0x14,0x22,0x41,0x00,0x00},
  {0x00,0x7F,0x40,0x40,0x40,0x40,0x00,0x00},
  {0x00,0x7F,0x02,0x0C,0x02,0x7F,0x00,0x00},
  {0x00,0x7F,0x04,0x08,0x10,0x7F,0x00,0x00},
  {0x00,0x3E,0x41,0x41,0x41,0x3E,0x00,0x00},
  {0x00,0x7F,0x09,0x09,0x09,0x06,0x00,0x00},
  {0x00,0x3E,0x41,0x51,0x21,0x5E,0x00,0x00},
  {0x00,0x7F,0x09,0x19,0x29,0x46,0x00,0x00},
  {0x00,0x26,0x49,0x49,0x49,0x32,0x00,0x00},
  {0x00,0x01,0x01,0x7F,0x01,0x01,0x00,0x00},
  {0x00,0x3F,0x40,0x40,0x40,0x3F,0x00,0x00},
  {0x00,0x1F,0x20,0x40,0x20,0x1F,0x00,0x00},
  {0x00,0x3F,0x40,0x38,0x40,0x3F,0x00,0x00},
  {0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x00},
  {0x00,0x03,0x04,0x78,0x04,0x03,0x00,0x00},
  {0x00,0x61,0x51,0x49,0x45,0x43,0x00,0x00},
  {0x00,0x7F,0x41,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x02,0x04,0x08,0x10,0x20,0x00,0x00},
  {0x00,0x41,0x41,0x7F,0x00,0x00,0x00,0x00},
  {0x00,0x04,0x02,0x01,0x02,0x04,0x00,0x00},
  {0x00,0x80,0x80,0x80,0x80,0x80,0x00,0x00},
  {0x00,0x01,0x02,0x04,0x00,0x00,0x00,0x00},
  {0x00,0x20,0x54,0x54,0x54,0x78,0x00,0x00},
  {0x00,0x7F,0x48,0x44,0x44,0x38,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x28,0x00,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x48,0x7F,0x00,0x00},
  {0x00,0x38,0x54,0x54,0x54,0x18,0x00,0x00},
  {0x00,0x08,0x7E,0x09,0x02,0x00,0x00,0x00},
  {0x00,0x18,0xA4,0xA4,0xA4,0x7C,0x00,0x00},
  {0x00,0x7F,0x08,0x04,0x04,0x78,0x00,0x00},
  {0x00,0x00,0x7D,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x80,0x84,0x7D,0x00,0x00,0x00,0x00},
  {0x00,0x7F,0x10,0x28,0x44,0x00,0x00,0x00},
  {0x00,0x41,0x7F,0x40,0x00,0x00,0x00,0x00},
  {0x00,0x7C,0x04,0x18,0x04,0x78,0x00,0x00},
  {0x00,0x7C,0x08,0x04,0x7C,0x00,0x00,0x00},
  {0x00,0x38,0x44,0x44,0x38,0x00,0x00,0x00},
  {0x00,0xFC,0x24,0x24,0x18,0x00,0x00,0x00},
  {0x00,0x18,0x24,0x24,0xFC,0x00,0x00,0x00},
  {0x00,0x00,0x7C,0x08,0x04,0x00,0x00,0x00},
  {0x00,0x48,0x54,0x54,0x24,0x00,0x00,0x00},
  {0x00,0x04,0x7F,0x44,0x00,0x00,0x00,0x00},
  {0x00,0x3C,0x40,0x40,0x7C,0x00,0x00,0x00},
  {0x00,0x1C,0x20,0x40,0x20,0x1C,0x00,0x00},
  {0x00,0x3C,0x40,0x30,0x40,0x3C,0x00,0x00},
  {0x00,0x44,0x28,0x10,0x28,0x44,0x00,0x00},
  {0x00,0x1C,0xA0,0xA0,0x7C,0x00,0x00,0x00},
  {0x00,0x44,0x64,0x54,0x4C,0x44,0x00,0x00},
  {0x00,0x08,0x36,0x41,0x00,0x00,0x00,0x00},
  {0x00,0x00,0x7F,0x00,0x00,0x00,0x00,0x00},
  {0x00,0x41,0x36,0x08,0x00,0x00,0x00,0x00},
  {0x00,0x02,0x01,0x01,0x02,0x01,0x00,0x00},
  {0x00,0x02,0x05,0x05,0x02,0x00,0x00,0x00},
};

static void oled_put_char(char c);

static void I2CWait();
static void I2CWriteRegisters(const uint8_t* ui8_data, uint8_t ui8_dataLength);

static uint32_t ui32g_i2cBase;

// Before using SSD1315 setup I2C and call this function to set which I2C base will SSD1315 use
void SSD1315_Init(uint32_t ui32_i2cBase)
{
    // Set the global variable which I2C will LIS3DH use
    ui32g_i2cBase = ui32_i2cBase;

    // Turn display off
    const uint8_t cmd_display_off = 0xAE;
    uint8_t display_off_sequence[DISPLAY_OFF_LEN] = {0x00, cmd_display_off};
    I2CWriteRegisters(display_off_sequence, DISPLAY_OFF_LEN);

    // Set display clock divider
    const uint8_t cmd_display_clock_div = 0xD5;
    const uint8_t divide_ratio = 0x80;
    uint8_t display_clock_div_sequence[CLOCK_DIV_LEN] = {0x00, cmd_display_clock_div, divide_ratio};
    I2CWriteRegisters(display_clock_div_sequence, CLOCK_DIV_LEN);

    // Set multiplex ratio
    const uint8_t cmd_multiplex = 0xA8;
    const uint8_t multiplex_ratio = 0x1F;
    uint8_t multiplex_sequence[MULTIPLEX_LEN] = {0x00, cmd_multiplex, multiplex_ratio};
    I2CWriteRegisters(multiplex_sequence, MULTIPLEX_LEN);

    // Set display offset
    const uint8_t cmd_offset = 0xD3;
    const uint8_t offset_val = 0;
    uint8_t offset_sequence[OFFSET_LEN] = {0x00, cmd_offset, offset_val};
    I2CWriteRegisters(offset_sequence, OFFSET_LEN);

    // Set display start line
    const uint8_t cmd_start_line = 0x40;
    const uint8_t start_line_val = 0x00;
    const uint8_t start_line_combined = cmd_start_line | start_line_val;
    uint8_t start_line_seq[START_LINE_LEN] = {0x00, start_line_combined};
    I2CWriteRegisters(start_line_seq, START_LINE_LEN);

    // Set charge pump settings
    const uint8_t cmd_charge_pump = 0x8D;
    const uint8_t pump_setting = 0x94;
    uint8_t charge_pump_seq[CHARGE_PUMP_LEN] = {0x00, cmd_charge_pump, pump_setting};
    I2CWriteRegisters(charge_pump_seq, CHARGE_PUMP_LEN);

    // Set memory addressing mode (0x20 = page, 0x00 = horizontal, 0x01 = vertical)
    const uint8_t cmd_mem_addr_mode = 0x20;
    const uint8_t mem_addr_mode_val = 0x20;
    uint8_t mem_addr_mode_seq[MEM_ADDR_MODE_LEN] = {0x00, cmd_mem_addr_mode, mem_addr_mode_val};
    I2CWriteRegisters(mem_addr_mode_seq, MEM_ADDR_MODE_LEN);

    // Set segment remap
    const uint8_t cmd_seg_remap = 0xA0;
    uint8_t seg_remap_seq[SEG_REMAP_LEN] = {0x00, cmd_seg_remap};
    I2CWriteRegisters(seg_remap_seq, SEG_REMAP_LEN);

    // Set COM output scan direction
    const uint8_t cmd_scan_dir = 0xC0;
    uint8_t scan_dir_seq[SCAN_DIR_LEN] = {0x00, cmd_scan_dir};
    I2CWriteRegisters(scan_dir_seq, SCAN_DIR_LEN);

    // Set COM pins hardware configuration
    const uint8_t cmd_com_pin_hw_config = 0xDA;
    const uint8_t com_pin_hw_config_val = 0x02;
    uint8_t com_pin_hw_config_seq[COM_PIN_HW_CONFIG_LEN] =
        {0x00, cmd_com_pin_hw_config, com_pin_hw_config_val};
    I2CWriteRegisters(com_pin_hw_config_seq, COM_PIN_HW_CONFIG_LEN);

    // Set contrast
    const uint8_t cmd_contrast = 0x81;
    const uint8_t contrast_val = 0x8F;
    uint8_t contrast_seq[CONTRAST_LEN] = {0x00, cmd_contrast, contrast_val};
    I2CWriteRegisters(contrast_seq, CONTRAST_LEN);

    // Set pre-charge
    const uint8_t cmd_precharge = 0xD9;
    const uint8_t precharge_period = 0xF1;
    uint8_t precharge_seq[PRECHARGE_LEN] = {0x00, cmd_precharge, precharge_period};
    I2CWriteRegisters(precharge_seq, PRECHARGE_LEN);

    // Set Vcomh select level
    const uint8_t cmd_vcomsel = 0xDB;
    const uint8_t vcomsel_level = 0x20;
    uint8_t vcomsel_seq[VCOMSEL_LEN] = {0x00, cmd_vcomsel, vcomsel_level};
    I2CWriteRegisters(vcomsel_seq, VCOMSEL_LEN);

    // Display RAM content instead of turning all pixels on (0xA4 for normal, 0xA5 for entire)
    const uint8_t cmd_entire_display = 0xA4;
    uint8_t entire_display_seq[ENTIRE_DISPLAY_LEN] = {0x00, cmd_entire_display};
    I2CWriteRegisters(entire_display_seq, ENTIRE_DISPLAY_LEN);

    // Normal display instead of inverse
    const uint8_t cmd_normal_display = 0xA6;
    uint8_t normal_display_seq[NORMAL_DISPLAY_LEN] = {0x00, cmd_normal_display};
    I2CWriteRegisters(normal_display_seq, NORMAL_DISPLAY_LEN);

    // Turn the display on
    const uint8_t cmd_display_on = 0xAF;
    uint8_t display_on_sequence[DISPLAY_ON_LEN] = {0x00, cmd_display_on};
    I2CWriteRegisters(display_on_sequence, DISPLAY_ON_LEN);

    SSD1315_Clear();
}

// Clear the SSD1315
void SSD1315_Clear()
{
    uint8_t ui8_row;
    uint8_t ui8_column;

    for(ui8_row = 0u; ui8_row < ROW_COUNT; ui8_row++)
    {
        for(ui8_column = 0; ui8_column < COLUMN_COUNT; ui8_column++)
        {
            SSD1315_SetCursorPosition(ui8_row, ui8_column);
            oled_put_char(' ');
        }
    }
}

// Write given string to the SSD1315
void SSD1315_WriteString(uint8_t* ui8p_string, uint8_t stringLength)
{
    uint8_t ui8_counter;

    for (ui8_counter = 0u; ui8_counter < stringLength; ui8_counter++)
    {
        oled_put_char(ui8p_string[ui8_counter]);
    }
}

static void oled_put_char(char c)
{
    // Catch unsupported characters
    if((c < 32) || (c > 127))
    {
        c = '?';
    }

    // Whole payload (command header + pixel data) will be constructed here
    uint8_t payload[9];
    uint32_t len = 9;

    // First byte says we are writing data to pixel matrix
    const uint8_t control_byte_write = 0x40;
    payload[0] = control_byte_write;

    // Next 8 bytes comprise the pixel data from font database
    uint32_t i;
    const uint8_t *font_base = &cui8_ssd1315Font[c-32][0];
    for(i = 0; i < 8; i++)
    {
        payload[i+1] = font_base[i];
    }

    // Sent total payload of 9 bytes
    I2CWriteRegisters(payload, len);
}

void SSD1315_SetCursorPosition(uint8_t row, uint8_t col)
{
    // Set page start address
    const uint8_t cmd_page_start = 0xB0 + row;
    uint8_t page_start_seq[PAGE_START_LEN] = {0x00, cmd_page_start};
    I2CWriteRegisters(page_start_seq, PAGE_START_LEN);

    // Set lower column start address
    const uint8_t cmd_column_low = (8*col & 0x0F);
    uint8_t column_low_seq[COLUMN_LOW_LEN] = {0x00, cmd_column_low};
    I2CWriteRegisters(column_low_seq, COLUMN_LOW_LEN);


    // Set higher column start address
    const uint8_t cmd_column_high = 0x10 + (((8*col) >> 4) & 0x0F);
    uint8_t column_high_seq[COLUMN_HIGH_LEN] = {0x00, cmd_column_high};
    I2CWriteRegisters(column_high_seq, COLUMN_HIGH_LEN);
}

// Wait while the I2C peripheral is busy
static void I2CWait()
{
    // Wait while the I2C1 is busy
    while(I2CMasterBusy(ui32g_i2cBase));
}

// Write data to SSD1315 registers
static void I2CWriteRegisters(const uint8_t* ui8_data, uint8_t ui8_dataLength)
{
    uint8_t ui8_counter;

    // Set address of slave device
    I2CMasterSlaveAddrSet(ui32g_i2cBase, SSD1315_SLAVE_ADDRESS, false);

    // Send first byte of data
    I2CMasterDataPut(ui32g_i2cBase, ui8_data[0u]);
    I2CMasterControl(ui32g_i2cBase, I2C_MASTER_CMD_BURST_SEND_START);
    I2CWait();

    // Send the rest of the bytes except the last one
    for(ui8_counter = 1u; ui8_counter < ui8_dataLength - 1u; ui8_counter++)
    {
        I2CMasterDataPut(ui32g_i2cBase, ui8_data[ui8_counter]);
        I2CMasterControl(ui32g_i2cBase, I2C_MASTER_CMD_BURST_SEND_CONT);
        I2CWait();
    }

    // Send last byte of data
    I2CMasterDataPut(ui32g_i2cBase, ui8_data[ui8_counter]);
    I2CMasterControl(ui32g_i2cBase, I2C_MASTER_CMD_BURST_SEND_FINISH);
    I2CWait();
}
