// Project Ray Tracing 
// CSE386-B 2025
// Jackson Frey

#include <ctime>
#include "defs.h"
#include "io.h"
#include "ishape.h"
#include "framebuffer.h"
#include "raytracer.h"
#include "iscene.h"
#include "light.h"
#include "image.h"
#include "camera.h"
#include "rasterization.h"

Image im1("usflag.ppm");
Image im2("earth.ppm");

namespace Project {
    int numReflections = 0;
    int antiAliasing = 1;

    dvec3 cameraPos(0,0,0);
    dvec3 cameraFocus(0,0,0);
    dvec3 cameraUp = Y_AXIS;
    double cameraFOV = glm::radians(60.0);

    std::vector<PositionalLight*> lights;

    FrameBuffer frameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
    RayTracer rayTrace(paleGreen);
    IScene scene;
};

/**
* Resizes the display window
* 
* @param width the new window width
* @param height the new window height
*/
void resize(int width, int height) {
    Project::frameBuffer.setFrameBufferSize(width, height);
    glutPostRedisplay();
}

void render() {
    int frameStartTime = glutGet(GLUT_ELAPSED_TIME);
    int width = Project::frameBuffer.getWindowWidth();
    int height = Project::frameBuffer.getWindowHeight();
    Project::frameBuffer.clearColorBuffer();

    Project::scene.camera = new PerspectiveCamera(
        Project::cameraPos, Project::cameraFocus, 
        Project::cameraUp, Project::cameraFOV,
        width, height
    );

    Project::rayTrace.raytraceScene(
        Project::frameBuffer, Project::numReflections,
        Project::scene, Project::antiAliasing
    );

    Project::frameBuffer.showColorBuffer();

    int frameEndTime = glutGet(GLUT_ELAPSED_TIME);
    double totalTime = (frameEndTime - frameStartTime) / 1000.0;
    std::cout << "Render Time: " << totalTime << " seconds" << std::endl;
}

int main(int argc, char** argv) {
    graphicsInit(argc, argv, __FILE__);

    glutDisplayFunc(render);
    glutReshapeFunc(resize);
    glutTimerFunc(TIME_INTERVAL, timer, 0);
}
