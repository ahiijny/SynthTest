#ifndef MIDIDECODER_H
#define MIDIDECODER_H
#include <vector>

using namespace std;

class MidiDecoder
{
    public:
        MidiDecoder(string path);
        int size;
        int formatType;
        int trackNum;
        vector<int> trackDataStartIndexes;
        vector<int> trackSizes;
        int pulsesPerQuarterNote;
        int framesPerSecond;
        int ticksPerFrame;
        int microsecondsPerTick;
        bool isTimeDivisionInPPQN;
        char * bytes;
        char * read(int &length);
        
    private:
        const char * path;
        void decodeParams();
        void decodeFormat();
        void decodeNumTracks();
        void decodeTimeDivision();
        void decodeChunks();
        void decodeTrackChunk(int &index);
        
        void decodeEvent(char statusByte, int &index);
        void decodeMIDIEvent(char statusByte, int &index);
        void decodeMetaEvent(int &index);
        void decodeSystemExclusiveEvent(char statusByte, int &index);
        
        int getShort(int startIndex);
        int getInt(int startIndex);
        int getInt(int startIndex, int byteNum);
        int getVarLen(int &index);
};

#endif // MIDIDECODER_H
