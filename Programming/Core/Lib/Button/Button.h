#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "stm32f4xx_hal.h"

typedef enum {
	BUTTON_ACTIVE_LOW = 0,
	BUTTON_ACTIVE_HIGH
} Button_active_t;

typedef enum {
	BUTTON_PRESSED,
	BUTTON_RELEASED,
	BUTTON_CLICKED,
	BUTTON_PRESSED_LONG,
	BUTTON_TIMEOUT
} Button_event_state_t;

typedef void (*buttonEventCallback)(uint8_t id, Button_event_state_t state);

class Button {
private:
	typedef enum {
		BUTTON_INTERNAL_READ,
		BUTTON_INTERNAL_WAIT_DEBOUND,
		BUTTON_INTERNAL_WAIT_RELEASE_AND_CHECK_LONG_PRESS,
		BUTTON_INTERNAL_WAIT_RELEASE
	} Button_internal_state_t;
	static uint8_t totalButton;

	GPIO_TypeDef* gpio_port;
	uint16_t gpio_pin;
	uint8_t current_status;
	uint8_t last_status;
	uint32_t time_debounce;
	uint32_t t_long_press;
	Button_internal_state_t button_state;
	Button_active_t button_active;
	uint32_t t_accel_call;
	uint32_t t_accel_press;
	buttonEventCallback button_cb;
	void buttonConfigGpio(void);
public:
	Button(GPIO_TypeDef* port, uint16_t pin, Button_active_t button_active, buttonEventCallback cb);
	Button(GPIO_TypeDef* port, uint16_t pin, Button_active_t button_active);
	void handleButton(void);
	uint8_t buttonId;
};

#endif /* _BUTTON_H_ */

#ifndef _BUTTON_H_
#define _BUTTON_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"

typedef struct Button_s Button;

typedef enum {
	BUTTON_ACTIVE_LOW = 0,
	BUTTON_ACTIVE_HIGH
} Button_active_t;

typedef enum {
	BUTTON_PRESSED,
	BUTTON_RELEASED,
	BUTTON_CLICKED,
	BUTTON_PRESSED_LONG,
	BUTTON_TIMEOUT
} Button_event_state_t;

typedef void (*buttonEventCallback)(Button *btn, Button_event_state_t state);

struct Button_s {
	GPIO_TypeDef* gpio_port;
	uint16_t gpio_pin;
	uint8_t current_status;
	uint8_t last_status;
	uint32_t time_debounce;
	uint32_t t_long_press;
	Button_event_state_t button_state;
	Button_active_t button_active;
	int16_t t_accel_call;
	uint32_t t_accel_press;
	buttonEventCallback button_cb;
};

void handle_button(Button* btn);
void button_init(Button* btn, GPIO_TypeDef* port, uint16_t pin, Button_active_t button_active, buttonEventCallback cb);

#ifdef __cplusplus
}
#endif

#endif /* _BUTTON_H_ */
