#include <Arduino.h>

// ===================================================================== 
// 1. Q-CHANNEL SPREADING CODE (Length 10)
// ===================================================================== 
const uint8_t Q_CDMA_CODE[10] = { 1, 0, 0, 1, 0, 1, 1, 1, 0, 1};

// ===================================================================== 
// 2. SETUP
// ===================================================================== 
void setup() { 
    // Set Pin 10 as OUTPUT (PORTB, bit 2)
    DDRB |= (1 << DDB2); 
} 

// ===================================================================== 
// 3. THE TRANSMISSION LOOP (Strict 100 kHz Chip Rate)
// ===================================================================== 
void loop() { 
    uint8_t payload_byte = 0b10101010;
    
    // Disable interrupts so background timers don't cause jitter
    noInterrupts(); 
    
    while(true) {
        for (int bit_index = 0; bit_index < 8; bit_index++) { 
            
            uint8_t data_bit = (payload_byte >> (7 - bit_index)) & 0x01; 
            
            for (int chip_index = 0; chip_index < 10; chip_index++) {
                
                uint8_t prn_chip = Q_CDMA_CODE[chip_index];
                uint8_t modulated_chip = data_bit ^ prn_chip;

                // 1-Cycle Direct Port Manipulation (Pin 10)
                if (modulated_chip) { 
                    PORTB |= (1 << PORTB2); 
                } else { 
                    PORTB &= ~(1 << PORTB2); 
                } 
                
                // ---------------------------------------------------
                // EXACT TIMING CALIBRATION
                // 100 kHz = 10 microseconds = 160 clock cycles @ 16MHz
                // Math & port manipulation above takes ~20 cycles.
                // Delay exactly 140 cycles to achieve a perfect 100 kHz.
                // ---------------------------------------------------
               //delayMicroseconds(9.8);
               __builtin_avr_delay_cycles(155); 
            }
        } 
    }
}