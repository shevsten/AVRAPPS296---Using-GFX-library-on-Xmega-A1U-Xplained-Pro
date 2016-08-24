/**
 * \file
 *
 * \brief Monochrome graphic library example application
 *
 * Copyright (c) 2014-2015 Atmel Corporation. All rights reserved.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 *    Atmel microcontroller product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * \asf_license_stop
 *
 */
/**
 * \mainpage
 *
 * \section intro Introduction
 * This is an example application for the \ref gfx_mono.
 * It will output some random graphic primitives to a connected LCD screen or
 * framebuffer.
 *
 * \section files Main Files
 * - \ref conf_board.h
 * - \ref conf_clock.h
 * - \ref conf_st7565r.h or conf_ssd1306.h
 * - \ref conf_usart_spi.h
 *
 * \section device_info Device Info
 * All AVR devices can be used.
 * This example has been tested with the following setup:
 * - XMEGA-A3BU Xplained
 * - XMEGA-C3 Xplained
 *
 * \section dependencies Dependencies
 * Relevant module dependencies for this application are:
 * - \ref gfx_mono
 *
 * \section compinfo Compilation info
 * This software was written for the GNU GCC and IAR for AVR. Other compilers
 * may or may not work.
 *
 * \section contactinfo Contact Information
 * For further information, visit <a href="http://www.atmel.com/">Atmel</a>.\n
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <board.h>
#include <sysclk.h>
#include <stdio.h>
#include <gfx_mono.h>
#include "gfx_mono_text.h"
#include "sysfont.h"
#include "gfx_mono_spinctrl.h"
#include "gfx_mono_menu.h"
#include "pmic.h"

// Bitmap of two 8x8 pixel smileyfaces on top of each other stored in RAM
uint8_t smiley_data[16] = {
		0x3C, 0x42, 0x95, 0xA1, 0xA1, 0x95, 0x42, 0x3C,
		0x3C, 0x42, 0x95, 0xA1, 0xA1, 0x95, 0x42, 0x3C
};

// Bitmap of two 8x8 pixel smileyfaces on top of each other stored in FLASH
PROGMEM_DECLARE(gfx_mono_color_t, flash_bitmap[16]) =  {
		0x3C, 0x42, 0x95, 0xA1, 0xA1, 0x95, 0x42, 0x3C,
		0x3C, 0x42, 0x95, 0xA1, 0xA1, 0x95, 0x42, 0x3C
};

PROGMEM_DECLARE(char const, spinnertitle[]) = "Strings:";
PROGMEM_DECLARE(char const, spinnertitle2[]) = "Numbers:";
PROGMEM_DECLARE(char const, spinnertitle3[]) = "Numbers2:";
PROGMEM_DECLARE(char const, spinner_choice1[]) = "String";
PROGMEM_DECLARE(char const, spinner_choice2[]) = "String 2";
PROGMEM_DECLARE(char const, spinner_choice3[]) = "String 3";
PROGMEM_DECLARE(char const, spinner_choice4[]) = "String 4";


PROGMEM_STRING_T spinner_choicestrings[] = {
	spinner_choice1,
	spinner_choice2,
	spinner_choice3,
	spinner_choice4,
};


//! \name Menu string declarations
//@{
PROGMEM_DECLARE(char const, main_menu_title[]) = "Menu Title";
PROGMEM_DECLARE(char const, main_menu_1[]) = "Selection 1";
PROGMEM_DECLARE(char const, main_menu_2[]) = "Selection 2";
PROGMEM_DECLARE(char const, main_menu_3[]) = "Selection 3";
PROGMEM_DECLARE(char const, main_menu_4[]) = "Selection 4";
PROGMEM_DECLARE(char const, main_menu_5[]) = "Selection 5";
PROGMEM_DECLARE(char const, main_menu_6[]) = "Selection 6";
//@}

PROGMEM_STRING_T main_menu_strings[] = {
	main_menu_1,
	main_menu_2,
	main_menu_3,
	main_menu_4,
	main_menu_5,
	main_menu_6,
};

static volatile bool show_result = false;
static struct gfx_mono_spinctrl_spincollection spinners;
static volatile int16_t results[GFX_MONO_SPINCTRL_MAX_ELEMENTS_IN_SPINCOLLECTION];

#define OLED_SCREENT_CLEAR()  {gfx_mono_draw_filled_rect(0, 0, GFX_MONO_LCD_WIDTH, GFX_MONO_LCD_HEIGHT, GFX_PIXEL_CLR);}

/**
 * \brief gfx menu example using fake keyboard input
 *
 * This example runs the menu system, goes to a menu item, selects it,
 * and verifies that it has been selected. The example assumes that at
 * least three menu items are available on each page.
 *
 */
