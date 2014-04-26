#ifndef WAVEDECODER_H
#define WAVEDECODER_H

using namespace std;

class WaveDecoder
{
    public:
        WaveDecoder(string path);
        int audioFormat;
        int numChannels;
        int sampleRate;
        int byteRate;
        int blockAlign;
        int bitsPerSample;
        int size;
        char * bytes;
        char * read(int &length);
        
    private:
        const char * path;
        void decodeParams();
        int getShort(int startIndex);
        int getInt(int startIndex);
};

#endif // WAVEDECODER_H
