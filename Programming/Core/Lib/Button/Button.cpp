#include "Button.h"

#define TIME_DEBOUND_BUTTON		(20)
#define TIME_SHORT_PRESS		(1000)
#define TIME_LONG_PRESS			(3000)
#define TIME_ACCEL_MIN			(100)
#define TIME_ACCEL_MAX			(1000)
#define TIME_ACCEL_DELTA		(200)

uint8_t Button::totalButton = 0;

void Button::handleButton(void) {
	current_status = HAL_GPIO_ReadPin(gpio_port, gpio_pin);
	if(button_active == BUTTON_ACTIVE_HIGH) {
		current_status = !current_status;
	}
	switch(button_state) {
		case BUTTON_INTERNAL_READ: {
			if((current_status == 0 && last_status == 1)) {
				time_debounce = HAL_GetTick();
				button_state = BUTTON_INTERNAL_WAIT_DEBOUND;
			}
		}
		break;
		case BUTTON_INTERNAL_WAIT_DEBOUND: {
			if(HAL_GetTick() - time_debounce >= TIME_DEBOUND_BUTTON) {
				if(current_status == 0 && last_status == 1) { //nhan xuong
					button_cb(buttonId, BUTTON_PRESSED);
					t_long_press = HAL_GetTick();
					last_status = 0;
					t_accel_press = HAL_GetTick();
					t_accel_call = TIME_ACCEL_MAX;
					button_state = BUTTON_INTERNAL_WAIT_RELEASE_AND_CHECK_LONG_PRESS;
				}
				else if(current_status ==1 && last_status ==0) { //nha ra
					t_long_press = HAL_GetTick() - t_long_press;
					if(t_long_press <= TIME_SHORT_PRESS) {
						button_cb(buttonId, BUTTON_CLICKED);
					}
					button_cb(buttonId, BUTTON_RELEASED);
					last_status = 1;
					button_state = BUTTON_INTERNAL_READ;
				}
				else { //khong dung
					last_status = 1;
					button_state = BUTTON_INTERNAL_READ;
				}
			}
		}
		break;
		case BUTTON_INTERNAL_WAIT_RELEASE_AND_CHECK_LONG_PRESS: {
				if(current_status == 1 && last_status == 0) {
					button_state = BUTTON_INTERNAL_WAIT_DEBOUND;
					time_debounce = HAL_GetTick();
				}
				else if(HAL_GetTick() - t_long_press >= TIME_LONG_PRESS) {
					button_cb(buttonId, BUTTON_TIMEOUT);
					button_state = BUTTON_INTERNAL_WAIT_RELEASE;
				}
				else if(HAL_GetTick() -  t_accel_press >= t_accel_call) {
					t_accel_call -= TIME_ACCEL_DELTA;
					if(t_accel_call <= TIME_ACCEL_MIN) {
						t_accel_call = TIME_ACCEL_MIN;
					}
					button_cb(buttonId, BUTTON_PRESSED_LONG);
					t_accel_press = HAL_GetTick();
				}
		}
		break;
		case BUTTON_INTERNAL_WAIT_RELEASE: {
			if(current_status == 1 && last_status == 0) {
				button_state = BUTTON_INTERNAL_WAIT_DEBOUND;
				time_debounce = HAL_GetTick();
			}
			else if(HAL_GetTick() -  t_accel_press >= t_accel_call) {
				t_accel_call -=TIME_ACCEL_DELTA;
				if(t_accel_call <= TIME_ACCEL_MIN)
				{
					t_accel_call = TIME_ACCEL_MIN;
				}
				button_cb(buttonId, BUTTON_PRESSED_LONG);
				t_accel_press = HAL_GetTick();
			}
		}
		break;
		default:
		break;
	}
}

void Button::buttonConfigGpio(void) {
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if(gpio_port == GPIOA) {
		__HAL_RCC_GPIOA_CLK_ENABLE();
	}
	else if(gpio_port == GPIOB) {
		__HAL_RCC_GPIOB_CLK_ENABLE();
	}
	else if(gpio_port == GPIOC) {
		__HAL_RCC_GPIOC_CLK_ENABLE();
	}
	else if(gpio_port == GPIOD) {
		__HAL_RCC_GPIOD_CLK_ENABLE();
	}
	GPIO_InitStruct.Pin = gpio_pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = button_active == BUTTON_ACTIVE_LOW ? GPIO_PULLUP : GPIO_PULLDOWN;
	HAL_GPIO_Init(gpio_port, &GPIO_InitStruct);
}

//button_active: BUTTON_ACTIVE_LOW, BUTTON_ACTIVE_HIGH
Button::Button(GPIO_TypeDef* port, uint16_t pin, Button_active_t button_active, buttonEventCallback cb) {
	totalButton++;
	buttonId = totalButton;
	gpio_port = port;
	gpio_pin = pin;
	button_active = button_active;
	button_state = BUTTON_INTERNAL_READ;
	current_status = 1;
	last_status = 1;
	button_cb = cb;
	buttonConfigGpio();
}

Button::Button(GPIO_TypeDef* port, uint16_t pin, Button_active_t button_active) {
	gpio_port = port;
	gpio_pin = pin;
	button_active = button_active;
	button_state = BUTTON_INTERNAL_READ;
	current_status = 1;
	last_status = 1;
	button_cb = nullptr;
	buttonConfigGpio();
}


