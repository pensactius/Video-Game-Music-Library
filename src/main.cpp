#include <Arduino.h>
#include "VgmPlayer.h"

//                     LSB                     MSB
//                     D7  D6  D5  D4  D3  D2  D1  D0 ~WE ~CE
#if defined ESP32
Sn76489 psg = Sn76489( 15,  2,  0,  4, 16, 17,  5, 18, 19, 21 );
#elif defined ESP8266
Sn76489 psg = Sn76489( D0, D1, D2, D3, D4, D5, D6, D7, D8 );
#endif

// Create a VGM player with only one PSG chip
VgmPlayer psgPlayer = VgmPlayer(&psg);

void setup() {
	Serial.begin(115200);

	psg.begin();
	psgPlayer.begin();
	psgPlayer.dbgPrint();
	
	psgPlayer.play("/");
}

void loop() {
  // put your main code here, to run repeatedly:
}
