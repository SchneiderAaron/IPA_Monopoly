/*
 * Monopoly_ZufallsgeneratorDriver.h
 *
 * Created: 10.04.2025 10:19:03
 *  Author: e1Schnei
 */ 


#ifndef MONOPOLY_ZUFALLSGENERATORDRIVER_H_
#define MONOPOLY_ZUFALLSGENERATORDRIVER_H_
#define ASCII_Verschiebung 48
uint16_t ausgabeStartSequenz(void);
uint8_t modusAuswahl(void);
void adm_ADC_init(void);
uint16_t adm_ADC_read(uint8_t kanal);


#endif /* MONOPOLY_ZUFALLSGENERATORDRIVER_H_ */