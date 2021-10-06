#include "VgmReader.h"

VgmReader::VgmReader() :
	m_fs_ptr {0},
	m_gd3Offset {0},
	m_dataOffset {0},
	m_loopOffset {0},
	m_dataLength {0}
{

}

VgmReader::~VgmReader()
{
}

void
VgmReader::begin(FileSystem* fs_ptr)
{
	m_fs_ptr = fs_ptr;
}

void
VgmReader::parseHeader()
{
	VgmHeader header;
	m_fs_ptr->seek(0);
	m_fs_ptr->read((byte*)&header, sizeof(VgmHeader));

	// Store absolute offset to GD3 data
	m_gd3Offset = header.gd3Offset + GD3_OFFSET;
	Serial.printf("\tGD3 absolute offset: $%X\n", m_gd3Offset);

	// Read the relative offset to VGM data stream
	m_fs_ptr->seek(VGM_DATA_OFFSET);
	uint32_t offset;
	m_fs_ptr->read((byte*)&offset, sizeof(uint32_t));
	m_dataOffset = ( offset != 0 ? offset + VGM_DATA_OFFSET : 0x40 );
	Serial.printf("\tSong data absolute offset: $%X\n", m_dataOffset);

	// Store absolute loop offset
	m_loopOffset += (header.loopOffset != 0 ? 0x1C : 0);

	m_dataLength = m_gd3Offset - m_dataOffset;
	//m_fileCursor = m_dataOffset;
	m_fs_ptr->setFileCursor(m_dataOffset);
}

VgmFormat
VgmReader::getFormat() const
{
	struct {
		byte m0;
		byte m1;
		byte m2;
		byte m3;
	} vgmId { 0, 0, 0, 0 };

	//FileSystem file = *m_fs_ptr;
	m_fs_ptr->seek(0);
	m_fs_ptr->read((byte*)&vgmId, sizeof(vgmId));

	Serial.printf("\nFirst 4 Bytes: %X %X %X %X\n", vgmId.m0, vgmId.m1, vgmId.m2, vgmId.m3);
	if (vgmId.m0 == 0x1f && vgmId.m1 == 0x8b)
		return VgmFormat::compressed;

	else if (vgmId.m0 == 0x56
		&& vgmId.m1 == 0x67
		&& vgmId.m2 == 0x6D
		&& vgmId.m3 == 0x20)	// "Vgm "
			return VgmFormat::uncompressed;

	return VgmFormat::unknown;
}

void
VgmReader::end()
{
	m_gd3Offset = 0;
	m_dataOffset = 0;
	m_loopOffset = 0;
	m_dataLength = 0;
}