static void gfx_menu_example(void)
{
	uint8_t menu_status;
	struct gfx_mono_menu menu_options = {
		.title             = main_menu_title,
		.strings           = main_menu_strings,
		.num_elements      = NUM_OF_ELEMENTS,
		.current_selection = FIRST_ELEMENT
	};

	// Initialize menu system
	gfx_mono_menu_init(&menu_options);

	// Check that current page is first and that current selection is the first
	if( menu_options.current_page != FIRST_PAGE || menu_options.current_selection != FIRST_ELEMENT)
		return;

	// Go down to selection 2
	menu_status = gfx_mono_menu_process_key(&menu_options,
			GFX_MONO_MENU_KEYCODE_DOWN);
	menu_status = gfx_mono_menu_process_key(&menu_options,
			GFX_MONO_MENU_KEYCODE_DOWN);

	// Verify that menu system is IDLE and waiting
	if(menu_status != GFX_MONO_MENU_EVENT_IDLE)
		return;

	// Press enter on the selection
	menu_status = gfx_mono_menu_process_key(&menu_options,
			GFX_MONO_MENU_KEYCODE_ENTER);

	// Check that the menu item has been pressed
	if(menu_status != 2)
		return;

	// Exit the current selection
	menu_status = gfx_mono_menu_process_key(&menu_options,
			GFX_MONO_MENU_KEYCODE_BACK);

	// Check that the menu has returned EXIT status
	if(menu_status != GFX_MONO_MENU_EVENT_EXIT)
		return;

	// Go up a selection, and verify that it is selection 1
	menu_status = gfx_mono_menu_process_key(&menu_options,
			GFX_MONO_MENU_KEYCODE_UP);
	menu_status = gfx_mono_menu_process_key(&menu_options,
			GFX_MONO_MENU_KEYCODE_ENTER);

	// Check that the correct selection has been made
	if(menu_status != 1)
		return;

}

/**
 * \brief gfx graphic library example
 *
 * This example runs the graphic library, draws bitmap, lines, rectangle
 * and circle
 */
static void gfx_graphic_library_example(void)
{
	struct gfx_mono_bitmap smiley;
	struct gfx_mono_bitmap smiley_flash;
	
	// Setup bitmap struct for bitmap stored in RAM
	smiley.type = GFX_MONO_BITMAP_RAM;
	smiley.width = 8;
	smiley.height = 16;
	smiley.data.pixmap = smiley_data;

	// Setup bitmap for bitmap stored in FLASH
	smiley_flash.type = GFX_MONO_BITMAP_PROGMEM;
	smiley_flash.width = 8;
	smiley_flash.height = 16;
	smiley_flash.data.progmem = flash_bitmap;

	// Output bitmaps to display
	gfx_mono_put_bitmap(&smiley, 50, 0);
	gfx_mono_put_bitmap(&smiley_flash, 0, 0);

	//Draw  horizontal an vertical lines with length 128 and 32 pixels
	gfx_mono_draw_vertical_line(1, 0, 32, GFX_PIXEL_SET);
	gfx_mono_draw_horizontal_line(0, 2, 128, GFX_PIXEL_SET);

	// Draw a line from the top-left corner to the bottom right corner
	gfx_mono_draw_line(0, 0, 127, 31, GFX_PIXEL_SET);

	// Draw a rectangle with upper left corner at x=20,y=10 - width=height=10px
	gfx_mono_draw_rect(20, 10, 10, 10,GFX_PIXEL_SET);

	// Draw a 10x10 filled rectangle at x=10,y=10
	gfx_mono_draw_filled_rect(10, 10, 10, 10, GFX_PIXEL_SET);

	// Draw a whole circle at x=50,y=16 with radios=8 and all octants drawn
	gfx_mono_draw_circle(50, 16, 8, GFX_PIXEL_SET,GFX_WHOLE);

	// Draw a filled circle with all quadrant drawn
	gfx_mono_draw_filled_circle(80, 16, 10, GFX_PIXEL_SET, GFX_WHOLE);
	
}

