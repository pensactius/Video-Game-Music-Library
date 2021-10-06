#pragma once

#include "Config.h"
#include <Arduino.h>

// Error codes when opening files
enum class FileError {
	kNoError, kOpenError, kUnknownFile
 };

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

#if defined ESP8266
constexpr size_t BUFSIZE=250;
#elif defined ESP32
constexpr size_t BUFSIZE=4096;
#endif

class FileSystem
{

public:
	FileSystem();
	~FileSystem();

	bool		begin();
	FileError	open(char const *filePath);
	bool		isDir(char const *pathName);
	FileError	openDir(char const *dirPath);
	bool		openNextFile();
	void		close();
	bool		delFile(char const *fileName);
	char const *getFileName() const;

	bool		seek(uint32_t pos);
	size_t		read(uint8_t* buf, size_t size);
	byte		readByte();
	void		setFileCursor(uint32_t pos);

private:
	size_t		readFromFile(uint32_t offset);
	void		dbgPrintBuffer();

	File		m_file;
	// TODO: Use Dir object where available (ESP8266)
	File		m_dir;

	uint8_t*	m_buf_ptr;
	uint32_t	m_bufCursor;
	uint32_t	m_fileCursor;
};

