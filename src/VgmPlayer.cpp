#include "VgmPlayer.h"
#include "config.h"
#include <ESP32-targz.h>

VgmPlayer::VgmPlayer(Sn76489* psgL, Sn76489* psgR, Ym2413* ym2413)
    : m_psgL { psgL }
    , m_psgR { psgR }
    , m_ym2413 { ym2413 }
{
}

VgmPlayer::~VgmPlayer()
{
}

void VgmPlayer::begin()
{
    // Initialize chips
    if (m_psgL) {
        m_psgL->begin();
        m_psgL->muteAll();
    }
    if (m_psgR) {
        m_psgR->begin();
        m_psgR->muteAll();
    }
    if (m_ym2413) {
        m_ym2413->begin();
        m_ym2413->muteAll();
    }

    dbgPrint();
    // TODO Check error code
    if (!tarGzFS.begin())
        Serial.println("tarGzFS init failed!");
    else {
        m_gzip = new GzUnpacker();
        m_gzip->haltOnError(true); // stop on fail (manual restart/reset required)
        m_gzip->setupFSCallbacks(targzTotalBytesFn, targzFreeBytesFn); // prevent the partition from exploding, recommended
        m_gzip->setGzProgressCallback(BaseUnpacker::defaultProgressCallback); // targzNullProgressCallback or defaultProgressCallback
        m_gzip->setLoggerCallback(BaseUnpacker::targzPrintLoggerCallback); // gz log verbosity
    }
    if (!m_vgmReader.begin()) {
        Serial.println("Error mounting file system!");
        return;
    }
}

void VgmPlayer::play(const char* filePath)
{
    if (m_vgmReader.open(filePath) == ERR_NOERROR) {
        if (m_vgmReader.isDir()) {
            playDir(filePath);
        } else {
            playCurrentFile();
        }
    } else {
        Serial.printf("Error VgmPlayer:51 - open %s\n", filePath);
    }
}

void VgmPlayer::playDir(const char* dirName)
{
    m_vgmReader.openDir(dirName);
    m_vgmReader.openNextFile();
    while (m_vgmReader.isValid()) {
        if (m_vgmReader.isDir()) {
            // FIXME: avoid infinite recursion
            play(m_vgmReader.getPath());
        } else {
            playCurrentFile();
        }
        m_vgmReader.openNextFile();
    }
}

void VgmPlayer::playCurrentFile()
{
    VgmFormat format = m_vgmReader.getFormat();
    uint8_t err = 0;
    char const* fileName = m_vgmReader.getPath();
    // boolean tmpCreated = false;

    Serial.printf("Found file %s\n", fileName);

    // Exit if file not recognized as VGM file
    if (format == VgmFormat::unknown) {
        Serial.printf("Error opening %s, file unknown", fileName);
        return;
    }
    // Uncompress the file to a temporary if it's a compressed VGM
    if (format == VgmFormat::compressed) {
        // Decompress the VGM file to a temporary file
        Serial.printf("Uncompressing %s", fileName);
        if (!m_gzip->gzExpander(tarGzFS, fileName, tarGzFS, "/tmp.vgm")) {
            Serial.printf("operation failed with return code #%d", m_gzip->tarGzGetError());
        }
        // close original file and open tmp file
        m_vgmReader.close();
        err = m_vgmReader.open("/tmp.vgm");
        // tmpCreated = true;
    }
    // Parse the uncompressed file
    m_vgmReader.parseHeader();

    if (err != 0) {
        Serial.printf("Couldn't open file %s\n", fileName);
    } else {
        Serial.printf("Playing %s\n", fileName);
        parseCommands();

        m_vgmReader.close();
        /*if (tmpCreated) {
            Serial.println("Deleting /tmp.vgm");
            m_vgmReader.delFile("/tmp.vgm");
            // FIXME: guru mediation
        }*/
    }
}

////////////////////////////////////////////////////////////////////
// PRIVATE FUNCTIONS
///////////////////////////////////////////////////////////////////
uint8_t VgmPlayer::readByte()
{
    return m_vgmReader.readByte();
}

