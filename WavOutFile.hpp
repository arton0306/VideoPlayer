#ifndef WAVOUTFILE_HPP
#define WAVOUTFILE_HPP

/// WAV audio file 'riff' section header
struct WavRiff
{
    char riff_char[4];
    int  package_len;
    char wave[4];
};

/// WAV audio file 'format' section header
struct WavFormat
{
    char  fmt[4];
    int   format_len;
    short fixed;
    short channel_number;
    int   sample_rate;
    int   byte_rate;
    short byte_per_sample;
    short bits_per_sample;
};

/// WAV audio file 'data' section header
struct WavData
{
    char  data_field[4];
    unsigned int  data_len;
};


/// WAV audio file header
struct WavHEader
{
    WavRiff   riff;
    WavFormat format;
    WavData   data;
};

class WavOutFile
{
public:
    WavOutFile();
};

#endif // WAVOUTFILE_HPP
