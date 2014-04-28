#include <iostream>
#include <fstream>
#include <conio.h>
#include "MidiManipulationClasses/MidiWriter.h"

using namespace std;

/** From: http://josh.agarrado.net/music/anime/get/3595/sketchbook-kaze-sagashi.pdf
 */
int main()
{
    MidiWriter mw("Audio/Tesssssst.mid");
    mw.updateFormatType(1);
    mw.updateTrackNum(3);
    mw.updateTimeDivision(120);
    mw.selectInstrument(0, 0, 0);
    mw.setMicrosecondsPerBeat(300000);

    // Track 1
    mw.noteOn(0, 0, 48, 64);
    mw.noteOn(100, 0, 48, 0);
    mw.noteOn(20, 0, 55, 64);
    mw.noteOn(40, 0, 55, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    mw.noteOn(20, 0, 64, 64);
    mw.noteOn(40, 0, 64, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    
    mw.noteOn(20, 0, 50, 64);
    mw.noteOn(100, 0, 50, 0);
    mw.noteOn(20, 0, 57, 64);
    mw.noteOn(40, 0, 57, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    mw.noteOn(20, 0, 65, 64);
    mw.noteOn(40, 0, 65, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    
    mw.noteOn(20, 0, 52, 64);
    mw.noteOn(100, 0, 52, 0);
    mw.noteOn(20, 0, 55, 64);
    mw.noteOn(40, 0, 55, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    mw.noteOn(20, 0, 67, 64);
    mw.noteOn(40, 0, 67, 0);
    mw.noteOn(20, 0, 64, 64);
    mw.noteOn(100, 0, 64, 0);
    
    mw.noteOn(20, 0, 67, 64);
    mw.noteOn(40, 0, 67, 0);
    mw.noteOn(20, 0, 64, 64);
    mw.noteOn(100, 0, 64, 0);
    mw.noteOn(20, 0, 65, 64);
    mw.noteOn(100, 0, 65, 0);
    mw.noteOn(20, 0, 55, 64);
    mw.noteOn(40, 0, 55, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    
    mw.noteOn(20, 0, 67, 64);
    mw.noteOn(40, 0, 67, 0);
    mw.noteOn(20, 0, 64, 64);
    mw.noteOn(100, 0, 64, 0);
    mw.noteOn(20, 0, 65, 64);
    mw.noteOn(100, 0, 65, 0);
    mw.noteOn(20, 0, 55, 64);
    mw.noteOn(40, 0, 55, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    
    mw.noteOn(20, 0, 64, 64);
    mw.noteOn(40, 0, 64, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    mw.noteOn(20, 0, 65, 64);
    mw.noteOn(100, 0, 65, 0);
    mw.noteOn(20, 0, 55, 64);
    mw.noteOn(40, 0, 55, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    
    mw.noteOn(20, 0, 65, 64);
    mw.noteOn(40, 0, 65, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    mw.noteOn(20, 0, 64, 64);
    mw.noteOn(100, 0, 64, 0);
    mw.noteOn(20, 0, 55, 64);
    mw.noteOn(40, 0, 55, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    
    mw.noteOn(20, 0, 67, 64);
    mw.noteOn(40, 0, 67, 0);
    mw.noteOn(20, 0, 64, 64);
    mw.noteOn(100, 0, 64, 0);
    mw.noteOn(20, 0, 62, 64);
    mw.noteOn(100, 0, 62, 0);
    mw.noteOn(20, 0, 55, 64);
    mw.noteOn(40, 0, 55, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    
    mw.noteOn(20, 0, 65, 64);
    mw.noteOn(40, 0, 65, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    mw.noteOn(20, 0, 64, 64);
    mw.noteOn(100, 0, 64, 0);
    mw.noteOn(20, 0, 55, 64);
    mw.noteOn(100, 0, 55, 0);
    mw.noteOn(20, 0, 60, 64);
    mw.noteOn(100, 0, 60, 0);
    
    mw.noteOn(20, 0, 64, 64);
    mw.noteOn(150, 0, 64, 0);
    mw.noteOn(20, 0, 100, 0);
    mw.noteOn(240, 0, 100, 0);
    mw.endTrack();
    
    // Track 2
    
    mw.newTrack();
    mw.noteOn(0, 0, 64, 64);
    mw.noteOn(120, 0, 64, 0);
    mw.noteOn(360, 0, 65, 64);
    mw.noteOn(120, 0, 65, 0);
    mw.noteOn(360, 0, 64, 64);
    mw.noteOn(120, 0, 64, 0);
    mw.noteOn(540, 0, 50, 64);
    mw.noteOn(120, 0, 50, 0);
    mw.noteOn(360, 0, 52, 64);

    mw.noteOn(120, 0, 52, 0);
    mw.noteOn(360, 0, 50, 64);
    mw.noteOn(120, 0, 50, 0);
    mw.noteOn(360, 0, 52, 64);
    mw.noteOn(120, 0, 52, 0);
    mw.noteOn(360, 0, 50, 64);
    mw.noteOn(120, 0, 50, 0);
    mw.noteOn(360, 0, 48, 64);
    mw.noteOn(120, 0, 48, 0);
    mw.endTrack();
    
    // Track 3
    
    mw.newTrack();
    mw.noteOn(840, 0, 60, 64);
    mw.noteOn(60, 0, 60, 0);
    mw.noteOn(660, 0, 60, 64);
    mw.noteOn(120, 0, 60, 0);
    mw.noteOn(120, 0, 67, 64);
    mw.noteOn(60, 0, 67, 0);
    mw.noteOn(180, 0, 72, 64);
    mw.noteOn(120, 0, 72, 0);
    mw.noteOn(2100, 0, 64, 64);
    mw.noteOn(120, 0, 64, 0);
    mw.noteOn(0, 0, 62, 64);
    mw.noteOn(180, 0, 62, 0);
    mw.endTrack();
    
    mw.write();
    system("PAUSE");
    return 0;
}