/**
 * \brief OLED1 board button 1 ISR
 *
 * This button is used as UP button, to select spinner and adjust 
 * spinner values
 */
ISR(PORTK_INT0_vect){
	uint8_t switchport;
	cpu_irq_disable();
	delay_ms(10);
	gfx_mono_spinctrl_spincollection_process_key(&spinners, GFX_MONO_SPINCTRL_KEYCODE_UP, (int16_t*)&results[0]);
	/* Make sure button is released before continuing */
	switchport = OLED1_BOARD_BUTTON1_PORT.IN | ~OLED1_BOARD_BUTTON1_PIN_MASK;
	while (switchport != 0xFF) {
		switchport = (OLED1_BOARD_BUTTON1_PORT.IN | ~OLED1_BOARD_BUTTON1_PIN_MASK);
	}
	delay_ms(10);

	/* Clear interrupt that was generated due to bouncing */
	OLED1_BOARD_BUTTON1_PORT.INTFLAGS = PORT_INT0IF_bm;
	cpu_irq_enable();
	
}

/**
 * \brief OLED1 board button 2 ISR
 *
 * This button is used as DOWN button, to select spinner and adjust 
 * spinner values
 */
ISR(PORTA_INT0_vect){
	uint8_t switchport;
	cpu_irq_disable();
	delay_ms(10);
	gfx_mono_spinctrl_spincollection_process_key(&spinners, GFX_MONO_SPINCTRL_KEYCODE_DOWN, (int16_t*)&results[0]);

	/* Make sure button is released before continuing */
	switchport = OLED1_BOARD_BUTTON2_PORT.IN | ~OLED1_BOARD_BUTTON2_PIN_MASK;
	while (switchport != 0xFF) {
		switchport = (OLED1_BOARD_BUTTON2_PORT.IN | ~OLED1_BOARD_BUTTON2_PIN_MASK);
	}
	delay_ms(10);

	/* Clear interrupt that was generated due to bouncing */
	OLED1_BOARD_BUTTON2_PORT.INTFLAGS = PORT_INT0IF_bm;
	cpu_irq_enable();
	
}

/**
 * \brief OLED1 board button 3 ISR
 *
 * This button is used as ENTER button, to enter spinner value selection and 
 * OK button
 */
ISR(PORTA_INT1_vect){
	
	uint8_t switchport;
	uint8_t spinctrl_status;
	
	cpu_irq_disable();
	delay_ms(10);
	
	spinctrl_status = gfx_mono_spinctrl_spincollection_process_key(&spinners, 
						GFX_MONO_SPINCTRL_KEYCODE_ENTER, (int16_t*)&results[0]);
	if(spinctrl_status == GFX_MONO_SPINCTRL_EVENT_FINISH){
		// OK button pressed, display spincollection results
		show_result = true;
	}

	// Make sure button is released before continuing
	switchport = OLED1_BOARD_BUTTON3_PORT.IN | ~OLED1_BOARD_BUTTON3_PIN_MASK;
	while (switchport != 0xFF) {
		switchport = (OLED1_BOARD_BUTTON3_PORT.IN | ~OLED1_BOARD_BUTTON3_PIN_MASK);
	}
	delay_ms(10);

	// Clear interrupt that was generated due to bouncing
	OLED1_BOARD_BUTTON3_PORT.INTFLAGS = PORT_INT1IF_bm;
	cpu_irq_enable();
	
}

/**
 * \brief show spinctrl results when OK button is pressed
 *
 */
