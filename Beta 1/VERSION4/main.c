/*
 * ece295_I2C_tutorial.c
 *
 * Created: 9/2/2021 5:38:25 PM
 * Author : ingrid, leo, dwight
 * Newest Version of Master Main - created 1pm Monday
 */ 



// with uart  (bad copy) (khz) Most Recent

#define F_CPU		8000000UL // Clock speed
#define SCREEN_ADDR 0x78

#include <avr/io.h>
#include <util/delay.h> //for delay function
#include <stdio.h>

#include "i2c.h"
#include "screen_cmds.h"
#include "si5351a.h"    

#include <avr/interrupt.h>
#include <math.h>
#include <string.h>

#define TRUE 0x1
#define FALSE 0x0

#define ROW1 0x1
#define ROW2 0x2
#define ROW3 0x4
#define ROW4 0x8
#define COL1 0x1
#define COL2 0x2
#define COL3 0x4
#define COL4 0x8

#define KEY_FREQSEL 0xa
#define KEY_TOGGLE 0xb
#define KEY_ENTER 0xc
#define KEY_CANCEL 0xd
#define KEY_HASHTAG 0xe
#define KEY_ASTERISK 0xf
#define KEY_UNKNOWN 0x10

#define KEY_DEBOUNCE_TIME 200 //200 ms



int is_entering = FALSE;
int freq = 0;
char freq_input[99];

int TXEN = FALSE;

int UPDATE_LCD = TRUE;
int UPDATE_PLL = FALSE; // if true will reset the PLL frequency
int UPDATE_TXRX_SWITCH = TRUE;



unsigned char received_char_int = '\0';
int RX_FLAG = FALSE;
char received_str[100];
int rx_counter = 0;

void screen_write_string(char string_to_write[], int line_num)
{
	int letter=0;

	I2Csendcmd(SCREEN_ADDR, line_num);
	int current_line = line_num;
	
	for(letter = 0; string_to_write[letter] != 0; letter++)	{
		if ((letter != 0) && (letter % LINE_LENGTH == 0))
		{
			if (current_line == COMMAND_SET_CURSOR_LINE_4){
				current_line = line_num;// We've gone past the end of the line, go to the next one
				I2Csendcmd(SCREEN_ADDR, COMMAND_CLEAR_DISPLAY);
			}
			else
			current_line = current_line+0x20;
			I2Csendcmd(SCREEN_ADDR, current_line); // We've gone past the end of the line, go to the next one
		}
		I2Csenddatum(SCREEN_ADDR, string_to_write[letter]);
	}
}

int getInputChar(int row, int col){
	if (row == ROW1){
		if(col == COL1){
			return 1;
		}
		else if(col == COL2){
			return 2;
		}
		else if(col == COL3){
			return 3;
		}
		else if(col == COL4){
			return KEY_FREQSEL;
		}
		else {
			return KEY_UNKNOWN;
		}
	}
	else if (row == ROW2){
		if(col == COL1){
			return 4;
		}
		else if(col == COL2){
			return 5;
		}
		else if(col == COL3){
			return 6;
		}
		else if(col == COL4){
			return KEY_TOGGLE;
		}
		else {
			return KEY_UNKNOWN;
		}
	}
	else if (row == ROW3){
		if(col == COL1){
			return 7;
		}
		else if(col == COL2){
			return 8;
		}
		else if(col == COL3){
			return 9;
		}
		else if(col == COL4){
			return KEY_ENTER;
		}
		else {
			return KEY_UNKNOWN;
		}
	}
	else if (row == ROW4){
		if(col == COL1){
			return KEY_ASTERISK;
		}
		else if(col == COL2){
			return 0;
		}
		else if(col == COL3){
			return KEY_HASHTAG;
		}
		else if(col == COL4){
			return KEY_CANCEL;
		}
		else {
			return KEY_UNKNOWN;
		}
	}
	else {
		return KEY_UNKNOWN;
	}

}

//Computer
void UART_Init(){
	UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0);

	//UCSR0B = (1 << RXEN0) | (1 << TXEN0);
	UCSR0C = (1<<UCSZ01) | (1<<UCSZ00);   // Async-mode, 8 data bits, 1 stop bit, no parity bit
	UCSR0A = 0x0; // Clear the UASRT status register
	
	//Set Baud Rate
	UBRR0L = 51; // 9600 at 8 MHz
	UBRR0H = 0;
}

void TX_Char(char c){
	while(!(UCSR0A & (1<<UDRE0))); // Check transmission buffer empty flag UDRE0

	// Transmit data
	UDR0 = c;
}

char* RX_String(){
	int i = 0;
	//while()
}

