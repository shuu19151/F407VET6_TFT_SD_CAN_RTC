

#include <stdlib.h>
#include <stdbool.h>
#include <string>
#include "stm32f4xx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DS3231_ADDR             (0x68 << 1) //!< I2C address

#define DS3231_REG_TIME         (0x00)
#define DS3231_REG_ALARM1       (0x07)
#define DS3231_REG_ALARM2       (0x0B)
#define DS3231_REG_CONTROL      (0x0E)
#define DS3231_REG_STATUS       (0x0F)
#define DS3231_REG_TEMP         (0x11)

#define DS3231_CON_EOSC         (0x80)
#define DS3231_CON_BBSQW        (0x40)
#define DS3231_CON_CONV         (0x20)
#define DS3231_CON_RS2          (0x10)
#define DS3231_CON_RS1          (0x08)
#define DS3231_CON_INTCN        (0x04)
#define DS3231_CON_A2IE         (0x02)
#define DS3231_CON_A1IE         (0x01)

#define DS3231_STA_OSF          (0x80)
#define DS3231_STA_32KHZ        (0x08)
#define DS3231_STA_BSY          (0x04)
#define DS3231_STA_A2F          (0x02)
#define DS3231_STA_A1F          (0x01)

#define MASK_SECOND             (0x7F) // 0b01111111
#define MASK_MINUTE             (0x7F) // 0b01111111
#define MASK_HOUR               (0x3F) // 0b00111111
#define MASK_DAY                (0x07) // 0b00000111
#define MASK_DATE               (0x3F) // 0b00111111
#define MASK_MONTH              (0x1F) // 0b00011111
#define MASK_YEAR               (0xFF) // 0b11111111


typedef enum {
  ALARM_MODE_ALL_MATCHED = 0,
  ALARM_MODE_HOUR_MIN_SEC_MATCHED,
  ALARM_MODE_MIN_SEC_MATCHED,
  ALARM_MODE_SEC_MATCHED,
  ALARM_MODE_ONCE_PER_SECOND
} AlarmMode_t;

typedef enum {
  SUNDAY = 1,
  MONDAY,
  TUESDAY,
  WEDNESDAY,
  THURSDAY,
  FRIDAY,
  SATURDAY
} DaysOfWeek_t;

typedef struct {
  uint8_t Year;
  uint8_t Month;
  uint8_t Date;
  uint8_t DaysOfWeek;
  uint8_t Hour;
  uint8_t Min;
  uint8_t Sec;
} Time_t;

class DS3231 {
private:
  I2C_HandleTypeDef& m_hi2c;
//  Time_t m_time;
  HAL_StatusTypeDef write(uint8_t reg, const uint8_t* data, uint8_t size);
  HAL_StatusTypeDef read(uint8_t reg, uint8_t* data, uint8_t size);
public:
  DS3231(I2C_HandleTypeDef& hi2c);
  virtual ~DS3231();
  HAL_StatusTypeDef setDateTime(const Time_t* time);
  HAL_StatusTypeDef getDateTime(Time_t* time);
  HAL_StatusTypeDef getStringDateTime(std::string& str);
};

#ifdef __cplusplus
}
#endif
