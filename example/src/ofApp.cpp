#include "ofMain.h"
#include "ofxEquiMap.h"

class ofApp : public ofBaseApp, public ofxEquiMap::Scene{
    ofxEquiMap::Renderer em;
    ofxEquiMap::CustomFboRenderer em2;

    ofVboMesh m;
    ofEasyCam mEasyCam;

public:
    void setup()
    {
        ofSetVerticalSync(true);
        ofSetFrameRate(60);

        em.setup(1024, this);
        em2.setup(1024, this, GL_RGB, 4);

        ofSpherePrimitive p(1000, 24);
        m = p.getMesh();
    }

    void drawEquiScene()
    {
        ofPushStyle();
        ofSetLineWidth(3);
        ofSetColor(192);
        m.drawWireframe();
        ofPopStyle();

        // draw moving yellow ball
        float t = ofGetElapsedTimef();
        float factor = 0.5f;
        float radius = 10.0f;

        ofVec3f pos =
          ofVec3f(
            std::sin(t * factor),
            std::cos(t * factor),
            std::sin((t+1.5f) * factor)
          ) * radius;

        ofSetColor(255, 255, 0);
        ofFill();
        ofDrawSphere(pos, 1.0f);
    }

    void update() {
        em.render();
        em2.render();
    }

    void draw()
    {
        ofClear(0);
        if (ofGetKeyPressed(' ')) {
            em.draw(0, 0, ofGetWidth(), ofGetHeight());
            ofDrawBitmapStringHighlight("Renderer", 10, 40);
        } else if (ofGetKeyPressed('c')) {
            auto cubemap = em.getCubeMap();
            mEasyCam.begin();
            cubemap.bind();
            cubemap.drawSkybox( ofVec3f(0), 800 );
            cubemap.unbind();
            mEasyCam.end();

            ofDrawBitmapStringHighlight("CubeMap Skybox", 10, 40);
        } else if (ofGetKeyPressed('C')) {
            auto cubemap = em2.getCubeMap();
            mEasyCam.begin();
            cubemap.bind();
            cubemap.drawSkybox( ofVec3f(0), 800 );
            cubemap.unbind();
            mEasyCam.end();

            ofDrawBitmapStringHighlight("Custom CubeMap Skybox", 10, 40);
        } else if (ofGetKeyPressed('f')) {
            auto cubemap = em.getCubeMap();
            cubemap.getFbo().draw(0,0);
            ofDrawBitmapStringHighlight("CubeMap fbo", 10, 40);
        } else if (ofGetKeyPressed('F')) {
            auto cubemap = em2.getCubeMap();
            cubemap.getFbo().draw(0,0);
            ofDrawBitmapStringHighlight("CubeMap fbo (custom)", 10, 40);
        } else {
            em2.draw(0, 0, ofGetWidth(), ofGetHeight());
            ofDrawBitmapStringHighlight("CustomFboRenderer", 10, 40);
        }

        ofDrawBitmapStringHighlight(ofToString(ofGetFrameRate()), 10, 20);
    }

    void keyPressed(int key) override {
      if (key == 's') {
          ofxEquiMap::saveToFile(em, "frame-"+ofToString(ofGetFrameNum())+".jpg");
      }
    }
};

//========================================================================

int main(int argc, char** argv){

    if (argc < 2) {
        ofGLFWWindowSettings settings;
        settings.glVersionMajor = 4;
        settings.glVersionMinor = 1;
        //    settings.windowMode = OF_WINDOW;
        settings.width = 1280;
        settings.height = 720;
        //    settings.multiMonitorFullScreen=true;
        shared_ptr<ofAppBaseWindow> mainWindow = ofCreateWindow(settings);
        mainWindow->setWindowTitle("Multiverse - MAIN");
        mainWindow->setWindowPosition(0, 0);
        shared_ptr<ofApp> mainApp(new ofApp());
        ofRunApp(mainWindow, mainApp);
        ofRunMainLoop();
        return 0;
    }


    ofSetupOpenGL(1280,640,OF_WINDOW);            // <-------- setup the GL context

    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp(new ofApp());
    return 0;

}