void TX_String(char* str){
	char* ptr = str;
	
	while(*ptr != '\0'){
		TX_Char(*ptr);
		ptr++;
	}
	
	TX_Char(';');
}

char RX_Char(){
	while(!(UCSR0A & (1<<RXC0))); //Check receive completed flag RXC0

	// Receive data
	return UDR0;
}
// Computer

int main(void)
{
	CLKPR = 1 << CLKPCE;
	CLKPR = 0;

	//LCD Part
	DDRA = 0xff; //PortA as output (only need PA0 for display)
	
	_delay_ms(5);
	PORTA = PORTA | (0<<PA0); // turn off
	_delay_ms(200);
	PORTA = PORTA | (1<<PA0); // turn on display
	_delay_ms(5);
	
	//Set up I2C
	i2cInit();
	//I2Cstart();
	
	
	//TESTING 
	i2cSendRegister(SI_CLK0_CONTROL, 0x0);
	
	
	//Initialize display
	I2Csendcmd(SCREEN_ADDR, 0x01); // clear display
	I2Csendcmd(SCREEN_ADDR, 0x3A);
	I2Csendcmd(SCREEN_ADDR, 0x09);
	I2Csendcmd(SCREEN_ADDR, 0x06);
	I2Csendcmd(SCREEN_ADDR, 0x1E);
	I2Csendcmd(SCREEN_ADDR, 0x39);
	I2Csendcmd(SCREEN_ADDR, 0x1B);
	I2Csendcmd(SCREEN_ADDR, 0x6E);
	I2Csendcmd(SCREEN_ADDR, 0x57);
	I2Csendcmd(SCREEN_ADDR, 0x7A);
	I2Csendcmd(SCREEN_ADDR, 0x38);
	I2Csendcmd(SCREEN_ADDR, 0x0F);
	I2Csendcmd(SCREEN_ADDR, 0x01); // clear display
	
	//TXRX_Switch Part
		DDRD = 0x40;
	
	//Keypad Part
		DDRB = 0xf0; //Set ports B 4-7 as output, 0-3 as input
		
		PCICR = 0x2; //Setting PCICR (ATMEGA's GIC) to enable interrupts from PCINT1
		PCMSK1 = 0xf; //Setting interrupt masks of pins PCINT8-11 to 1
		sei(); //Enable interrupts by setting I bit of the Status Register to 1
		
	// Computer Interface Part
		UART_Init();
	
	printf("enter the while loop");
    //Write some data
	    while (1)
	    {
		    PORTB = 0xf0; //ToDo will this matter if PORTB[3:0] are set to input?
		    // ToDo Store and restore SREG
			if(UPDATE_LCD){
				
				//TESTING
				i2cSendRegister(SI_CLK0_CONTROL, 0x0);
				
				
				I2Csendcmd(SCREEN_ADDR, 0x01); // clear display
				if(is_entering){
					//strcat(freq_input, "Hhello");
					screen_write_string(freq_input, COMMAND_SET_CURSOR_LINE_4);
				}
				else {
					char freq_disp[99] = "FREQ: ";	
					char freq_val[99];
					itoa(freq, freq_val, 10);
					strcat(freq_disp, freq_val);
					strcat(freq_disp, "M");
					//set PLL frequency
					screen_write_string(freq_disp, COMMAND_SET_CURSOR_LINE_1);
				}
			
				//ToDo output TXEN
				char mode_disp[99] = "MODE: ";
				if(TXEN){
					strcat(mode_disp, "TX");
				}
				else{
					strcat(mode_disp, "RX");
				}
				screen_write_string(mode_disp, COMMAND_SET_CURSOR_LINE_3);

				UPDATE_LCD = FALSE;
			}
	
			if(UPDATE_PLL){

                // old code (using github files)
				//char PLL_disp[30] = "PLL";
				//screen_write_string(PLL_disp, COMMAND_SET_CURSOR_LINE_4);
				//si5351_init();
				//si5351_set_pll(900000000, SI5351_PLLA);
				//si5351_set_freq(freq*1000000, 0, SI5351_CLK0);
				
				
				//si5351_set_pll(900000000, SI5351_PLLB);
				//si5351_set_freq(freq*1000000, 900000000, SI5351_CLK1);
				
				//UPDATE_PLL = FALSE;

                // si5351aSetFrequency(14000000); 
                // si5351aSetFrequency(freq*1000000); 
				
				//i2cInit();
				si5351aSetFrequency(freq*1000000);
				UPDATE_PLL = FALSE; 
			
			}
			
			if(UPDATE_TXRX_SWITCH){
				PORTD = (PORTD & 0xbf) | (TXEN ? 0x0 : 0x40);
				UPDATE_TXRX_SWITCH = FALSE;
			}
		}
}

