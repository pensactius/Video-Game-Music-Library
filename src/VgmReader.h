#pragma once

#include "Config.h"
#include <Arduino.h>

#if defined ESP32
  #if defined DEST_FS_USES_SPIFFS
    #include <SPIFFS.h>
  #endif
#elif defined ESP8266
    #if defined DEST_FS_USES_LITTLEFS
      #include <LittleFS.h>
    #else // use core SPIFFS
        #include <FS.h>
    #endif
#endif


// Error codes when opening VGM files
constexpr uint8_t ERR_NOERROR = 0;
constexpr uint8_t ERR_OPENFILE = 1;
constexpr uint8_t ERR_FILEUNKNOWN = 2;

// Possible VGM file types
enum class VgmFormat { unknown, compressed, uncompressed };

// Relative offset to loop point
constexpr uint32_t LOOP_OFFSET = 0x1C;

// Relative offset to GD3 data
constexpr uint32_t GD3_OFFSET = 0x14;

// Relative offset to VGM data stream. 
constexpr uint32_t VGM_DATA_OFFSET = 0x34;

#if defined ESP8266
constexpr size_t BUFSIZE=250;
#elif defined ESP32
constexpr size_t BUFSIZE=4096;
#endif

struct VgmHeader {
	uint32_t  id		= 0;
	uint32_t offset		= 0;
	uint32_t version	= 0;
	uint32_t clkSn76489	= 0;
	uint32_t clkYm2413	= 0;
	uint32_t gd3Offset	= 0;
	uint32_t totalSamples = 0;
	uint32_t loopOffset	= 0;
};

class VgmReader
{

public:
	VgmReader();
	~VgmReader();

	bool	begin();
	uint8_t	open(char const *filePath);
	bool	isDir(char const *pathName);
	uint8_t	openDir(char const *dirPath);
	bool	openNextFile();
	void	close();
	bool	delFile(char const *fileName);
	bool	isVgmFile() const;
	char const *getFileName() const;
	void	parseHeader();
	byte	readByte();
	VgmFormat getFormat() const;
	
private:
	void	_reset();	
	size_t	_readFromFile(uint32_t offset);
	void	_dbgPrintBuffer();
	/*void parseGd3Info();*/

	File		m_file;
#if defined ESP32
	File		m_dir;
#elif defined ESP8266	
	Dir			m_dir;
#endif
	
	uint32_t	m_gd3Offset;
	uint32_t	m_dataOffset;
	uint32_t	m_loopOffset;
	uint32_t	m_dataLength;

	uint8_t		*m_buf;//[BUFSIZE];
	uint32_t	m_bufCursor;
	uint32_t	m_fileCursor;
};
