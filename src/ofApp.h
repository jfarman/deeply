#pragma once

#include "ofMain.h"
#include "ofxStk.h"
#include "ofxGui.h"
#include "ofxVboParticles.h"
#include "Granulate.h"
#include "Goom.h"

#include <sstream>
#include <algorithm>
using namespace std;

#define expand 7.0
#define GROWTH_CONST 70.0f
#define NUM_COMPONENTS 6
#define NUM_VOICES 4
#define NUM_CHORDS 4

#define RGB 3
#define R_INDEX 0
#define G_INDEX 1
#define B_INDEX 2
#define NUM_COLORS 5

// Constants that define the phases
// of the breathing cycle (ints)
#define CYCLE_TOTAL 12
#define CYCLE_INHALE_PHASE 4
#define CYCLE_HOLD_BREATH 6
#define CYCLE_EXHALE_PHASE 10

// Constants that define the phases
// of the breathing cycle (floats)
#define CYCLE_TOTAL_F 12.0f
#define CYCLE_INHALE_PHASE_F 4.0f
#define CYCLE_HOLD_BREATH_F 6.0f
#define CYCLE_EXHALE_PHASE_F 10.0f

struct MusicalNote {
    stk::StkFloat noteNumber;
    long voiceTag;
};


class ofApp : public ofBaseApp{
public:
    void setup();
    void update();
    void draw();
    
    void keyPressed(int key);
    void keyReleased(int key);
    void mouseMoved(int x, int y);
    void mouseDragged(int x, int y, int button);
    void mousePressed(int x, int y, int button);
    void mouseReleased(int x, int y, int button);
    void windowResized(int w, int h);
    void dragEvent(ofDragInfo dragInfo);
    void gotMessage(ofMessage msg);
    
    ofEasyCam camera;
    
    ofIcoSpherePrimitive centerSphere;
    ofIcoSpherePrimitive icospheres[NUM_COMPONENTS];
    
    int colors[NUM_COLORS][RGB];
    std::vector<int> colorOrder;
    
    void adjustNotes(int i);
    void initColors();
    
    float time;             // keeps track of the elapsed time
    float timer;            // timer that starts after the into
    
    int currentStep;        // current step of the breathing cycle
    int numSpheres;         // number of spheres to be rendered
    
    float width;            // keeps track of the largest sphere's width
    float defaultWidth;     // the default width for the spheres
    
    void audioOut(float *output,int bufferSize,int nChannels);
    
    stk::Voicer *voicer;
    stk::Granulate granulater;
    
    MusicalNote c,d,e,f,g,a,b,c2,d2,e2;
    MusicalNote x1,x2,x3,x4;
    
    void initChords(int offset, bool update);
    int newRandom(int n, int length);
    
    MusicalNote base;
    bool baseOn;
    
    MusicalNote chords[4][4];
    MusicalNote newChords[4][4];
   
    bool notesOn[4];
    bool isIntro;
    bool scaleChanged;
    
    stk::FreeVerb reverb;
    stk::OnePole filter;
    
    void reverbParametersChanged(float &value);
    void filterCutoffChanged(float &value);
    
    float reverbValue;
    float mod_f;
    int mod_i;
    int currScale;
    int currCycle;
    int currChord;
    
    ofTrueTypeFont header;
    ofTrueTypeFont headerLG;
    ofTrueTypeFont instructions;
    
    ofxVboParticles *vboParticles;
    ofVec3f particlePosition;
};

