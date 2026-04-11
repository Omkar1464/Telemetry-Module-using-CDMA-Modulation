#include <Arduino.h>

// ===================================================================== 
// 1. Q-CHANNEL SPREADING CODE (Length 10)
// ===================================================================== 
const uint8_t Q_CDMA_CODE[10] = { 1,0,1,0,1,0,1,0 };
// ===================================================================== 
// 2. SETUP
// ===================================================================== 
void setup() { 
    // Set Pin 10 as OUTPUT (PORTB, bit 2)
    DDRB |= (1 << DDB2); 
} 

// ===================================================================== 
// 3. THE TRANSMISSION LOOP
// ===================================================================== 
void loop() { 
    // Dummy sensor data payload (10101010)
    uint8_t payload_byte = 0b10101010;
    
    // Shut down Arduino background tasks for perfect timing
    noInterrupts(); 
    
    // Infinite continuous transmission
    while(true) {
        
        // Loop through the 8 bits of the payload data
        for (int bit_index = 0; bit_index < 8; bit_index++) { 
            uint8_t data_bit = (payload_byte >> (7 - bit_index)) & 0x01; 
            
            // Loop through the 10-chip CDMA sequence
            for (int chip_index = 0; chip_index < 10; chip_index++) {
                
                uint8_t prn_chip = Q_CDMA_CODE[chip_index];
                uint8_t modulated_chip = data_bit ^ prn_chip;

                // Direct Port Manipulation: Output to Pin 10 instantly
                if (modulated_chip) { 
                    PORTB |= (1 << PORTB2); 
                } else { 
                    PORTB &= ~(1 << PORTB2); 
                } 
                
                // ---------------------------------------------------
                // THE TIMING CALIBRATION
                // ~159 clock cycles to hit 100 kHz chip rate
                // (10 kHz Data Rate * 10 Chips = 100,000 chips/sec)
                // ---------------------------------------------------
                //__builtin_avr_delay_cycles(80); 
                delayMicroseconds(5);
            }
        } 
    }
}