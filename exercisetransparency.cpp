/****************************************************
 * 2016-2023 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

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

Image im1("usflag.ppm"/*, true*/);
Image im2("earth.ppm"/*, true)*/);

int currLight = 0;
double angle = 0.5;
const int MINZ = -10;
const int MAXZ = 4;
double z = MINZ;
double inc = 0.4;
bool isAnimated = false;
int numReflections = 0;
int antiAliasing = 1;
bool multiViewOn = false;
double spotDirX = 0;
double spotDirY = -1;
double spotDirZ = 0;

dvec3 cameraPos(16, 3, 16); /**/ dvec3 cameraFocus(0, 0, 0);
dvec3 cameraUp = Y_AXIS;

double cameraFOV = glm::radians(60.0);/**/

vector<PositionalLightPtr> lights = {
						new PositionalLight(dvec3(15, 15, 15), white),
						new SpotLight(dvec3(0, 15, 0)/**/,
										dvec3(spotDirX,spotDirY,spotDirZ),
										glm::radians(90.0),
										white)
};

PositionalLightPtr posLight = lights[0];
SpotLightPtr spotLight = (SpotLightPtr)lights[1];

FrameBuffer frameBuffer(WINDOW_WIDTH, WINDOW_HEIGHT);
RayTracer rayTrace(paleGreen);
IScene scene;

IPlane* plane = new IPlane(dvec3(0.0, -2.0, 0.0), dvec3(0.0, 1.0, 0.0));
IPlane* verticalPlane = new IPlane(dvec3(0.0, 0.0, 9.5), dvec3(0.0, 0.0, 1.0));
IPlane* clearPlane = new IPlane(dvec3(0.0, 0.0, MINZ), dvec3(0.0, 0.0, 1.0));
ISphere* sphere1 = new ISphere(dvec3(0.0, 2.0, 0.0), 4.0);
IEllipsoid* ellipsoid = new IEllipsoid(dvec3(4, 0, 5), dvec3(1, 1, 2.5));
ICylinderY* cylinderY = new ICylinderY(dvec3(-4.0, 1.5, -4.0), 1.5, 3.0);
IDisk* disk = new IDisk(dvec3(-8, 0, 10), dvec3(1, 0, 0), 3);

void resize(int width, int height) {
	frameBuffer.setFrameBufferSize(width, height);
	glutPostRedisplay();
}

void buildScene() {

	Material mirror(color(0.1, 0.1, 0.1), color(0.2, 0.2, 0.3), color(1.0, 1.0, 1.0), 128.0);
	mirror.isDielectric = true;
	mirror.dielectricRefractionIndex = 1.5;
	
	Material greenTransparent(color(0.15, 0.3, 0.15), color(0.55, 0.85, 0.55), color(1.0, 1.0, 1.0), 128.0);

	greenTransparent.alpha = 0.2;
	
	//scene.addOpaqueObject(new VisibleIShape(verticalPlane, greenTransparent));

	scene.addOpaqueObject(new VisibleIShape(plane, tin));

	scene.addOpaqueObject(new VisibleIShape(sphere1, mirror, &im2));
	scene.addOpaqueObject(new VisibleIShape(ellipsoid, mirror));

	scene.addOpaqueObject(new VisibleIShape(cylinderY, gold, &im1));
	scene.addOpaqueObject(new VisibleIShape(disk, redPlastic, &im1));

	scene.addLight(lights[0]);
	scene.addLight(lights[1]);
	lights[0]->isOn = true;
	lights[1]->isOn = true;
}
void render() {

	int frameStartTime = glutGet(GLUT_ELAPSED_TIME);
	int width = frameBuffer.getWindowWidth();
	int height = frameBuffer.getWindowHeight();
	frameBuffer.clearColorBuffer();

	scene.camera = new PerspectiveCamera(cameraPos, cameraFocus, cameraUp, cameraFOV, width, height);
	rayTrace.raytraceScene(frameBuffer, numReflections, scene, antiAliasing);

	frameBuffer.showColorBuffer();
	int frameEndTime = glutGet(GLUT_ELAPSED_TIME); // Get end time
	double totalTimeSec = (frameEndTime - frameStartTime) / 1000.0;
	if (isAnimated) {
		cout << "Transparent plane's z value: " << clearPlane->a.z << endl;
	}
	cout << "Render time: " << totalTimeSec << " sec." << endl;
}
void incrementClamp(double& v, double delta, double lo, double hi) {
	v = glm::clamp(v + delta, lo, hi);
}

void incrementClamp(int& v, int delta, int lo, int hi) {
	v = glm::clamp(v + delta, lo, hi);
}

