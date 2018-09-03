#pragma once

#include "ofMain.h"
#include "ofxCubeMap.h"

namespace ofxEquiMap
{
    class Scene
    {
    public:
        virtual ~Scene() {}
        virtual void drawEquiScene() = 0;
    };

    class Renderer
    {
    protected:
        Scene* scene = NULL;
        ofxCubeMap cm;
        ofShader warpShader;
    public:
        void setup(int size, Scene* s, int internalformat = GL_RGB);
        virtual void render();
        void draw(float x, float y, float w, float h);

        void setPosition(const ofVec3f& p) {
            ofVec3f p2 = p;
            cm.setPosition(p2);
        }

        const ofFbo& getCubeMapFbo() const { return cm.getFbo(); }

        void registerScene(Scene* s) {scene = s;}
        void setPosition(float x, float y, float z) {cm.setPosition(x, y, z);}
        ofxCubeMap& getCubeMap() {return cm;}

    };

    class CustomFboRenderer : public Renderer
    {
    protected:
        ofFbo fbo;
    public:
        void setup(int size, Scene* s, int internalformat = GL_RGB, int numSamples = 0);
        void setup(int size, Scene* s, ofFbo::Settings fbo_settings);
        void render() override;

        ofFbo& getFbo() { return fbo; }
        const ofFbo& getFbo() const { return fbo; }
    };

    class Exporter {
    private:
      Renderer* renderer;
      ofFbo fbo;
      ofPixels pixels;
      int width = 2048, height = 1024;

    public:
      Exporter(Renderer& r) : renderer(&r) {}
      void update();
      bool toFile(const std::string& fname);

      const ofFbo& getFbo() const { return fbo; }
    };

    bool saveToFile(Renderer& renderer, const std::string& fileName);
};
