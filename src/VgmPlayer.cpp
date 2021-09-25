#include "VgmPlayer.h"
// Set **destination** filesystem by uncommenting one of these:
#include "config.h"
#include <ESP32-targz.h>


VgmPlayer::VgmPlayer(Sn76489 const &psgL) :
	m_psgL { psgL }
{

}

VgmPlayer::~VgmPlayer()
{

}

// TODO: return error codes
void
VgmPlayer::begin()
{
	// TODO Check error code
	if (!tarGzFS.begin()) Serial.println("tarGzFS init failed!");
	else {
		m_gzip = new GzUnpacker();
		m_gzip->haltOnError( true ); // stop on fail (manual restart/reset required)
		m_gzip->setupFSCallbacks( targzTotalBytesFn, targzFreeBytesFn ); // prevent the partition from exploding, recommended
		m_gzip->setGzProgressCallback( BaseUnpacker::defaultProgressCallback ); // targzNullProgressCallback or defaultProgressCallback
		m_gzip->setLoggerCallback( BaseUnpacker::targzPrintLoggerCallback  );    // gz log verbosity
	}
	if (!m_vgmReader.begin())
	{
		Serial.println("Error mounting file system!");
		return;
	}
}

void
VgmPlayer::play()
{
	bool endOfSoundData = false;

	while (!endOfSoundData) 
	{
		uint8_t cmd = _readByte();
		/*Serial.write('$');
		Serial.print(cmd, HEX); 
		Serial.write(':');*/

		switch (cmd)
		{
			uint8_t dd;

		// 0x4F dd (TODO - NOT IMPLEMENTED)
		// Game Gear PSG stereo, write dd to port 0x06 
		case 0x4f:			
			_readByte();
			break;
		// 0x50 dd
		// SN76489 Write dd value
		case 0x50:
			dd = _readByte();
			m_psgL.writeData(dd);
			//dbgPrint(cmd, dd);
			break;

		// 0x61 nn nn 
		// Wait n samples, n can range from 0 to 65535 (approx 1.49 seconds). 
		// Longer pauses than this are represented by multiple wait commands. 
		case 0x61:
		{
			uint8_t lo = _readByte();
			uint8_t hi = _readByte();
			uint32_t nsamples = (uint32_t)word(hi, lo);
			uint32_t wait = WAIT1SAMPLE * nsamples;
			//uint32_t wait = (uint32_t)((1000000.0 / 44100.0) * nsamples);
			delayMicroseconds(wait);
			//dbgPrint(cmd, wait);
			break;
		}

		// wait 735 samples (60th of a second), a shortcut for 0x61 0xdf 0x02
		case 0x62:
			delayMicroseconds(WAIT60TH);
			//dbgPrint(cmd);
			break;

		// wait 882 samples (50th of a second), a shortcut for 0x61 0x72 0x03
		case 0x63:
			delayMicroseconds(WAIT50TH);
			//dbgPrint(cmd);
			break;			

		// 0x7n
		// wait n+1 samples, n can range from 0 to 15. 
		case  0x70:delayMicroseconds(WAIT1SAMPLE);  /*dbgPrint(cmd, WAIT1SAMPLE);*/break;
		case  0x71:delayMicroseconds(WAIT2SAMPLE);  /*dbgPrint(cmd, WAIT2SAMPLE);*/break;
		case  0x72:delayMicroseconds(WAIT3SAMPLE);  /*dbgPrint(cmd, WAIT3SAMPLE);*/break;
		case  0x73:delayMicroseconds(WAIT4SAMPLE);  /*dbgPrint(cmd, WAIT4SAMPLE);*/break;
		case  0x74:delayMicroseconds(WAIT5SAMPLE);  /*dbgPrint(cmd, WAIT5SAMPLE);*/break;
		case  0x75:delayMicroseconds(WAIT6SAMPLE);  /*dbgPrint(cmd, WAIT6SAMPLE);*/break;
		case  0x76:delayMicroseconds(WAIT7SAMPLE);  /*dbgPrint(cmd, WAIT7SAMPLE);*/break;
		case  0x77:delayMicroseconds(WAIT8SAMPLE);  /*dbgPrint(cmd, WAIT8SAMPLE);*/break;
		case  0x78:delayMicroseconds(WAIT9SAMPLE);  /*dbgPrint(cmd, WAIT9SAMPLE);*/break;
		case  0x79:delayMicroseconds(WAIT10SAMPLE); /*dbgPrint(cmd, WAIT10SAMPLE);*/break;
		case  0x7A:delayMicroseconds(WAIT11SAMPLE); /*dbgPrint(cmd, WAIT11SAMPLE);*/break;
		case  0x7B:delayMicroseconds(WAIT12SAMPLE); /*dbgPrint(cmd, WAIT12SAMPLE);*/break;
		case  0x7C:delayMicroseconds(WAIT13SAMPLE); /*dbgPrint(cmd, WAIT13SAMPLE);*/break;
		case  0x7D:delayMicroseconds(WAIT14SAMPLE); /*dbgPrint(cmd, WAIT14SAMPLE);*/break;
		case  0x7E:delayMicroseconds(WAIT15SAMPLE); /*dbgPrint(cmd, WAIT15SAMPLE);*/break;
		case  0x7F:delayMicroseconds(WAIT16SAMPLE); /*dbgPrint(cmd, WAIT16SAMPLE);*/break;

		// 0x66 End of sound data
		case 0x66:
			endOfSoundData = true;

		default:
			dbgPrint(cmd);
			endOfSoundData = true;
			break;
		}
	}

	m_psgL.muteAll();
}

