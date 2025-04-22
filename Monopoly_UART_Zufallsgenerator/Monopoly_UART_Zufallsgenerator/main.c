/*
 * Monopoly_UART_Zufallsgenerator.c
 *
 * Created: 10.04.2025 09:31:03
 * Author : e1Schnei
 */ 

#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>

#include "UartDriver.h"
#include "Monopoly_ZufallsgeneratorDriver.h"



//#pragma GCC optimize 0
int main(void)
{
    adm_ADC_init();
    _delay_ms(10);
    init_Uart(8); //103 => 9600       8 => 115200
    
    DDRC = 0xC0;
    PORTC = 0x00;
    
    uint8_t neueDaten = 0;
    uint8_t alteDaten = 0;
    uint8_t flagNeueDaten = 0;
    uint16_t benutzerEingabe = 0;
    uint32_t seed = 0;
    uint32_t adcWert = 0;
    uint8_t modus = 0;
    uint16_t setAdc[4] = {166,167,166,168};
    benutzerEingabe = ausgabeStartSequenz();
    modus = modusAuswahl();
    UART_Text("Es werden ");
    char str[15];
    char ausgabeSeed[50];
    sprintf(str, "%d", benutzerEingabe);
    UART_Text(str);
    UART_Text(" Messungen durchgefuehrt");
    for (uint8_t i = 0; i < 4; i = i + 1)
    {
        UART_Text("\n\r");
    }
    
    
    switch (modus)
    {
        case 1:
        UART_Text("Seed");
        for (uint32_t i = 0; i < benutzerEingabe; i = i + 1)
        {
            UART_Text("\n\r");
            /*sprintf(ausgabeSeed, "%lu", i);
            UART_Text(ausgabeSeed);
            UART_Text(" ");*/
            for (uint8_t j = 0; j < 4; j = j + 1)
            {
                _delay_us(500);
                adcWert = adm_ADC_read(j);
                seed ^= adcWert << 22;
                seed ^= ~adcWert << 12;
                seed ^= adcWert << 2;
            }
            sprintf(ausgabeSeed, "%lu", seed);
            UART_Text(ausgabeSeed);
        }
    	break;
        case 2:
        UART_Text("Nr ADC-0 ADC-1 ADC-2 ADC-3 Seed");
        for (uint32_t i = 0; i < benutzerEingabe; i = i + 1)
        {
            UART_Text("\n\r");
            sprintf(ausgabeSeed, "%lu", i);
            UART_Text(ausgabeSeed);
            UART_Text(" ");
            for (uint8_t j = 0; j < 4; j = j + 1)
            {
                _delay_us(500);
                adcWert = adm_ADC_read(j);
                sprintf(ausgabeSeed, "%lu", adcWert);
                UART_Text(ausgabeSeed);
                UART_Text(" ");
                seed ^= adcWert << 22;
                seed ^= ~adcWert << 12;
                seed ^= adcWert << 2;
            }
            sprintf(ausgabeSeed, "%lu", seed);
            UART_Text(ausgabeSeed);
        }        
        break;
        case 3:
        UART_Text("Seed");
        for (uint32_t i = 0; i < benutzerEingabe; i = i + 1)
        {
            UART_Text("\n\r");
            for (uint8_t j = 0; j < 4; j = j + 1)
            {
                _delay_us(500);
                adcWert = setAdc[i];
                seed ^= adcWert << 22;
                seed ^= ~adcWert << 12;
                seed ^= adcWert << 2;
                UART_Text("\n\r");
                sprintf(ausgabeSeed, "%lu", j);
                UART_Text(ausgabeSeed);
            }
            UART_Text("\n\r");
            UART_Text("\n\r");
            UART_Text("\n\r");
            UART_Text("\n\r");
            sprintf(ausgabeSeed, "%lu", seed);
            UART_Text(ausgabeSeed);
            _delay_ms(5000);
        }
    }
    
    while (1) 
    {
        PORTC = 0xC0;
    }
}

