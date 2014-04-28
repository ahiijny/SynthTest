#ifndef MIDIDECODER_H
#define MIDIDECODER_H

using namespace std;

class MidiDecoder
{
    public:
        MidiDecoder(string path);
        int size;
        int formatType;
        int trackNum;
        int pulsesPerQuarterNote;
        int framesPerSecond;
        int ticksPerFrame;
        bool isTimeDivisionSMPTE;
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
        int getVarLen(int &index);
};

#endif // MIDIDECODER_H
