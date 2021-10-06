#pragma once

#include "FileSystem.h"

// Possible VGM file types
enum class VgmFormat { unknown, compressed, uncompressed };

// Relative offset to loop point
constexpr uint32_t LOOP_OFFSET = 0x1C;

// Relative offset to GD3 data
constexpr uint32_t GD3_OFFSET = 0x14;

// Relative offset to VGM data stream.
constexpr uint32_t VGM_DATA_OFFSET = 0x34;

struct VgmHeader {
	uint32_t id			= 0;
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

	void		begin(FileSystem* fs_ptr);
	bool		isVgmFile() const;
	void		parseHeader();
	VgmFormat	getFormat() const;
	void		end();

private:
	/*void parseGd3Info();*/

	FileSystem* m_fs_ptr;
	uint32_t	m_gd3Offset;
	uint32_t	m_dataOffset;
	uint32_t	m_loopOffset;
	uint32_t	m_dataLength;
};
