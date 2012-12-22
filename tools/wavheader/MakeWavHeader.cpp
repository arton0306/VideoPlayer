// after you get the wav header file, you can use "copy /b header + data my.wav" to get a wav file
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include "MakeWavHeader.hpp"

using namespace std;

typedef unsigned int uint;

static const char riffStr[] = "RIFF";
static const char waveStr[] = "WAVE";
static const char fmtStr[]  = "fmt ";
static const char dataStr[] = "data";

FILE * openFile(const char *fileName);
WavHeader fillInHeader(uint fileSize, uint sampleRate, uint bits, uint channels);
void writeHeader(FILE * fptr, WavHeader wavHeader);

int main(int argc, char const *argv[])
{
    FILE *inFile = fopen(argv[1], "rb");
    FILE *outFile = fopen(argv[2], "wb");

    fseek(inFile, 0L, SEEK_END);
    uint fileSize = ftell(inFile);
    fseek(inFile, 0L, SEEK_SET);
    cout << "file size: " << fileSize << endl;

    uint sampleRate, bits, channels;
    cout << "channel cout :";
    cin >> channels;
    cout << "bits per sample :";
    cin >> bits;
    cout << "sample rate :";
    cin >> sampleRate;

    WavHeader wavHeader = fillInHeader( fileSize, sampleRate, bits, channels );
    writeHeader( outFile, wavHeader );

    if (outFile) fclose(outFile);
    outFile = NULL;
    if (inFile) fclose(inFile);
    outFile = NULL;
    return 0;
}

FILE * openFile(const char *fileName)
{
    // Try to open the file for reading
    FILE * fptr = fopen(fileName, "rb");
    if (fptr == NULL)
    {
        // didn't succeed
        string msg = "Error : Unable to open file \"";
        msg += fileName;
        msg += "\" for reading.";
        cout << msg << endl;
    }
    return fptr;
}

WavHeader fillInHeader(uint fileSize, uint sampleRate, uint bits, uint channels)
{
    WavHeader header;
    memset( &header, 0, sizeof(header) );

    // fill in the 'riff' part..

    // copy string 'RIFF' to riff_char
    memcpy(&(header.riff.riff_char), riffStr, 4);
    // package_len unknown so far
    header.riff.package_len = fileSize + 36;
    // copy string 'WAVE' to wave
    memcpy(&(header.riff.wave), waveStr, 4);


    // fill in the 'format' part..

    // copy string 'fmt ' to fmt
    memcpy(&(header.format.fmt), fmtStr, 4);

    header.format.format_len = 0x10;
    header.format.fixed = 1;
    header.format.channel_number = (short)channels;
    header.format.sample_rate = (int)sampleRate;
    header.format.bits_per_sample = (short)bits;
    header.format.byte_per_sample = (short)(bits * channels / 8);
    header.format.byte_rate = header.format.byte_per_sample * (int)sampleRate;
    header.format.sample_rate = (int)sampleRate;

    // fill in the 'data' part..

    // copy string 'data' to data_field
    memcpy(&(header.data.data_field), dataStr, 4);
    // data_len unknown so far
    header.data.data_len = fileSize;

    return header;
}

void writeHeader(FILE * fptr, WavHeader wavHeader)
{
    int res;

    // write the supplemented header in the beginning of the file
    // fseek(fptr, 0, SEEK_SET);
    res = (int)fwrite(&wavHeader, sizeof(wavHeader), 1, fptr);
    if (res != 1)
    {
        cout << "Error while writing to a wav file." << endl;
        // throw runtime_error("Error while writing to a wav file.");
    }

    // jump back to the end of the file
    //fseek(fptr, 0, SEEK_END);
}

