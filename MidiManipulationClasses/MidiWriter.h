#ifndef MIDIWRITER_H
#define MIDIWRITER_H
#include <vector>

using namespace std;

class MidiWriter
{
    public:
        MidiWriter(string path);
        vector<char> bytes;
        void append(char datum);
        void append(char * data, int length);
        int size();
        void swap(int start, char * newBytes, int length);
        bool write();
        
        char * getBytes(short n);
        char * getBytes(int n);
        char * getBytes(int n, int byteNum);
        char * getVarLenBytes(int n, int &length);

        void updateFormatType(short format);
        void updateTrackNum(short num);
        void updateTimeDivision(short ppqn);
        void updateTimeDivision(char fps, char ticks_per_frame);
        
        void newTrack();
        void endTrack();
        void appendEvent(char * bytes, int length);
        void appendEvent(int deltaTime, char * bytes, int length);
        void appendEvent(int deltaTime, char statusByte, char * bytes, int length);
        
        void appendMidiEvent(int deltaTime, char command, int channel, int var1, int var2);
        void appendMidiEvent(int deltaTime, char command, int channel, int var1);
        void noteOn(int deltaTime, int channel, int note, int velocity);
        void noteOff(int deltaTime, int channel, int note, int velocity);
        void aftertouch(int deltaTime, int channel, int note, int pressure);
        void selectInstrument(int deltaTime, int channel, int instrument);
        
        void appendMetaEvent(char typeByte, int dataLength, char * data);
        void setMicrosecondsPerBeat(int microSecondsPerQuarterNote);
        void appendSequenceNumber(short num);
        void appendTextEvent(int textLength, char * text);
        void appendCopyrightNotice(int textLength, char * text);
        void appendTrackName(int textLength, char * text);
        void appendInstrumentName(int textLength, char * text);
        void appendLyrics(int textLength, char * text);
        void appendMarker(int textLength, char * text);
        void appendCuePoint(int textLength, char * text);
        void appendMidiChannelPrefix(int channel);

    private:
        const char * path;
        short formatType;
        short trackNum;
        short pulsesPerQuarterNote;
        bool isTimeDivisionInPPQN;
        char framesPerSecond;
        char ticksPerFrame;
        char activeEventStatus;
        
        int trackStartIndex;
        
        void setMidiHeader();
        char * getTimeDivision(short ppqn);
        char * getTimeDivision(char fps, char ticksPerFrame);
        
        void updateTrackChunkSize();
};

#endif // MIDIWRITER_H
