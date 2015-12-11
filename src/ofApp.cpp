#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){
    
    stk::Stk::setRawwavePath(ofToDataPath("rawwaves",true));
    voicer = new stk::Voicer();
    
    instructions.loadFont("RobotoTTF/Roboto-Light.ttf", 20);
    header.loadFont("amatic/Amatic-Bold.ttf", 40);
    headerLG.loadFont("amatic/Amatic-Bold.ttf", 70);
    
    vboParticles = new ofxVboParticles(14000, 1000);
    vboParticles->friction = 0.0001; // set friction (0.0 - 1.0);
    particlePosition = ofVec3f(0, 0, 0);
    
    camera.setFov(80);
    camera.setDistance(600);
    
    for (int i = 0; i < 100 ; i++) {
        stk::Goom *moog = new stk::Goom();
        voicer->addInstrument(moog);
    }
    
    currentStep = -1;
    initChords(0, false);
    currChord = 0;
    currCycle = 0;
    
    for (int i = 0; i < NUM_VOICES; i++) notesOn[i] = false;
    
    ofSoundStreamSetup(2, 0);
    ofSetVerticalSync(true);
    initColors();
    
    width = 50;
    defaultWidth = width;
    numSpheres = 0;
    
    centerSphere.setRadius(50);
    centerSphere.setResolution(0);
    
    for (int i = 0; i < NUM_COMPONENTS ; i++) {
        icospheres[i].setRadius(width);
    }
    
    reverb.setDamping(.6);

    isIntro = true;
    baseOn = false;
    
    time = ofGetElapsedTimef();
}

//--------------------------------------------------------------
void ofApp::update(){
    time += ofGetElapsedTimef() - time;

    int totalCycles = (int)(time-timer) / CYCLE_TOTAL;
    if (currCycle != totalCycles) {
        random_shuffle(colorOrder.begin(), colorOrder.end());
        currCycle = totalCycles;
    }
    currChord = totalCycles % NUM_CHORDS;
    
    particlePosition.z = sin(ofGetElapsedTimef()) * 400;
    particlePosition.x  = cos(ofGetElapsedTimef()) * 400;
    particlePosition.y  = sin(ofGetElapsedTimef() / 2.0) * 100;
    
    for (int i = 0; i < 40; i++) {
        ofVec3f position = particlePosition;
        ofVec3f velocity = ofVec3f(ofRandom(-1, 1), ofRandom(-1, 1), ofRandom(-1, 1));
        ofColor color;
        color.set(253,255,223,100);
        vboParticles->addParticle(position, velocity, color);
    }
    vboParticles->update();
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    float mod_f =(fmod(time-timer,CYCLE_TOTAL_F));
    int mod_i = (int)mod_f;
    int sphereIndex = numSpheres - 1;
    
    bool inhalePhase = mod_f < CYCLE_INHALE_PHASE_F;
    bool holdPhase = (mod_f >= CYCLE_INHALE_PHASE_F) && (mod_f < CYCLE_HOLD_BREATH_F);
    bool exhalePhase = (mod_f >= CYCLE_HOLD_BREATH_F) && (mod_f < CYCLE_EXHALE_PHASE_F);
    bool restPhase = mod_f >= CYCLE_EXHALE_PHASE_F;
    
    ofColor darkGray = ofColor::fromHex(0x1c2f2f);
    ofBackgroundGradient(darkGray * .9, darkGray * .2);
    
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    glDisable( GL_DEPTH_TEST );
    ofEnableAlphaBlending();
    ofNoFill();
    
    camera.begin();

    float spinX = sin(ofGetElapsedTimef()*.35f);
    float spinY = cos(ofGetElapsedTimef()*.075f);
    
    centerSphere.rotate(spinX, 1.0, 0.0, 0.0);
    centerSphere.rotate(spinY, 0, 1.0, 0.0);
    
    ofPushStyle();
        ofSetColor(253,255,223,180);
        ofFill();
        centerSphere.draw();
    ofPopStyle();
    ofPushStyle();
        ofSetColor(253,255,223,250);
        ofNoFill();
        centerSphere.setScale(1.01f);
        centerSphere.drawWireframe();
        centerSphere.setScale(1.f);
    ofPopStyle();
    
    float xOffset = (20.0f*(sin(mod_f)));
    float yOffset = (20.0f*(cos(mod_f)));
    
    string strA = "welcome to";
    float aX = (0-(header.stringWidth(strA)*0.5f));
    string strB = "DEEPLY: A MEDITATIVE BREATHING EXPERIENCE";
    float bX = (0-(headerLG.stringWidth(strB)*0.5f));
    string strC = "find somewhere comfortable to sit...";
    float cX = (0-(header.stringWidth(strC)*0.5f));
    string strD = "and click anywhere to begin...";
    float dX = (0-(header.stringWidth(strD)*0.5f));
    
    if (isIntro) {
        ofSetColor(243,245,195, 250);
        header.drawString(strA,aX + xOffset + 15, 300);
        ofSetColor(147,237,212, 250);
        headerLG.drawString(strB,bX + xOffset + 20, 200);
        ofSetColor(243,245,195, 250);
        header.drawString(strC,cX + xOffset + 20, 140);
        header.drawString(strD,dX + xOffset + 25, 100);
        if (!baseOn) {
            base.voiceTag = voicer->noteOn(base.noteNumber,50);
            baseOn = true;
        }
        camera.end();
        return;
    }
    
    if ((mod_i != currentStep)) {
        adjustNotes(mod_i);
        currentStep = (currentStep + 1) % CYCLE_TOTAL;
    }
    
    if (inhalePhase){
        filter.setPole(.95);
        width = defaultWidth + (GROWTH_CONST*mod_f);
        icospheres[mod_i].setRadius(width);
        numSpheres = mod_i + 1;
    } else if (holdPhase) {
        float poleValue = (0.02f*(sin(11.0f * mod_f)))+.95;
        filter.setPole(poleValue);
    } else if (exhalePhase) {
        filter.setPole(.95);
        icospheres[sphereIndex].setRadius(width);
        width = defaultWidth + (GROWTH_CONST*(CYCLE_EXHALE_PHASE-mod_f));
        numSpheres = CYCLE_EXHALE_PHASE-(mod_i);
    } else if (restPhase) {
        numSpheres = 0;
    }
    
    float alpha;
    
    for (int i = 0; i < numSpheres; i++) {
        float spinX = sin(ofGetElapsedTimef()*(.35f*2));
        float spinY = cos(ofGetElapsedTimef()*(.075f*2));
        
        icospheres[i].rotate(spinX, 1.0, 0.0, 0.0);
        icospheres[i].rotate(spinY, 0, 1.0, 0.0);
        icospheres[i].setResolution(i);
        
        if (inhalePhase && i == mod_i){
            alpha = (mod_f - (float)mod_i) * 100.0f;
        } else if (exhalePhase && i == sphereIndex) {
            alpha = (100.0f-((mod_f - (float)mod_i)) * 100.0f);
        } else {
            alpha = 100;
        }
        
        int colorIndex = colorOrder[i];
        int rValue = colors[colorIndex][R_INDEX];
        int gValue = colors[colorIndex][G_INDEX];
        int bValue = colors[colorIndex][B_INDEX];
       
        ofPushStyle();
            ofFill();
            ofSetColor(rValue,gValue,bValue,alpha);
            icospheres[i].draw();
        ofPopStyle();
        ofPushStyle();
            ofNoFill();
            ofSetColor(rValue,gValue,bValue,alpha);
            icospheres[i].setScale(1.01f);
            icospheres[i].drawWireframe();
            icospheres[i].setScale(1.f);
        ofPopStyle();
    }
    
    ofDisableAlphaBlending();
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    if (!isIntro) {
        ofRotate(ofGetElapsedTimef() * 10, 0, 1, 1);
        vboParticles->draw();
    }
    camera.end();
}

