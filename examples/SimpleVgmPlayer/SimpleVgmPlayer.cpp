#include "VgmPlayer.h"
#include <Arduino.h>

//                     LSB                     MSB
//                     D7  D6  D5  D4  D3  D2  D1  D0 ~WE ~CE
Sn76489 psg = Sn76489 { 19, 18, 17, 16, 15, 14, 13, 12, 5, 4 };

VgmPlayer psgPlayer = VgmPlayer(&psg, nullptr, nullptr);

void setup()
{
    Serial.begin(115200);

    Serial.printf("Running at %iMhz\n", getCpuFrequencyMhz());
    psg.begin();
    psgPlayer.begin();

    psgPlayer.play("/sn76489/Lemming1.vgm");
    psgPlayer.play("/sn76489/Lemming7.vgm");
}

void loop()
{
    // put your main code here, to run repeatedly:
}