ISR(PCINT1_vect){
	int key_row = 0;
	int key_col = 0;
	int input_char = KEY_UNKNOWN;
	
	
	_delay_ms(KEY_DEBOUNCE_TIME);
	
	//Return if no key is pressed
	if((PINB & 0xf) == 0){ //No key is pressed
		PORTD = (PORTD & 0xc3) | 0;
		return;
	}
	
	//Obtain row of key press
	key_row = (PINB & 0xf);

	//Swap input and output
	DDRB = 0xf;
	PORTB = 0xf;
	_delay_ms(KEY_DEBOUNCE_TIME);

	//Return if key is released in the middle of detection
	if((PINB & 0xf0) == 0){
		DDRB = 0xf0;
		PORTB |= 0xf0;
		PORTD = (PORTD & 0xc3) | 0;
		return;
	}
	
	//Obtain column of key press
	key_col = (PINB & 0xf0) >> 0x4;
	
	
	
	//Parse input
	input_char = getInputChar(key_row, key_col);

	//Numeric keys
	if(input_char <= 0x9){
		if (!is_entering){ //User just started entering frequency setting
			char s[10];
			itoa(input_char, s, 10);
			strcat(freq_input, s);
			is_entering = TRUE;
		}
		else { //User is entering second or third digit go frequency setting
			char s[10];
			itoa(input_char, s, 10);
			strcat(freq_input, s);
		}
		UPDATE_LCD = TRUE;
	}
	else if(input_char == KEY_CANCEL){
		is_entering = FALSE;
		freq_input[0] = '\0';
		UPDATE_LCD = TRUE;
	}
	else if(input_char == KEY_ENTER){
		is_entering = FALSE;
		freq = atoi(freq_input); //ToDo: display error when input > 4 digits
		freq_input[0] = '\0';
		UPDATE_LCD = TRUE; //Redundant
		UPDATE_PLL = TRUE; //the only place PLL will be updated
	}
	else if(input_char == KEY_TOGGLE){
		TXEN = !TXEN;
		UPDATE_LCD = TRUE;
		UPDATE_TXRX_SWITCH = TRUE;
	}
	
	//Swap input and output back
	DDRB = 0xf0;
	PORTB |= 0xf0;
}



ISR(USART0_RX_vect){
	received_char_int = UDR0;
	if(received_char_int != ';'){
		received_str[rx_counter] = received_char_int;
		rx_counter++;
	}
	else{
		received_str[rx_counter] = ';';
		received_str[rx_counter + 1] = '\0';
		rx_counter = 0;
		
		// Parse instruction
		char inst[3];
		strncpy(inst, received_str, 2);
		inst[2] = '\0';
		int respond = (received_str[2] == ';') ? TRUE : FALSE;
		
		char str_to_send[100];
		str_to_send[0] = '\0';
		
		if(strcmp(inst, "TX") == 0){ //TX Command
			if(respond){
				strcat(str_to_send, "TX");
				char c[2];
				c[0] = TXEN ? '1' : '0';
				c[1] = '\0';
				strcat(str_to_send, c);
				TX_String(str_to_send);
			}
			else{
				char TXEN_to_set = received_str[2];
				TXEN = (TXEN_to_set == '1') ? TRUE : FALSE;
				UPDATE_LCD = TRUE;
				UPDATE_TXRX_SWITCH = TRUE;
				//TX_String("Set TXEN!");
			}
		}
		else if(strcmp(inst, "FA") == 0){
			if(respond){
				strcat(str_to_send, "FA");
				char c[10];
				c[0] = '\0';
				itoa(freq, c, 10);
				for (int i = 0; i < 3 - strlen(c); i++){
					strcat(str_to_send, "0");
				}
				strcat(str_to_send, c);
				strcat(str_to_send, "000000");
				TX_String(str_to_send);
			}
			else{
				char freq_to_set[10];
				strncpy(freq_to_set, received_str + 2, 3);
				freq_to_set[3] = '\0';
				freq = atoi(freq_to_set);
				UPDATE_LCD = TRUE;
				UPDATE_PLL = TRUE;
			}
		}
		else if(strcmp(inst, "IF") == 0){
			strcat(str_to_send, "IF000");
			char c[10];
			c[0] = '\0';
			itoa(freq, c, 10);
			for (int i = 0; i < 3 - strlen(c); i++){
				strcat(str_to_send, "0");
			}
			strcat(str_to_send, c);
			strcat(str_to_send, "0000000000000000000");
			TX_String(str_to_send);
		}
		else if(strcmp(inst, "SP") == 0){
			TX_String("Never Gonna Give You Up!");
		}
		
		received_str[0] = '\0';
	}
}