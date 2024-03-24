#include "DS3231.h"
uint8_t buf[8] = {0};
static uint8_t bcd2dec(uint8_t val) {
    return (val >> 4) * 10 + (val & 0x0f);
}

static uint8_t dec2bcd(uint8_t val) {
    return ((val / 10) << 4) + (val % 10);
}

DS3231::DS3231(I2C_HandleTypeDef& hi2c) : m_hi2c(hi2c) {
}

HAL_StatusTypeDef DS3231::setDateTime(const Time_t* time) {
    uint8_t data[7] = {0};
    data[0] = dec2bcd(time->Sec);
    data[1] = dec2bcd(time->Min);
    data[2] = dec2bcd(time->Hour);
    data[3] = dec2bcd(time->DaysOfWeek);
    data[4] = dec2bcd(time->Date);
    data[5] = dec2bcd(time->Month);
    data[6] = dec2bcd(time->Year);

    return write(DS3231_REG_TIME, data, 7);
}

HAL_StatusTypeDef DS3231::getDateTime(Time_t* time) {
    Time_t t = {0};
    uint8_t raw_data[7] = {0};
    HAL_StatusTypeDef ret;
    ret = read(DS3231_REG_TIME, raw_data, 7);
    if(ret != HAL_OK) {
        return ret;
    }
    /* 0..59 */
    t.Sec = bcd2dec(raw_data[0] & MASK_SECOND);
    /* 0..59 */
    t.Min = bcd2dec(raw_data[1] & MASK_MINUTE);
    /* 0..23 */
    t.Hour = bcd2dec(raw_data[2] & MASK_HOUR);
    /* 1..7 */
    t.DaysOfWeek = bcd2dec(raw_data[3] & MASK_DAY);
    /* 1..31 */
    t.Date = bcd2dec(raw_data[4] & MASK_DATE);
    /* 1..12 */
    t.Month = bcd2dec(raw_data[5] & MASK_MONTH);
    /* 0..99 */
    t.Year = bcd2dec(raw_data[6] & MASK_YEAR);

    *time = t;
    return ret;
}

HAL_StatusTypeDef DS3231::read(uint8_t reg, uint8_t* data, uint8_t size) {
    HAL_StatusTypeDef ret;
    ret = HAL_I2C_Master_Transmit(&m_hi2c, DS3231_ADDR, &reg, 1, HAL_MAX_DELAY);     /* Notify slave which register to read */
    if(ret != HAL_OK) {
        return ret;
    }
    ret = HAL_I2C_Master_Receive(&m_hi2c, DS3231_ADDR, data, size, HAL_MAX_DELAY);    /* This function already add read bit */
    return ret;
}

HAL_StatusTypeDef DS3231::write(uint8_t reg, const uint8_t* data, uint8_t size) {
    if (size > 7) {
        return HAL_ERROR;
    }

    uint8_t buf[8] = {reg, data[0], data[1], data[2], data[3],
    						data[4], data[5], data[6]};
    m_hi2c.State;
    HAL_StatusTypeDef ret = HAL_I2C_Master_Transmit(&m_hi2c, DS3231_ADDR, buf, size + 1, 100);
    return ret;
}

DS3231::~DS3231() {

}
