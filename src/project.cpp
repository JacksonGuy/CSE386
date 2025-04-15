// Project Ray Tracing 
// CSE386-B 2025
// Jackson Frey

#include <GL/freeglut_std.h>
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

namespace Project {
    int numReflections = 0;
    int antiAliasing = 1;
    const int MINZ = -10;
    const int MAXZ = 4;
    double clearPlaneZ = MINZ;
    double inc = 0.5;
    bool isAnimated = false;

    dvec3 cameraPos(16,8,16);
    dvec3 cameraFocus(0,0,0);
    dvec3 cameraUp = Y_AXIS;
    double cameraFOV = glm::radians(60.0);

    std::vector<PositionalLight*> lights;
    SpotLight* spotlight = nullptr;
    int currentLight = 0;
    double spotDirX = 0;
    double spotDirY = -1;
    double spotDirZ = 0;
    double spotAngle = 120;

    IPlane* clearPlane = nullptr;

    FrameBuffer frameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
    RayTracer rayTrace(paleGreen);
    IScene scene;
};

/**
* @brief Resizes the display window
* 
* @param width the new window width
* @param height the new window height
*/
void resize(int width, int height) {
    Project::frameBuffer.setFrameBufferSize(width, height);
    glutPostRedisplay();
}

/**
* @brief Renders the scene
*/
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
    if (Project::isAnimated) {
        std::cout << "Transparent plane's z value: " << Project::clearPlane->a.z << std::endl;
    }
    std::cout << "Render Time: " << totalTime << " seconds" << std::endl;

    free(Project::scene.camera);
}

/**
 * @brief Update function that is called at regular interval. Refreshes display.
 * @param id
 */
void timer(int id) {
    if (Project::isAnimated) {
        Project::clearPlaneZ += Project::inc;
        if (Project::clearPlaneZ <= Project::MINZ) {
            Project::inc = -Project::inc;
        }
        else if (Project::clearPlaneZ >= Project::MAXZ) {
            Project::inc = -Project::inc;
        }
    }
    Project::clearPlane->a = dvec3(0, 0, Project::clearPlaneZ);
    glutTimerFunc(TIME_INTERVAL, timer, 0);
    glutPostRedisplay();
}

/**
* @brief Handles keyboard input
*
* @param key the keycode of the key pressed
* @param x 
* @param y
*/
void keyboard(unsigned char key, int x, int y) {
    using namespace Project;
    const double INC = 0.5;

    switch (key) {
        // Switch current light
        case 'A':
        case 'a':
            Project::currentLight = 0;
            std::cout << *lights[currentLight] << std::endl;
            break;
        case 'B':
        case 'b':
            Project::currentLight = 1;
            std::cout << *lights[currentLight] << std::endl;
            break;

        // Toggle current light
        case 'O':
        case 'o':
            lights[currentLight]->isOn = !lights[currentLight]->isOn;
            std::cout << (lights[currentLight]->isOn ? "ON" : "OFF") << std::endl;
            break;

        // Move current camera
        case 'X':
        case 'x':
            lights[currentLight]->pos.x += (isupper(key) ? INC : -INC);
            std::cout << lights[currentLight]->pos << std::endl;
            break;
        case 'Y':
        case 'y':
            lights[currentLight]->pos.y += (isupper(key) ? INC : -INC);
            std::cout << lights[currentLight]->pos << std::endl;
            break;
        case 'Z':
        case 'z':
            lights[currentLight]->pos.z += (isupper(key) ? INC : -INC);
            std::cout << lights[currentLight]->pos << std::endl;
            break;

        // Change spotlight direction
        case 'J':
        case 'j':
            spotDirX += (isupper(key) ? INC : -INC);
            spotlight->setDir(spotDirX, spotDirY, spotDirZ);
            std::cout << spotlight->spotDir << std::endl;
            break;
        case 'K':
        case 'k':
            spotDirY += (isupper(key) ? INC : -INC);
            spotlight->setDir(spotDirX, spotDirY, spotDirZ);
            std::cout << spotlight->spotDir << std::endl;
            break; 
        case 'L':
        case 'l':
            spotDirZ += (isupper(key) ? INC : -INC);
            spotlight->setDir(spotDirX, spotDirY, spotDirZ);
            std::cout << spotlight->spotDir << std::endl;
            break;

        // Toggle animation
        case 'P':
        case 'p':
            isAnimated = !isAnimated;
            std::cout << "Animation: " << (isAnimated ? "On" : "Off") << std::endl;
            break; 

        // AntiAliasing
        case '+':
            antiAliasing = 3;
            std::cout << "Anti Aliasing: " << antiAliasing << std::endl;
            break;
        case '-':
            antiAliasing = 1;
            std::cout << "Anti Aliasing: " << antiAliasing << std::endl;
            break;

        // Reflections
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
            numReflections = key - '0';
            std::cout << "Num reflections: " << numReflections << std::endl;
            break;

        // Exit program
        case ESCAPE:
            glutLeaveMainLoop();
            break;
        
        default:
            std::cout << (int)key << " unmapped key pressed" << std::endl;
    }

    glutPostRedisplay();
}

int main(int argc, char** argv) {
    graphicsInit(argc, argv, __FILE__);

    glutDisplayFunc(render);
    glutReshapeFunc(resize);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseUtility);
    glutTimerFunc(TIME_INTERVAL, timer, 0);

    // Load Images
    Image im1("usflag.ppm");
    Image im2("earth.ppm");

    // Custom Materials
    Material mirror(color(0.1, 0.1, 0.1), color(0.2, 0.2, 0.3), color(1.0, 1.0, 1.0), 128.0);
	mirror.isDielectric = true;
	mirror.dielectricRefractionIndex = 1.5;

    // Create Lights
    PositionalLight* posLight = new PositionalLight(dvec3(15, 15, 15), white);
    Project::lights.push_back(posLight);
    Project::scene.addLight(posLight);
    posLight->isOn = true;

    SpotLight* spotlight = new SpotLight(
        dvec3(-15, 5, 10),
        dvec3(Project::spotDirX, Project::spotDirY, Project::spotDirZ),
        glm::radians(Project::spotAngle),
        white
    );
    Project::lights.push_back(spotlight);
    Project::scene.addLight(spotlight);
    Project::spotlight = spotlight;
    spotlight->isOn = true;

    // Create Objects
    Project::clearPlane = new IPlane(dvec3(0.0, 0.0, Project::MINZ), dvec3(0.0, 0.0, 1.0));
    IPlane* plane = new IPlane(dvec3(0.0, -2.0, 0.0), dvec3(0.0, 1.0, 0.0));
    ISphere* sphere1 = new ISphere(dvec3(0.0, 2.0, 0.0), 4.0);
    IClosedCylinderY* cylinder1 = new IClosedCylinderY(dvec3(8.0, 0.0, 4.0), 1.5, 3.0);
    ICylinderY* cylinder2 = new ICylinderY(dvec3(2.0, 0.0, 8.0), 1.5, 3.0);
    IDisk* disk = new IDisk(dvec3(-8, 1.0, 8.0), dvec3(1, 0, 0), 3);

    // Add objects to scene
    Project::scene.addOpaqueObject(new VisibleIShape(plane, tin));
    Project::scene.addOpaqueObject(new VisibleIShape(sphere1, white, &im2));
    Project::scene.addOpaqueObject(new VisibleIShape(cylinder1, copper));
    Project::scene.addOpaqueObject(new VisibleIShape(cylinder2, copper, &im1));
    Project::scene.addOpaqueObject(new VisibleIShape(disk, mirror));

    // Start Program
    glutMainLoop();

    return 0;
}
