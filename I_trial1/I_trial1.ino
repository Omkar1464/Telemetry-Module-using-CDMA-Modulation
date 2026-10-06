#include <Arduino.h>

uint8_t I_CDMA_PACKED[125]; 

void setup() { 
    // Fill the array with the perfect 10101010 square wave pattern
    for (int i = 0; i < 125; i++) {
        I_CDMA_PACKED[i] = 0xAA; 
    }

    // Set Pin 10 as OUTPUT
    DDRB |= (1 << DDB2); 
} 

void loop() { 
    // -------------------------------------------------------------
    // SHUT DOWN THE ARDUINO CORE
    // This stops Timer0, millis(), and Serial. The CPU is now 100% 
    // dedicated to your pins. (delayMicroseconds still works!)
    // -------------------------------------------------------------
    noInterrupts(); 

    // INFINITE LOOP: Never let the sequence stop, no gaps!
    while(true) {
        
        for (int byte_index = 0; byte_index < 125; byte_index++) {
            
            uint8_t current_prn_byte = I_CDMA_PACKED[byte_index]; 
            
            for (int bit_pos = 7; bit_pos >= 0; bit_pos--) {
                
                uint8_t prn_chip = (current_prn_byte >> bit_pos) & 0x01;
                
                // XOR with a dummy '1' data bit
                uint8_t modulated_chip = 1 ^ prn_chip;

                // Output to Pin 10 instantly
                if (modulated_chip) { 
                    PORTB |= (1 << PORTB2); 
                } else { 
                    PORTB &= ~(1 << PORTB2); 
                } 
                
                // Tweak this number (try 8, 9, or 10) until the DSO 
                // shows a rock-solid 50.0 kHz!
               delayMicroseconds(4.5);
                // This tells the CPU to wait an EXACT number of clock cycles.
// 16 clock cycles = 1 microsecond.
// 152 cycles = 9.5 microseconds.
//__builtin_avr_delay_cycles(160); 
            }
        } 
    }
}