void
VgmPlayer::play(char const *filePath)
{
	VgmFormat format = m_vgmReader.getFormat(filePath);
	uint8_t err = 0;

	// Exit if file not recognized as VGM file
	if (format == VgmFormat::unknown)
	{	Serial.printf ("Error opening %s, file unknown", filePath);
		return;
	}

	// Uncompress the file to a temporary if it's a compressed VGM
	if (format == VgmFormat::compressed)
	{
		// Decompress the VGM file to a temporary file
		if( !m_gzip->gzExpander(tarGzFS, filePath, tarGzFS, "/tmp.vgm") ) {
    	  Serial.printf("operation failed with return code #%d", m_gzip->tarGzGetError() );
    	}
		err = m_vgmReader.open("/tmp.vgm");
	}
	// Open the file directly if it's an uncompressed VGM
	else if (format == VgmFormat::uncompressed)
	{
	// Parse and play the uncompressed file
		err = m_vgmReader.open(filePath);
	}

	if ( err != 0 )
	{
		Serial.print(F("Couldn't open file "));
		Serial.println(filePath);
	}
	else
	{
		Serial.print(F("Playing "));
		Serial.println(filePath);
		play();
		m_vgmReader.close();
	}	
	

}

void VgmPlayer::dbgPrint() const
{
	m_psgL.dbgPrint();
}

void VgmPlayer::dbgPrint(uint8_t cmd, uint32_t value) const
{
	switch (cmd)
	{
	case 0x50:
		Serial.print("Sn76489 <-- $");
		Serial.println(value, HEX);
		break;

	case 0x62:
		Serial.println("WAIT60TH");
		break;

	case 0x61:	
	case 0x70:
	case 0x71:
	case 0x72:
	case 0x73:
	case 0x74:
	case 0x75:
	case 0x76:
	case 0x77:
	case 0x78:
	case 0x79:
	case 0x7A:
	case 0x7B:
	case 0x7C:
	case 0x7D:
	case 0x7E:
	case 0x7F:
		Serial.print("WAIT ");
		Serial.print(value);
		Serial.println(" microseconds");
		break;
	case 0x66: Serial.println("--END--");break;
	default:
		Serial.printf("UNKNOWN %02X", cmd);
		break;
	}
}


////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////////////
uint8_t VgmPlayer::_readByte()
{
	return m_vgmReader.readByte();
}