void VgmPlayer::parseCommands()
{
    bool endOfSoundData = false;

    while (!endOfSoundData) {
        uint8_t cmd = readByte();
        // Serial.printf("[%02X] ", cmd);

        switch (cmd) {
            uint8_t dd;
            uint8_t aa;

        // 0x4F dd (TODO - NOT IMPLEMENTED)
        // Game Gear PSG stereo, write dd to port 0x06
        case 0x4f:
            readByte();
            break;
        // 0x50 dd
        // SN76489 Write dd value
        case 0x50:
            dd = readByte();
            m_psgL->writeData(dd);
            // dbgPrint(cmd, dd);
            break;

        // 0x51 aa dd: YM2413, write value dd to register aa
        case 0x51:
            aa = readByte();
            dd = readByte();
            // dbgPrint(cmd, aa, dd);
            m_ym2413->writeData(aa, dd);
            break;

        // 0x61 nn nn
        // Wait n samples, n can range from 0 to 65535 (approx 1.49 seconds).
        // Longer pauses than this are represented by multiple wait commands.
        case 0x61: {
            uint8_t lo = readByte();
            uint8_t hi = readByte();
            uint32_t nsamples = (uint32_t)word(hi, lo);
            uint32_t wait = WAIT1SAMPLE * nsamples;
            // uint32_t wait = (uint32_t)((1000000.0 / 44100.0) * nsamples);
            delayMicroseconds(wait);
            // dbgPrint(cmd, wait);
            break;
        }

        // wait 735 samples (60th of a second), a shortcut for 0x61 0xdf 0x02
        case 0x62:
            delayMicroseconds(WAIT60TH);
            // dbgPrint(cmd);
            break;

        // wait 882 samples (50th of a second), a shortcut for 0x61 0x72 0x03
        case 0x63:
            delayMicroseconds(WAIT50TH);
            // dbgPrint(cmd);
            break;

        // 0x7n
        // wait n+1 samples, n can range from 0 to 15.
        case 0x70:
            delayMicroseconds(WAIT1SAMPLE); /*dbgPrint(cmd, WAIT1SAMPLE);*/
            break;
        case 0x71:
            delayMicroseconds(WAIT2SAMPLE); /*dbgPrint(cmd, WAIT2SAMPLE);*/
            break;
        case 0x72:
            delayMicroseconds(WAIT3SAMPLE); /*dbgPrint(cmd, WAIT3SAMPLE);*/
            break;
        case 0x73:
            delayMicroseconds(WAIT4SAMPLE); /*dbgPrint(cmd, WAIT4SAMPLE);*/
            break;
        case 0x74:
            delayMicroseconds(WAIT5SAMPLE); /*dbgPrint(cmd, WAIT5SAMPLE);*/
            break;
        case 0x75:
            delayMicroseconds(WAIT6SAMPLE); /*dbgPrint(cmd, WAIT6SAMPLE);*/
            break;
        case 0x76:
            delayMicroseconds(WAIT7SAMPLE); /*dbgPrint(cmd, WAIT7SAMPLE);*/
            break;
        case 0x77:
            delayMicroseconds(WAIT8SAMPLE); /*dbgPrint(cmd, WAIT8SAMPLE);*/
            break;
        case 0x78:
            delayMicroseconds(WAIT9SAMPLE); /*dbgPrint(cmd, WAIT9SAMPLE);*/
            break;
        case 0x79:
            delayMicroseconds(WAIT10SAMPLE); /*dbgPrint(cmd, WAIT10SAMPLE);*/
            break;
        case 0x7A:
            delayMicroseconds(WAIT11SAMPLE); /*dbgPrint(cmd, WAIT11SAMPLE);*/
            break;
        case 0x7B:
            delayMicroseconds(WAIT12SAMPLE); /*dbgPrint(cmd, WAIT12SAMPLE);*/
            break;
        case 0x7C:
            delayMicroseconds(WAIT13SAMPLE); /*dbgPrint(cmd, WAIT13SAMPLE);*/
            break;
        case 0x7D:
            delayMicroseconds(WAIT14SAMPLE); /*dbgPrint(cmd, WAIT14SAMPLE);*/
            break;
        case 0x7E:
            delayMicroseconds(WAIT15SAMPLE); /*dbgPrint(cmd, WAIT15SAMPLE);*/
            break;
        case 0x7F:
            delayMicroseconds(WAIT16SAMPLE); /*dbgPrint(cmd, WAIT16SAMPLE);*/
            break;

        // 0x66 End of sound data
        case 0x66:
            endOfSoundData = true;

        default:
            dbgPrint(cmd);
            endOfSoundData = true;
            break;
        }
    }

    m_psgL->muteAll();
}

void VgmPlayer::dbgPrint() const
{
    if (m_psgL)
        m_psgL->dbgPrint();
    if (m_psgR)
        m_psgR->dbgPrint();
    if (m_ym2413)
        m_ym2413->dbgPrint();
}

void VgmPlayer::dbgPrint(uint8_t cmd) const
{
    switch (cmd) {
    case 0x62:
        Serial.println("WAIT60TH");
        break;
    case 0x66:
        Serial.println("--END--");
        break;
    default:
        Serial.printf("UNKNOWN %02X\n", cmd);
        break;
    }
}

void VgmPlayer::dbgPrint(uint8_t cmd, uint8_t value) const
{
    switch (cmd) {
    case 0x50:
        Serial.printf("Sn76489 << (%02X)\n", value);
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
        Serial.printf("WAIT %d microseconds\n", value);
        break;
    default:
        Serial.printf("UNKNOWN %02X\n", cmd);
        break;
    }
}

void VgmPlayer::dbgPrint(uint8_t cmd, uint32_t value) const
{
    switch (cmd) {
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
        Serial.printf("WAIT %d microseconds\n", value);
        break;
    default:
        Serial.printf("UNKNOWN %02X\n", cmd);
        break;
    }
}

void VgmPlayer::dbgPrint(uint8_t cmd, uint8_t aa, uint8_t dd) const
{
    switch (cmd) {
    case 0x51:
        Serial.printf("ym2413 << (%02X, %02X)\n", aa, dd);
        break;
    default:
        Serial.printf("UNKNOWN %02X\n", cmd);
        break;
    }
}