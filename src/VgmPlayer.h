#pragma once
#include <Sn76489.h>
#include "VgmReader.h"

// delay in microseconds for n+1 sample
// for i in range(1,17):
//     print( "WAIT{}SAMPLE = {};".format( i,round( 1000000/44100 * i ) ) )
constexpr uint32_t WAIT1SAMPLE = 23;
constexpr uint32_t WAIT2SAMPLE = 45;
constexpr uint32_t WAIT3SAMPLE = 68;
constexpr uint32_t WAIT4SAMPLE = 91;
constexpr uint32_t WAIT5SAMPLE = 113;
constexpr uint32_t WAIT6SAMPLE = 136;
constexpr uint32_t WAIT7SAMPLE = 159;
constexpr uint32_t WAIT8SAMPLE = 181;
constexpr uint32_t WAIT9SAMPLE = 204;
constexpr uint32_t WAIT10SAMPLE = 227;
constexpr uint32_t WAIT11SAMPLE = 249;
constexpr uint32_t WAIT12SAMPLE = 272;
constexpr uint32_t WAIT13SAMPLE = 295;
constexpr uint32_t WAIT14SAMPLE = 317;
constexpr uint32_t WAIT15SAMPLE = 340;
constexpr uint32_t WAIT16SAMPLE = 363;

// delay in microseconds for a 1/60 frame of a second (735 samples)
constexpr uint32_t WAIT60TH = 16667;

// delay in microseconds for a 1/50 frame of a second (882 samples)
constexpr uint32_t WAIT50TH = 20000;

// Forward declaration GzUnpacker class (handles gzip decompression)
class GzUnpacker;

class VgmPlayer
{

public:
	VgmPlayer(Sn76489 const &);
	~VgmPlayer();

	void begin();
	void play(char const *filePath);

	void dbgPrint() const;
	void dbgPrint(uint8_t cmd, uint32_t value=0xff) const;

private:
	void play();
	uint8_t _readByte();

	GzUnpacker *m_gzip;
	VgmReader m_vgmReader;
	Sn76489 const &m_psgL;
};
