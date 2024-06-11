#include <Arduino.h>
#include "VgmPlayer.h"

// In the Sound Chip Library the MSB is right: D7 is MSB,
// but chip sn76489 has data pins reversed, D0 is MSB!
//                     MSB                     LSB
//                     D0  D1  D2  D3  D4  D5  D6  D7 ~WE ~CE
Sn76489 psg = Sn76489( 15,  2,  0,  4, 16, 17,  5, 18, 19, 21 );
Sn76489 psgR = Sn76489( 15,  2,  0,  4, 16, 17,  5, 18, 32, 33 );

// Create a VGM player with two PSG chips
VgmPlayer psgPlayer = VgmPlayer(&psg, &psgR);

void setup() {
	Serial.begin(115200);

	psgPlayer.begin();
	psgPlayer.dbgPrint();
	
	psgPlayer.play("/");
}

void loop() {
  // put your main code here, to run repeatedly:
}
