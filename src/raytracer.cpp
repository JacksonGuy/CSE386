/****************************************************
 * 2016-2024 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/
#include "raytracer.h"
#include "ishape.h"
#include "io.h"

 /**
  * @fn	RayTracer::RayTracer(const color &defa)
  * @brief	Constructs a raytracers.
  * @param	defa	The clear color.
  */

RayTracer::RayTracer(const color& defa)
	: defaultColor(defa) {
}

/**
 * @fn	void RayTracer::raytraceScene(FrameBuffer &frameBuffer, int depth, const IScene &theScene) const
 * @brief	Raytrace scene
 * @param [in,out]	frameBuffer	Framebuffer.
 * @param 		  	depth	   	The current depth of recursion.
 * @param 		  	theScene   	The scene.
 */

void RayTracer::raytraceScene(FrameBuffer& frameBuffer, int depth,
	const IScene& theScene) const {
	const RaytracingCamera& camera = *theScene.camera;
	const vector<VisibleIShapePtr>& objs = theScene.opaqueObjs;
	const vector<LightSourcePtr>& lights = theScene.lights;
	color defaultColor = frameBuffer.getClearColor();

	for (int y = 0; y < frameBuffer.getWindowHeight(); ++y) {
		for (int x = 0; x < frameBuffer.getWindowWidth(); ++x) {
			DEBUG_PIXEL = (x == xDebug && y == yDebug);
			if (DEBUG_PIXEL) {
				cout << "";
			}
			/* CSE 386 - todo  */

			Ray ray = camera.getRay(x, y);

			color col = traceIndividualRay(ray, theScene, 3);

			frameBuffer.setColor(x, y, col);

			// frameBuffer.showAxes(x, y, ray, 0.25);			// Displays R/x, G/y, B/z axes
		}
	}

	frameBuffer.showColorBuffer();
}

/**
 * @fn	color RayTracer::traceIndividualRay(const Ray &ray,
 *											const IScene &theScene,
 *											int recursionLevel) const
 * @brief	Trace an individual ray.
 * @param	ray			  	The ray.
 * @param	theScene	  	The scene.
 * @param	recursionLevel	The recursion level.
 * @return	The color to be displayed as a result of this ray.
 */

color RayTracer::traceIndividualRay(const Ray& ray, const IScene& theScene, int recursionLevel) const {
	OpaqueHitRecord hit;

    VisibleIShape::findIntersection(ray, theScene.opaqueObjs, hit);

	if (hit.t < FLT_MAX) {
        color accumulatedColor = black;

        for (auto& light : theScene.lights) {
            dvec3 normal = hit.normal;

            // Check if backside
            if (glm::dot(ray.dir, hit.normal) > 0) {
                normal = -normal;
            }

            
            bool inShadow = light->pointIsInAShadow(
                hit.interceptPt,
                /* hit.normal, */
                normal,
                theScene.opaqueObjs,
                theScene.camera->getFrame()
            );

            accumulatedColor += light->illuminate(
                hit.interceptPt,
                /* hit.normal, */
                normal,
                hit.material,
                theScene.camera->getFrame(),
                inShadow
            );
        }

		if (recursionLevel > 0) {
			Ray reflectionRay(hit.interceptPt + EPSILON * hit.normal, glm::reflect(ray.dir, hit.normal));

			accumulatedColor += 0.5 * traceIndividualRay(reflectionRay, theScene, recursionLevel - 1);
		}

		// return accumulatedColor;
	    return glm::clamp(accumulatedColor, 0.0, 1.0);
    }
	else
	{
		if (recursionLevel == 3) {
			return defaultColor;
		}
		else {
			return defaultColor * 0.1;
		}
	}
}
