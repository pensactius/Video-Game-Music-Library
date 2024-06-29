#include "VgmPlayer.h"
#include <Arduino.h>

//             LSB                         MSB
//              D7  D6  D5  D4  D3  D2  D1  D0 ~IC ~CS ~WE ~Ao
Ym2413 ym2413 { 19, 18, 17, 16, 15, 14, 13, 12, 2, 23, 5, 0 };
//                     LSB                     MSB
//                     D7  D6  D5  D4  D3  D2  D1  D0 ~WE ~CE
Sn76489 psg = Sn76489 { 19, 18, 17, 16, 15, 14, 13, 12, 5, 4 };

VgmPlayer player = VgmPlayer(&psg, nullptr, &ym2413);

void setup()
{
    Serial.begin(115200);

    Serial.printf("Running at %iMhz\n", getCpuFrequencyMhz());
    player.begin();

    player.play("/sn76489/Aladdin02.vgm");
    player.play("/ym2413/Domingo.vgm");
}

void loop()
{
    // put your main code here, to run repeatedly:
}