void show_spinctrl_result( void )
{
	char results_str[50];
	char temp_str[10],tempchar;
	uint8_t i = 0;
	
	cpu_irq_disable();
	OLED_SCREENT_CLEAR();
	
	// Read spinner string in flash
	uint8_t PROGMEM_PTR_T p = (uint8_t PROGMEM_PTR_T)spinner_choicestrings[results[0]];
	tempchar = PROGMEM_READ_BYTE(p);
	while(tempchar != '\0'){
		temp_str[i++] = tempchar;
		tempchar = PROGMEM_READ_BYTE((uint8_t PROGMEM_PTR_T)(++p));
	}
	
	// Format output
	sprintf(results_str, "Result is:\r\n%s\n%d\r\n%d", temp_str, results[1], results[2]);
	
	// Display results on OLED
	gfx_mono_draw_string(results_str,0, 0, &sysfont);
	delay_s(2);
	cpu_irq_enable();
	
}
/**
 * \brief initialize OLED1 board buttons 
 *
 */
static void button_init( void )
{
	pmic_init();
	
	OLED1_BOARD_BUTTON1_PORT.INTCTRL = PORT_INT0LVL_MED_gc;
	OLED1_BOARD_BUTTON1_PORT.INT0MASK = OLED1_BOARD_BUTTON1_PIN_MASK;
	
	OLED1_BOARD_BUTTON2_PORT.INTCTRL = PORT_INT0LVL_MED_gc;
	OLED1_BOARD_BUTTON2_PORT.INT0MASK = OLED1_BOARD_BUTTON2_PIN_MASK;
	
	OLED1_BOARD_BUTTON3_PORT.INTCTRL |= PORT_INT1LVL_MED_gc;
	OLED1_BOARD_BUTTON3_PORT.INT1MASK = OLED1_BOARD_BUTTON3_PIN_MASK;
	
	cpu_irq_enable();
	
}

/**
 * \brief gfx spinctrl widget example
 *
 * This example runs the gfx spinctrl widget system, using 3 buttons on OLED1 board as 
 * UP, DOWN and ENTER. When OK button is pressed, the spinners value will be displayed
 *
 */
static void gfx_spinctrl_example(void)
{
	struct gfx_mono_spinctrl spinner1;
	struct gfx_mono_spinctrl spinner2;
	struct gfx_mono_spinctrl spinner3;

	// Initialize buttons
	button_init();
	
	// Initialize spinners
	gfx_mono_spinctrl_init(&spinner1, SPINTYPE_STRING, spinnertitle,
	spinner_choicestrings, 0, 3, 0);
	gfx_mono_spinctrl_init(&spinner2, SPINTYPE_INTEGER,
	spinnertitle2, NULL, -60, -41, 0);
	gfx_mono_spinctrl_init(&spinner3, SPINTYPE_INTEGER,
	spinnertitle3, NULL, 19999, 20200, 0);

	// Initialize spincollection
	gfx_mono_spinctrl_spincollection_init(&spinners);

	// Add spinners to spincollection
	gfx_mono_spinctrl_spincollection_add_spinner(&spinner1, &spinners);
	gfx_mono_spinctrl_spincollection_add_spinner(&spinner2, &spinners);
	gfx_mono_spinctrl_spincollection_add_spinner(&spinner3, &spinners);

	// Show spincollection on screen
	gfx_mono_spinctrl_spincollection_show(&spinners);

}

/**
 * \brief gfx system font example
 *
 * This example display strings
 *
 */
static void gfx_system_font_example(void)
{
	gfx_mono_draw_string("My name is\r\nXMEGA-A1U Xplained!\r\nAnd I'm board...",
	0, 0, &sysfont);
}

int main(void){

	board_init();
	sysclk_init();

	/* Initialize GFX lib. Will configure the display controller and
	 * create a framebuffer in RAM if the controller lack two-way communication
	 */
	gfx_mono_init();

#if 0
	// system font demo
	gfx_system_font_example();
	
	// graphic library demo
	delay_s(5);
	OLED_SCREENT_CLEAR();
	gfx_graphic_library_example();
	
	// menu demo
	delay_s(5);
	OLED_SCREENT_CLEAR();
	gfx_menu_example();
#endif
		
	// spinctrl widget demo
	//delay_s(5);
	OLED_SCREENT_CLEAR();
	gfx_spinctrl_example();
	
	while(true) {
		// display spincollection results when OK button pressed
		if(show_result){
			show_result = false;
			// Show spincollection results
			show_spinctrl_result();
		
			// Show spincollection on screen
			gfx_mono_spinctrl_spincollection_show(&spinners);
		}
	}
}