void timer(int id) {
	if (isAnimated) {
		z += inc;
		if (z <= MINZ) {
			inc = -inc;
		}
		else if (z >= MAXZ) {
			inc = -inc;
		}
	}
	clearPlane->a = dvec3(0, 0, z);
	glutTimerFunc(TIME_INTERVAL, timer, 0);
	glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
	int W, H;
	const double INC = 0.5;
	switch (key) {
	case 'A':
	case 'a':	currLight = 0;
		cout << *lights[0] << endl;
		break;
	case 'B':
	case 'b':	currLight = 1;
		cout << *lights[1] << endl;
		break;
	case 'O':
	case 'o':	lights[currLight]->isOn = !lights[currLight]->isOn;
		cout << (lights[currLight]->isOn ? "ON" : "OFF") << endl;
		break;
	case 'V':
	case 'v':	lights[currLight]->isTiedToWorld = !lights[currLight]->isTiedToWorld;
		cout << (lights[currLight]->isTiedToWorld ? "World" : "Camera") << endl;
		break;
	case 'Q':
	case 'q':	lights[currLight]->attenuationIsTurnedOn = !lights[currLight]->attenuationIsTurnedOn;
		cout << (lights[currLight]->attenuationIsTurnedOn ? "Atten ON" : "Atten OFF") << endl;
		break;
	case 'W':
	case 'w':	incrementClamp(lights[currLight]->atParams.constant, isupper(key) ? INC : -INC, 0.0, 10.0);
		cout << lights[currLight]->atParams << endl;
		break;
	case 'E':
	case 'e':	incrementClamp(lights[currLight]->atParams.linear, isupper(key) ? INC : -INC, 0.0, 10.0);
		cout << lights[currLight]->atParams << endl;
		break;
	case 'R':
	case 'r':	incrementClamp(lights[currLight]->atParams.quadratic, isupper(key) ? INC : -INC, 0.0, 10.0);
		cout << lights[currLight]->atParams << endl;
		break;
	case 'X':
	case 'x': lights[currLight]->pos.x += (isupper(key) ? INC : -INC);
		cout << lights[currLight]->pos << endl;
		break;
	case 'Y':
	case 'y': lights[currLight]->pos.y += (isupper(key) ? INC : -INC);
		cout << lights[currLight]->pos << endl;
		break;
	case 'Z':
	case 'z': lights[currLight]->pos.z += (isupper(key) ? INC : -INC);
		cout << lights[currLight]->pos << endl;
		break;
	case 'J':
	case 'j':	spotDirX += (isupper(key) ? INC : -INC);
		spotLight->setDir(spotDirX, spotDirY, spotDirZ);
		cout << spotLight->spotDir << endl;
		break;
	case 'K':
	case 'k':	spotDirY += (isupper(key) ? INC : -INC);
		spotLight->setDir(spotDirX, spotDirY, spotDirZ);
		cout << spotLight->spotDir << endl;
		break;
	case 'L':
	case 'l':	spotDirZ += (isupper(key) ? INC : -INC);
		spotLight->setDir(spotDirX, spotDirY, spotDirZ);
		cout << spotLight->spotDir << endl;
		break;
	case 'F':
	case 'f':	incrementClamp(spotLight->fov, isupper(key) ? 0.2 : -0.2, 0.1, PI);
		cout << spotLight->fov << endl;
		break;
	case 'P':
	case 'p':	isAnimated = !isAnimated;
		cout << "Animation: " << (isAnimated ? "on" : "off") << endl;
		break;
	case 'C':
	case 'c':
		break;
	case 'U':
	case 'u':	incrementClamp(cameraFOV, isupper(key) ? 0.2 : -0.2, glm::radians(10.0), glm::radians(160.0));
		W = frameBuffer.getWindowWidth();
		H = frameBuffer.getWindowWidth();
		cout << cameraFOV << endl;
		break;
	case 'M':
	case 'm':	break;
	case '+':	antiAliasing = 3;
		cout << "Anti aliasing: " << antiAliasing << endl;
		break;
	case '-':	antiAliasing = 1;
		cout << "Anti aliasing: " << antiAliasing << endl;
		break;

	case '?':	multiViewOn = !multiViewOn;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':	numReflections = key - '0';
		cout << "Num reflections: " << numReflections << endl;
		break;
	case 'd':	isAnimated = !isAnimated;
		break;
	case ESCAPE:
		glutLeaveMainLoop();
		break;
	default:
		cout << (int)key << "unmapped key pressed." << endl;
	}

	glutPostRedisplay();
}

int main(int argc, char* argv[]) {
	graphicsInit(argc, argv, __FILE__);

	glutDisplayFunc(render);
	glutReshapeFunc(resize);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouseUtility);
	glutTimerFunc(TIME_INTERVAL, timer, 0);
	buildScene();

	glutMainLoop();

	return 0;
}
