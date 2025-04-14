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
 * @fn	static double fresnel(const dvec3& i, const dvec3& n, const double& etai, const double& etat)
 *
 * @brief	Compute Fresnel equation
 *
 * @param	i   	is the incident view direction.
 * @param	n   	is the normal at the intersection point.
 * @param	etai	is the refractive index of the material the light is leaving.
 * @param	etat	is the refractive index of the material the light is entering.
 *
 * @returns	kr is the percentage of light reflected As a consequence of the conservation of
 * 			energy, transmittance is given by: kt = 1 - kr
 * 			https://www.scratchapixel.com/lessons/3d-basic-rendering/introduction-to-shading/reflection-refraction-fresnel
 * 			http://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-overview/light-transport-ray-tracing-whitted
 * 			https://graphics.stanford.edu/courses/cs148-10-summer/docs/2006--degreve--reflection_refraction.pdf.
 * 			https://www.cs.cornell.edu/courses/cs4620/2012fa/lectures/36raytracing.pdf
 */
static double fresnel(const dvec3& i, const dvec3& n, const double& etai, const double& etat)
{
	// Percentage of light that is reflected
	// Percentage of light that is refracted is equal to 1-kr
	double kr;

	// Calculate the dot product of the incidence vector and the normal 
	// vector of the surface the the light is entering
	double cosi = glm::clamp(-1.0, 1.0, glm::dot(i, n));

	// Compute the sine of the angle of refraction using Snell's law
	double sint = etai / etat * sqrt(glm::max(0.0, 1.0 - cosi * cosi));

	// Check if angle of incidence exceeds critical angle 
	if (sint >= 1.0) {
		kr = 1.0; // Total internal reflection
	}
	else {
		// Calculate the percentage of light that will be reflected
		double cost = sqrt(glm::max(0.0, 1.0 - sint * sint));
		cosi = fabs(cosi);
		double Rs = ((etat * cosi) - (etai * cost)) / ((etai * cost) + (etat * cosi)); // S polarized light (parallel)
		double Rp = ((etai * cosi) - (etat * cost)) / ((etat * cost) + (etai * cosi)); // P polarized light (perpendicular)
		kr = (Rs * Rs + Rp * Rp) / 2.0;
	}

	return kr;
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
			/*
            Ray reflectionRay(hit.interceptPt + EPSILON * hit.normal, glm::reflect(ray.dir, hit.normal));
			accumulatedColor += 0.5 * traceIndividualRay(reflectionRay, theScene, recursionLevel - 1);
            if (hit.material.alpha < 1.0) {
                dvec3 refractionDir = ray.dir;
                Ray refractRay(hit.interceptPt - EPSILON * hit.normal, refractionDir);
                color refractColor = traceIndividualRay(refractRay, theScene, recursionLevel);
                accumulatedColor = (1.0 - hit.material.alpha) * accumulatedColor + hit.material.alpha * refractColor;
            }
            */
			
            if (hit.material.isDielectric == true) {

				double etai, etat;

				if (hit.rayStatus == ENTERING) {

					etai = 1.0; // Air
					etat = hit.material.dielectricRefractionIndex;
				}
				else { // closestHit.rayStatus == LEAVING

					etai = hit.material.dielectricRefractionIndex;
					etat = 1.0; // Air	

				}

				// Calculate the percentage of reflected light
				double kr = fresnel(ray.dir, hit.normal, etat, etai);

				// Calculate the percentage of refracted (transmitted light) to reflected light
				double kt = (1.0 - kr);

				// Get reflection ray direction
				dvec3 reflection = glm::normalize(glm::reflect(ray.dir, hit.normal));

				// Create reflection ray
				// Avoid "surface acne"
				Ray reflectRay(hit.interceptPt + EPSILON * hit.normal, reflection);

				accumulatedColor += kr * traceIndividualRay(reflectRay, theScene, recursionLevel - 1);

				// Check that this is not a case of total reflection
				if (kr < 1.0) {

					dvec3 refraction = glm::normalize(glm::refract(ray.dir, hit.normal, etat / etai));

					// Create the refracted ray
					// Avoid "surface acne"
					Ray refractRay = Ray(hit.interceptPt + EPSILON * -hit.normal, refraction);

					accumulatedColor += kt * traceIndividualRay(refractRay, theScene, recursionLevel - 1);
				}
			}
            else {
				// ********** Reflection Only ****************
				//dvec3 normal = glm::dot(ray.dir, closestHit.surfaceNormal) > 0 ? closestHit.surfaceNormal : -closestHit.surfaceNormal;

				// Get reflection ray direction
				dvec3 reflection = glm::normalize(glm::reflect(ray.dir, hit.normal));

				// Create reflection ray
				// Avoid "surface acne"
				Ray reflectionRay(hit.interceptPt + EPSILON * hit.normal, reflection);

				// Trace the reflection ray
				accumulatedColor += 0.25 * traceIndividualRay(reflectionRay, theScene, recursionLevel - 1);
			
                //Ray reflectionRay(hit.interceptPt + EPSILON * hit.normal, glm::reflect(ray.dir, hit.normal));
                //accumulatedColor += 0.5 * traceIndividualRay(reflectionRay, theScene, recursionLevel - 1);

                if (hit.material.alpha < 1.0) {
                    dvec3 refractionDir = ray.dir;
                    Ray refractRay(hit.interceptPt - EPSILON * hit.normal, refractionDir);
                    color refractColor = traceIndividualRay(refractRay, theScene, recursionLevel);
                    accumulatedColor = (1.0 - hit.material.alpha) * accumulatedColor + hit.material.alpha * refractColor;
                }
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
