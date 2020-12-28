#define F_CPU 8000000UL

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define DS PB0
#define ST_CP PB1
#define SH_CP PB2
#define PWM PB3

uint16_t percent[11] = {0x00, 0x200, 0x300, 0x380, 0x3C0, 0x3E0, 0x3F0, 0x3F8, 0x3FC, 0x3FE, 0x3FF};
uint8_t shift=0;

void setup(void) {
  DDRB |= (1 << DS) | (1 << ST_CP) | (1 << SH_CP) | (1 << PWM);
  PORTB = 0x00;

  ADMUX = 0x22; // опорное напряжение - VCC, левое ориентирование данных, выбран вход ADC2 (на нём висит перем. резистор)
  ADCSRA = 0xE2; // АЦП включен, запуск преобразования, режим автоизмерения, прерывание по окончанию преобразования, частота CLK/4
  ADCSRB = 0x00; // режим автоизмерения: постоянно запущено
  DIDR0 |= (1 << PB4); // запрещаем цифровой вход на ноге аналогового входа

  TCCR0A = 0X00;
  TCCR0B = 0X00;
  TCCR0B = (1 << CS01) | (1 << CS00);
  TIMSK0 = (1 << OCIE0A) | (1 << TOIE0);
  
  sei();
  
}

ISR(TIM0_COMPA_vect) {
  if (ADCH > 5) { 
    PORTB |= (1 << PWM);
  } else {
    PORTB &= ~(1 << PWM);
  }
}

ISR(TIM0_OVF_vect) {
  OCR0A = (255 - ADCH);
  if (ADCH < 250) { 
    PORTB &= ~(1 << PWM);
  } else {
    PORTB |= (1 << PWM);
  }
}

void shiftOut(uint16_t word) {
    word = (word << 6);
    PORTB &= ~(1 << DS);
    PORTB &= ~(1 << ST_CP);
    PORTB &= ~(1 << SH_CP);
    _delay_us(5);
    for (uint8_t i = 0; i < 16; i++) { 
        if ((1 << i) & (word)) {
            PORTB |= (1 << DS);
        } else {
            PORTB &= ~(1 << DS);
        };
        _delay_us(5);        
        PORTB |= (1 << SH_CP);
        _delay_us(5);
        PORTB &= ~(1 << DS);
        _delay_us(5);
        PORTB &= ~(1 << SH_CP);
    }
    PORTB |= (1 << ST_CP);
    _delay_us(5);
    PORTB &= ~(1 << ST_CP);
    _delay_us(5);
    PORTB &= ~(1 << DS);
    PORTB &= ~(1 << ST_CP);
    PORTB &= ~(1 << SH_CP);
}


void main(void) {
    setup();
        int count = 0,
            d = 1;
    while (1) {
        int ai = ADCH << 2;
        if (ai < 2) { shift = 0;} 
        if ((ai >= 2)&&(ai < 102)) { shift = 1;} 
        if ((ai >= 102)&&(ai < 204)) { shift = 2;}
        if ((ai >= 204)&&(ai < 306)) { shift = 3;}
        if ((ai >= 306)&&(ai < 409)) { shift = 4;}
        if ((ai >= 409)&&(ai < 512)) { shift = 5;}
        if ((ai >= 512)&&(ai < 613)) { shift = 6;}
        if ((ai >= 613)&&(ai < 716)) { shift = 7;}
        if ((ai >= 716)&&(ai < 818)) { shift = 8;}
        if ((ai >= 818)&&(ai < 920)) { shift = 9;}
        if (ai >= 920){ shift = 10;}

        if (shift > 10) {
            shift = 10;
        }
        if (shift < 0) {
            shift = 0;
        }     
        shiftOut(percent[shift]);
        _delay_ms(100);
    }
}



