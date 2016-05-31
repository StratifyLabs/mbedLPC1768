/*

Copyright 2011-2016 Tyler Gilbert

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include <unistd.h>
#include <mcu/core.h>
#include <mcu/pio.h>
#include <mcu/mcu.h>

#include "led_start.h"

void priv_led_on(void * args){
	uint32_t * pinmask = (uint32_t *)args;
	pio_attr_t attr;
	attr.mask = (*pinmask);
	attr.mode = PIO_MODE_OUTPUT;
	mcu_pio_setattr(1, &attr);
	mcu_pio_setmask(1, (void*)(attr.mask));
}

void priv_led_off(void * args){
	uint32_t * pinmask = (uint32_t *)args;
	pio_attr_t attr;
	attr.mask = (*pinmask);
	mcu_pio_clrmask(1, (void*)(attr.mask));
	attr.mode = PIO_MODE_INPUT;
	mcu_pio_setattr(1, &attr);
}

void led_on(u32 mask){ mcu_core_privcall(priv_led_on, &mask); }
void led_off(u32 mask){ mcu_core_privcall(priv_led_off, &mask); }

void led_flash(u32 mask, u32 period){
	led_on(mask); usleep(period); led_off(mask);
}

void led_start(){
	led_flash(1<<18, 50000);
	led_flash(1<<20, 100000);
	led_flash(1<<21, 150000);
	led_flash(1<<23, 200000);
}