void ofApp::initColors(){
    int defaultColors[NUM_COLORS][RGB] = {{60,186,200},
                           {147,237,212},{243,245,196},
                           {249,203,143},{241,145,129}};
    
    for (int i = 0; i < NUM_COLORS; i++) {
        for (int j = 0; j < RGB ; j++) {
            colors[i][j] = defaultColors[i][j];
        }
    }
    // this vector keeps track of the color order
    // and is randomized at each new cycle
    for (int i=1; i<5; ++i) colorOrder.push_back(i);
    random_shuffle(colorOrder.begin(), colorOrder.end());
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if (key == 'f') {
        ofToggleFullscreen();
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y){}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    if (isIntro) {
        isIntro = false;
        timer = time;
    }
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){}

void ofApp::initChords(int offset, bool update){
    
    int defaultChords[4][4] =  {{60, 67, 72, 74}, {55, 62, 65, 72},
                                {57, 67, 72, 74}, {62, 65, 72, 76}};
    
    base.noteNumber = (defaultChords[0][0]-24) + offset;
    
    for (int i = 0; i < NUM_CHORDS ; i++) {
        for (int j = 0; j < NUM_VOICES ; j++) {
            chords[i][j].noteNumber = defaultChords[i][j] + offset;
        }
    }
}

int ofApp::newRandom(int n, int length) {
    int arr[length-1];
    int index = 0;
    for (int i = 0; i < length; i++) {
        if (i != n) {
            arr[index] = i;
            index++;
        }
    }
    int randomIndex = rand() % (length-1);
    return arr[randomIndex];
}

//--------------------------------------------------------------
void ofApp::adjustNotes(int index){
    
    int defaultChords[4][4] =  {{60, 67, 72, 74}, {55, 62, 65, 72},
                                {57, 67, 72, 74}, {62, 65, 72, 76}};
    
    int steps[CYCLE_TOTAL] = {1, 2, 3, 4, 4, 4, 4, 3, 2, 1, 1, 1};
    int numActiveVoices = steps[index];
    
    if (index < CYCLE_INHALE_PHASE) {
        for (int i = 0; i < numActiveVoices; i++) {
            if (!notesOn[i]) {
                chords[currChord][i].voiceTag =
                voicer->noteOn(chords[currChord][i].noteNumber,50);
                notesOn[i] = true;
            }
        }
    }
    else if ((index > CYCLE_HOLD_BREATH) && (index <= CYCLE_EXHALE_PHASE)) {
        int i = CYCLE_EXHALE_PHASE - index;
        if (notesOn[i]) {
            voicer->noteOff(chords[currChord][i].voiceTag, 50);
            notesOn[i] = false;
        }
    }
}

void ofApp::audioOut(float *output, int bufferSize, int nChannels){
    stk::StkFrames frames(bufferSize,2);
    voicer->tick(frames);
    
    stk::StkFrames leftChannel(bufferSize,1);
    frames.getChannel(0, leftChannel, 0);
    
    filter.tick(leftChannel);
    
    stk::StkFrames reverbOut(bufferSize,2);
    reverb.tick(leftChannel,reverbOut,0,0);
    reverbOut.getChannel(0, leftChannel, 0);
    
    for (int i = 0; i < bufferSize ; i++) {
        output[2*i] = leftChannel(i,0);
        output[2*i+1] = leftChannel(i,0);
    }
}

void ofApp::reverbParametersChanged(float &value){
    reverb.setDamping(value);
}

void ofApp::filterCutoffChanged(float &value){}
