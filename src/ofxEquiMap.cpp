#include "ofxEquiMap.h"

#ifndef STRINGIFY
#define STRINGIFY(A) #A
#endif

namespace ofxEquiMap {

    // shaders

    static string warp_frag_shader_str = STRINGIFY(
        uniform samplerCube envMap;

        void main() {
            vec2 tc = gl_TexCoord[0].st / vec2(2.0) + 0.5;  //only line modified from the shader toy example
            vec2 thetaphi = ((tc * 2.0) - vec2(1.0)) * vec2(3.1415926535897932384626433832795, 1.5707963267948966192313216916398);
            vec3 rayDirection = vec3(cos(thetaphi.y) * cos(thetaphi.x), sin(thetaphi.y), cos(thetaphi.y) * sin(thetaphi.x));
            gl_FragColor = textureCube(envMap, rayDirection);
        }
    );

    string warp_vert_shader_str330 = string("#version 330\nprecision highp float;\n") + STRINGIFY(

        layout(location = 0) in vec3  position;
        // layout(location = 1) in vec4  color;
        // layout(location = 2) in vec3  normal;
        layout(location = 1) in vec2  texcoord;

        // // uniform mat4 projectionMatrix;
        // // uniform mat4 modelViewMatrix;
        uniform mat4 modelViewProjectionMatrix;
        // // uniform mat4 normalMatrix;
        uniform vec2 dimmensions;

        out vec3 verttexcoord;

        void main() {
            gl_Position = modelViewProjectionMatrix * vec4(position, 1);
            // vertex.texcoord = texcoord;
            verttexcoord  = vec3(position.xy/dimmensions, 0); // normalize(position);
        }
    );

    string warp_frag_shader_str330 = string("#version 330\nprecision highp float;\n") + STRINGIFY(

        uniform samplerCube envMap;
        in vec3 verttexcoord;
        out vec4 fragColor;

        void main (void)
        {
            // fragColor = vec4(1,0,0,1);

            vec2 tc = verttexcoord.st;  //only line modified from the shader toy example
            vec2 thetaphi = ((tc * 2.0) - vec2(1.0)) * vec2(3.1415926535897932384626433832795, 1.5707963267948966192313216916398);
            vec3 rayDirection = vec3(cos(thetaphi.y) * cos(thetaphi.x), sin(thetaphi.y), cos(thetaphi.y) * sin(thetaphi.x));
            fragColor = texture(envMap, rayDirection); //vec4(1,0,0,1);
        }
    );

    // Renderer

    void Renderer::setup(int size, Scene* s, int internalformat)
    {
        int type = ofGetGlTypeFromInternal(internalformat);
        int format = ofGetGLFormatFromInternal(internalformat);
        cm.init(size, format, type);
        if (ofIsGLProgrammableRenderer()) {
            // ofLogWarning() << "using programmable GL renderer";
            warpShader.setupShaderFromSource(GL_VERTEX_SHADER, warp_vert_shader_str330 );
            warpShader.setupShaderFromSource(GL_FRAGMENT_SHADER, warp_frag_shader_str330 );
            // warpShader.bindDefaults();
        } else {
            // warpShader.setupShaderFromSource(GL_VERTEX_SHADER, warp_vert_shader_str );
            warpShader.setupShaderFromSource(GL_FRAGMENT_SHADER, warp_frag_shader_str);
        }
        warpShader.linkProgram();
        registerScene(s);
    }

    void Renderer::render(std::function<void()> func) {
        for (int i = 0; i < 6; i++) {
            cm.beginDrawingInto3D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i );
            ofClear(0);
            // work around for ofLight issue caused by ofxCubeMap
            ofLoadViewMatrix(ofGetCurrentMatrix(OF_MATRIX_MODELVIEW));
            func();
            cm.endDrawingInto3D();
        }
    }

    void Renderer::render(std::function<void(ofCamera&)> func) {
        ofCamera cam;

        for (int i = 0; i < 6; i++) {
            cm.beginDrawingInto3D( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i );
            ofClear(0);
            // work around for ofLight issue caused by ofxCubeMap
            ofLoadViewMatrix(ofGetCurrentMatrix(OF_MATRIX_MODELVIEW));
            cm.loadFaceCamera(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cam);
            func(cam);
            cm.endDrawingInto3D();
        }
    }

    void Renderer::render() {
        if (!scene) {
            ofLogWarning() << "[ofxEquiMap::Renderer::render] no scene";
            return;
        }

        this->render([this](){ this->scene->drawEquiScene(); });
    }

    void Renderer::draw(float x, float y, float w, float h) {
        if (ofIsGLProgrammableRenderer()) {
            warpShader.begin();
            warpShader.setUniform1i("envMap", 0);
            warpShader.setUniform2f("dimmensions", w, h);
            cm.bind();
            ofDrawRectangle(x,y,w,h);
            warpShader.end();
        } else {
            warpShader.begin();
            warpShader.setUniform1i("envMap", 0);
            cm.drawFace(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, x, y, w, h);
            warpShader.end();
        }
    }

    // CustomFboRenderer

    void CustomFboRenderer::setup(int size, Scene* s, int internalformat, int numSamples)
    {
        Renderer::setup(size, s, internalformat);
        fbo.allocate(cm.getWidth(), cm.getHeight(), internalformat, numSamples);
    }

    void CustomFboRenderer::setup(int size, Scene* s, ofFbo::Settings fbo_settings)
    {
        Renderer::setup(size, s, fbo_settings.internalformat);
        fbo.allocate(fbo_settings);
    }

    void CustomFboRenderer::render() {
        this->render([this](){ this->scene->drawEquiScene(); });
    }

    void CustomFboRenderer::render(std::function<void()> func) {
        for (int i = 0; i < 6; ++i) {
            fbo.begin();
            ofClear(0);
            ofPushView();

            glMatrixMode( GL_PROJECTION );
            glLoadIdentity();
            glLoadMatrixf( cm.getProjectionMatrix().getPtr() );

            glMatrixMode( GL_MODELVIEW );
            glLoadIdentity();
            glLoadMatrixf( cm.getLookAtMatrixForFace( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i).getPtr() );

            // work around for ofLight issue caused by ofxCubeMap
            ofLoadViewMatrix(ofGetCurrentMatrix(OF_MATRIX_MODELVIEW));

            scene->drawEquiScene();

            ofPopView();
            fbo.end();

            cm.beginDrawingInto2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
            ofClear(0);
            fbo.draw(0, 0);
            cm.endDrawingInto2D();
        }
    }

    // Exporter

    void Exporter::update() {
        if (!fbo.isAllocated()) {
          fbo.allocate(width, height, GL_RGB);
        }

        // pixels.clear();
        fbo.begin();
        renderer->draw(0,0, width, height);
        fbo.end();
    }

    bool Exporter::toFile(const std::string& fname) {
        if (!pixels.isAllocated()) {
          pixels.allocate(height, height, OF_IMAGE_COLOR);
        }

        //get the frame buffer pixels
        fbo.readToPixels(pixels);
        ofSaveImage(pixels, fname, OF_IMAGE_QUALITY_BEST);
        return true;
    }

    // functions

    bool saveToFile(Renderer& renderer, const std::string& fileName) {
        auto ex = Exporter(renderer);
        ex.update();
        return ex.toFile(fileName);
   }
}
