#include "FileSystem.h"

FileSystem::FileSystem() :
	m_file{0},
	m_dir {0},
	m_bufCursor {BUFSIZE},
	m_fileCursor {0}
{
	m_buf_ptr = (byte*)malloc(BUFSIZE);
	if (!m_buf_ptr)
	{
		for(;;);
	}
}

FileSystem::~FileSystem()
{
	free(m_buf_ptr);
}

bool
FileSystem::begin()
{
	return destFS.begin();
}

FileError
FileSystem::open(char const *filePath)
{
	FileError error_code = FileError::kNoError;

	m_file = destFS.open(filePath, "r");

	if (!m_file)
	{
		error_code = FileError::kOpenError;
	}

	return error_code;
}

bool
FileSystem::isDir(char const *pathName)
{
	bool is_directory = false;

	m_dir = destFS.open(pathName, "r");
	is_directory = m_dir.isDirectory();
	if (!is_directory) m_dir.close();

	return is_directory;
}


bool
FileSystem::openNextFile()
{
	// TODO: Use Dir object where available (ESP8266?)
	m_file = m_dir.openNextFile();

	return m_file != 0;
}


char const*
FileSystem::getFileName() const
{
	return m_file.name();
}

void
FileSystem::close()
{
	m_file.close();
	m_bufCursor = BUFSIZE;
	m_fileCursor = 0;
}

bool
FileSystem::delFile(char const *fileName)
{
	return destFS.remove(fileName);
}

bool
FileSystem::seek(uint32_t pos)
{
    return m_file.seek(pos);
}

size_t
FileSystem::read(uint8_t* buf, size_t size)
{
    return m_file.read(buf, size);
}

byte
FileSystem::readByte()
{
	if (m_bufCursor == BUFSIZE)
	{
		if (!readFromFile(m_fileCursor) != 0)
			Serial.println("VgmReader::readByte() error");
		m_bufCursor = 0;

		//_dbgPrintBuffer();
	}
	m_fileCursor++;

	return m_buf_ptr[m_bufCursor++];
}

void
FileSystem::setFileCursor(uint32_t pos)
{
	m_fileCursor = pos;
}

/*********************************************************
 * PRIVATE FUNCTIONS
 * *******************************************************/
size_t
FileSystem::readFromFile(uint32_t offset)
{
	//Serial.printf("Reading from $%X\n", offset);
	m_file.seek(offset);
	size_t nbytes = m_file.readBytes((char*)m_buf_ptr, BUFSIZE);
	m_bufCursor = 0;

	return nbytes;
}

void
FileSystem::dbgPrintBuffer()
{
	Serial.print("          ");
	for (int j = 0; j < 0x10; j++)
	{
		Serial.printf("%02X  ", j);
	}
	Serial.println();

	byte *cursor = m_buf_ptr;

	for (int row = 0; row < 16; row++)
	{
		Serial.printf("    %04X  ", row<<4);
		for (int col = 0; col < 16; col++)
			Serial.printf("%02X  ", *cursor++);

		Serial.println();
	}
}
