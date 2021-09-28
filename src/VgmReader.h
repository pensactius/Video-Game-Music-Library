#pragma once

#include "Config.h"
#include <Arduino.h>

// The following section is taken from the ESP32-targz library
#if defined ESP32
  #if defined DEST_FS_USES_SPIFFS
    #include <SPIFFS.h>
	#define destFS SPIFFS
  #endif
#elif defined ESP8266
    #if defined DEST_FS_USES_LITTLEFS
      #include <LittleFS.h>
      #define destFS LittleFS
    #elif defined DEST_FS_USES_SPIFFS
      #if defined USE_LittleFS // emulate SPIFFS using LittleFS
        #include <LittleFS.h>
        #define destFS SPIFFS
      #else // use core SPIFFS
        #include <FS.h>
        #define destFS SPIFFS
      #endif
    #else // no destination filesystem defined in sketch
      #warning "Unspecified or invalid destination filesystem, please #define one of these before including the library: DEST_FS_USES_SPIFFS, DEST_FS_USES_LITTLEFS, DEST_FS_USES_SD, DEST_FS_USES_PSRAMFS"
      // however, check for USE_LittleFS as it is commonly defined since SPIFFS deprecation
      #if defined USE_LittleFS
        #include <LittleFS.h>
        #define destFS LittleFS
        #warning "Defaulting to LittleFS"
        #define DEST_FS_USES_LITTLEFS
      #else
        #include <FS.h>
        #define destFS SPIFFS
        #warning "Defaulting to SPIFFS (soon deprecated)"
        #define DEST_FS_USES_SPIFFS
      #endif
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
	// TODO: Use Dir object where available (ESP8266)
	File		m_dir;

	uint32_t	m_gd3Offset;
	uint32_t	m_dataOffset;
	uint32_t	m_loopOffset;
	uint32_t	m_dataLength;

	uint8_t		*m_buf;
	uint32_t	m_bufCursor;
	uint32_t	m_fileCursor;
};
