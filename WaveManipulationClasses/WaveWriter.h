#ifndef WAVEWRITER_H
#define WAVEWRITER_H
#include <vector>

/** https://ccrma.stanford.edu/courses/422/projects/WaveFormat/
 */
using namespace std;

class WaveWriter
{
    public:
        WaveWriter(string file_path, int sample_rate = 9600, int num_channels = 1, int bits_per_sample = 8);
        void append(char datum);
        void append(char * data, int length);
        int size();
        void swap(int start, char * newBytes, int length);
        bool write();
        
        std::vector<char> bytes;
        void finalizeChunkSizes();
        
        const char * path;

    private:
        int subchunk2Size;
        int sampleRate;
        short numChannels;
        short bitsPerSample;

        char * getBytes(short n);
        char * getBytes(int n);

        void chunkID();
		void chunkSize(bool init);
		void format();
		void subchunk1ID();
		void subchunk1Size();
		void audioformat();
		void numChannelsSet();
		void sampleRateSet();
		void byteRate();
		void blockAlign();
		void bitsPerSampleSet();
		void subchunk2ID();
		void subchunk2SizeSet(bool init);
};

#endif // WAVEWRITER_H
