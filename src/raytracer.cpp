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
	const IScene& theScene, int N) const {
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

			if (N < 2) {
				Ray ray = camera.getRay(x, y);
				color col = traceIndividualRay(ray, theScene, depth);
				frameBuffer.setColor(x, y, col);
			    //frameBuffer.showAxes(x, y, ray, 0.25);
            }
			else {
				// This is for Antialiasing 
                // Implement this in the project 
                /*
                std::vector<Ray> rays = theScene.camera->getAARays(x, y, N);
				color colorForPixel = black;
				for (auto& ray : rays) {
					colorForPixel += traceIndividualRay(ray, theScene, depth);
				}
				colorForPixel /= rays.size();
				frameBuffer.setColor(x, y, colorForPixel);
			    */
            }
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

        if (hit.texture != nullptr) {
            color texelColor = hit.texture->getPixelUV(hit.u, hit.v);

            hit.material.ambient = 0.15 * texelColor; // 0.15 is arbitrary
            hit.material.diffuse = texelColor;
        }

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
		
            if (hit.material.alpha < 1.0) {
                dvec3 refractionDir = ray.dir;
                Ray refractRay(hit.interceptPt - EPSILON * hit.normal, refractionDir);
                color refractColor = traceIndividualRay(refractRay, theScene, recursionLevel);
                accumulatedColor = (1.0 - hit.material.alpha) * accumulatedColor + hit.material.alpha * refractColor;
            }
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
