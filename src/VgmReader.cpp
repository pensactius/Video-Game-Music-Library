#include "VgmReader.h"

VgmReader::VgmReader()
{
	m_buf = (byte*)malloc(BUFSIZE);
	if (!m_buf)
	{
		for(;;);
	}
}

VgmReader::~VgmReader()
{
	free(m_buf);
}

bool
VgmReader::begin()
{
#if defined DEST_FS_USES_SPIFFS
	return SPIFFS.begin();
#elif defined DEST_FS_USES_LITTLEFS
	return LittleFS.begin();
#endif
}

uint8_t
VgmReader::open(char const *filePath)
{
	uint8_t error_code = ERR_NOERROR;
	
	_reset();
#if defined DEST_FS_USES_SPIFFS
	m_file = SPIFFS.open(filePath, "r");	
#elif defined DEST_FS_USES_LITTLEFS
	m_file = LittleFS.open(filePath, "r");
#endif

	if (!m_file)
	{
		error_code = ERR_OPENFILE;
	}
	_parseHeader();

	m_dataLength = m_gd3Offset - m_dataOffset;
	m_fileCursor = m_dataOffset;

	return error_code;
}

void
VgmReader::close()
{
	m_file.close();
	_reset();
}

VgmFormat
VgmReader::getFormat(char const *filePath)
{
	struct {
		byte m0;
		byte m1;
		byte m2;
		byte m3;
	} vgmId { 0, 0, 0, 0 };

#if defined DEST_FS_USES_SPIFFS
	m_file = SPIFFS.open(filePath, "r");	
#elif defined DEST_FS_USES_LITTLEFS
	m_file = LittleFS.open(filePath, "r");
#endif

	if (!m_file) {
		Serial.printf("Could not open %s\n", filePath); 
		return VgmFormat::unknown;
	}	
	else {
		Serial.printf("Opened file %s\n", filePath);
		m_file.read((byte*)&vgmId, sizeof(vgmId));
		m_file.close();

		Serial.printf("First 4 Bytes: %X %X %X %X\n", vgmId.m0, vgmId.m1, vgmId.m2, vgmId.m3);
		if (vgmId.m0 == 0x1f && vgmId.m1 == 0x8b)
			return VgmFormat::compressed;

		else if (vgmId.m0 == 0x56 
			&& vgmId.m1 == 0x67 
			&& vgmId.m2 == 0x6D 
			&& vgmId.m3 == 0x20)	// "Vgm "
				return VgmFormat::uncompressed;
	}


	return VgmFormat::unknown;
}


byte
VgmReader::readByte()
{
	if (m_bufCursor == BUFSIZE)
	{
		if (!_readFromFile(m_fileCursor) != 0)
			Serial.println("VgmReader::readByte() error");
		m_bufCursor = 0;

		//_dbgPrintBuffer();
	}
	m_fileCursor++;	

	return m_buf[m_bufCursor++];
}

/*********************************************************
 * PRIVATE FUNCTIONS
 * *******************************************************/
void
VgmReader::_reset()
{
	m_gd3Offset = 0;
	m_dataOffset = 0;
	m_loopOffset = 0;
	m_dataLength = 0;
	m_bufCursor = BUFSIZE;
	m_fileCursor = 0;
}

void
VgmReader::_parseHeader()
{
	VgmHeader header;
	m_file.seek(0);
	m_file.read((byte*)&header, sizeof(VgmHeader));

	// Store absolute offset to GD3 data	
	m_gd3Offset = header.gd3Offset + GD3_OFFSET;
	Serial.printf("GD3 absolute offset: $%X\n", m_gd3Offset);

	// Read the relative offset to VGM data stream
	m_file.seek(VGM_DATA_OFFSET);
	uint32_t offset;
	m_file.read((byte*)&offset, sizeof(uint32_t));
	m_dataOffset = ( offset != 0 ? offset + VGM_DATA_OFFSET : 0x40 );
	Serial.printf("Song data absolute offset: $%X\n", m_dataOffset);

	// Store absolute loop offset
	m_loopOffset += (header.loopOffset != 0 ? 0x1C : 0);

}

size_t
VgmReader::_readFromFile(uint32_t offset)
{
	//Serial.printf("Reading from $%X\n", offset);
	m_file.seek(offset);
	size_t nbytes = m_file.readBytes((char*)m_buf, BUFSIZE);
	m_bufCursor = 0;

	return nbytes;
}

void
VgmReader::_dbgPrintBuffer()
{
	Serial.print("          ");
	for (int j = 0; j < 0x10; j++) 
	{
		Serial.printf("%02X  ", j);
	}	
	Serial.println();

	byte *cursor = m_buf;

	for (int row = 0; row < 16; row++)
	{
		Serial.printf("    %04X  ", row<<4);
		for (int col = 0; col < 16; col++)
			Serial.printf("%02X  ", *cursor++);
			
		Serial.println();
	}
}
