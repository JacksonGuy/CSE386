/****************************************************
 * 2016-2024 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

#include "light.h"
#include "io.h"
#include "ishape.h"

 /**
  * @fn	color ambientColor(const color &matAmbient, const color &lightColor)
  * @brief	Computes the ambient color produced by a single light at a single point.
  * @param	mat  	Ambient material property.
  * @param	lightColor	Light's color.
  * @return	Ambient color.
   */

color ambientColor(const color& mat, const color& lightColor) {
	return mat * lightColor;
}

/**
 * @fn	color diffuseColor(const color &mat, const color &light, const dvec3 &l, const dvec3 &n)
 * @brief	Computes diffuse color produce by a single light at a single point.
 * @param	mat		 	Material.
 * @param	lightColor	 	The light's color.
 * @param	l		 	Light vector.
 * @param	n		 	Normal vector.
 * @return	Diffuse color.
 */

color diffuseColor(const color& mat, const color& lightColor,
    	const dvec3& l, const dvec3& n) 
{
	return mat * lightColor * glm::max(glm::dot(l, n), 0.0);
}

/**
 * @fn	color specularColor(const color &mat, const color &lightColor, double shininess,
 *							const dvec3 &r, const dvec3 &v)
 * @brief	Computes specular color produce by a single light at a single point.
 * @param	mat		 	Material.
 * @param	lightColor	 	The light's color.
 * @param	shininess	Material shininess.
 * @param	r		 	Reflection vector.
 * @param	v		 	Viewing vector.
 * @return	Specular color.
 */

color specularColor(const color& mat, const color& lightColor,
        double shininess,
        const dvec3& r, const dvec3& v) 
{
	double factor = glm::pow(glm::max(0.0, glm::dot(r, v)), shininess);
    return factor * mat * lightColor;
}

/**
 * @fn	color totalColor(const Material &mat, const color &lightColor,
 *						const dvec3 &viewingDir, const dvec3 &normal,
 *						const dvec3 &lightPos, const dvec3 &intersectionPt,
 *						bool attenuationOn, const LightAttenuationParameters &ATparams)
 * @brief	Color produced by a single light at a single point. This includes the summation
 * of the ambient, diffuse, and specular components. Attenuation is also applied, if applicable.
 * @param	mat			  	Material.
 * @param	lightColor	  	The light's color.
 * @param	v	  			The v vector.
 * @param	n   		  	Normal vector.
 * @param	lightPos	  	Light position.
 * @param	intersectionPt	(x,y,z) of intersection point.
 * @param	attenuationOn 	true if attenuation is on.
 * @param	ATparams	  	Attenuation parameters.
 * @return	Color produced by a single light at a single point. This includes the summation
 * of the ambient, diffuse, and specular components. Attenuation is also applied, if applicable.
 */

color totalColor(const Material& mat, const color& lightColor,
        const dvec3& v, const dvec3& n,
        const dvec3& lightPos, const dvec3& intersectionPt,
        bool attenuationOn, const LightATParams& ATparams) 
{
    // Calculate vectors for color calcs
    dvec3 lightVector = lightPos - intersectionPt;
    dvec3 lightVectorNorm = glm::normalize(lightVector);
    dvec3 reflectionVector = glm::reflect(-lightVectorNorm, n);

    // Calculate Color Values
	color ambientReflection = ambientColor(mat.ambient, lightColor);
    color diffuseReflection = diffuseColor(
        mat.diffuse, lightColor, lightVectorNorm, n
    );
    color specularReflection = specularColor(
        mat.specular, lightColor, mat.shininess, reflectionVector, v
    );

    // Find attenuation
    double attenuationFactor = attenuationOn ? 
        ATparams.factor(glm::length(lightVector)) : 1.0f;

    color finalColor = ambientReflection + 
        ( attenuationFactor * (diffuseReflection + specularReflection));

    return glm::clamp(finalColor, 0.0, 1.0);
}

/**
 * @fn	color PositionalLight::illuminate(const dvec3 &interceptWorldCoords,
 *										const dvec3 &normal, const Material &material,
 *										const Frame &eyeFrame, bool inShadow) const
 * @brief	Computes the color this light produces in RAYTRACING applications.
 * @param	interceptWorldCoords	(x, y, z) at the intercept point.
 * @param	normal				The normal vector.
 * @param	material			The object's material properties.
 * @param	eyeFrame			The coordinate frame of the camera.
 * @param	inShadow			true if the point is in a shadow.
 * @return	The color produced at the intercept point, given this light.
 */

color PositionalLight::illuminate(const dvec3& interceptWorldCoords,
        const dvec3& normal, const Material& material,
        const Frame& eyeFrame, bool inShadow) const 
{
    if (!this->isOn) return black;
    if (inShadow) return ambientColor(material.ambient, this->lightColor);

    // Calculate View Vector
    dvec3 viewVector = eyeFrame.origin - interceptWorldCoords;
    viewVector = glm::normalize(viewVector);
    
    // Position of light adjusted to appropriate coordinate frame
    dvec3 lightPos = actualPosition(eyeFrame); 

    // Calculate light color
    color totalColorCalc = totalColor(
        material, this->lightColor,
        viewVector, normal, lightPos, interceptWorldCoords,
        this->attenuationIsTurnedOn, this->atParams
    );
    
    return totalColorCalc;
}

/*
* @fn PositionalLight::actualPosition(const Frame& eyeFrame) const
* @brief	Returns the global world coordinates of this light.
* @param	eyeFrame	The camera's frame
* @return	The global world coordinates of this light. This will be the light's
*			raw position. Or, it will be the position relative to the camera's
*			frame (transformed from camera's frame into the world coordinate frame).
*/

dvec3 PositionalLight::actualPosition(const Frame& eyeFrame) const {
	return isTiedToWorld ? pos : eyeFrame.frameCoordsToGlobalCoords(pos);
}

/**
* @fn	bool PositionalLight::pointIsInAShadow(const dvec3& intercept, const dvec3& normal, const vector<VisibleIShapePtr>& objects, const Frame& eyeFrame) const
* @brief	Determines if an intercept point falls in a shadow.
* @param	intercept	the position of the intercept.
* @param	normal		the normal vector at the intercept point
* @param	objects		the collection of opaque objects in the scene
*/

bool PositionalLight::pointIsInAShadow(const dvec3& intercept,
	const dvec3& normal,
	const vector<VisibleIShapePtr>& objects,
	const Frame& eyeFrame) const 
{
    dvec3 lightVector = this->pos - intercept;   
    double distToLight = glm::length(lightVector);
    Ray shadowFeeler(intercept + EPSILON * normal, lightVector);

    // Ray shadowFeeler = getShadowFeeler(intercept, normal, eyeFrame);

    OpaqueHitRecord shadowHit;
    VisibleIShape::findIntersection(shadowFeeler, objects, shadowHit);

    if (shadowHit.t > distToLight) {
        return false;
    }

    return true;

    return (shadowHit.t > distToLight) ? false : true;
}

/**
* @fn	Ray PositionalLight::getShadowFeeler(const dvec3& interceptWorldCoords, const dvec3& normal, const Frame &eyeFrame) const
* @brief	Returns the shadow feeler for this light.
* @param	interceptWorldCoords	the position of the intercept.
* @param	normal		The normal vector at the intercept point
* @param	eyeFrame	The coordinate frame of the camera.
*/

Ray PositionalLight::getShadowFeeler(const dvec3& interceptWorldCoords,
	const dvec3& normal,
	const Frame& eyeFrame) const 
{
    dvec3 origin = interceptWorldCoords + EPSILON * normal;
    dvec3 dir = this->pos - interceptWorldCoords;

    Ray shadowFeeler(origin, dir);

	return shadowFeeler;
}

/**
 * @fn	color SpotLight::illuminate(const dvec3 &interceptWorldCoords,
 *									const dvec3 &normal, const Material &material,
 *									const Frame &eyeFrame, bool inShadow) const
 * @brief	Computes the color this light produces in raytracing applications.
 * @param	interceptWorldCoords				The surface properties of the intercept point.
 * @param	normal					The normal vector.
 * @param	material			The object's material properties.
 * @param	eyeFrame			The coordinate frame of the camera.
 * @param	inShadow			true if the point is in a shadow.
 * @return	The color produced at the intercept point, given this light.
 */

color SpotLight::illuminate(const dvec3& interceptWorldCoords,
	const dvec3& normal,
	const Material& material,
	const Frame& eyeFrame,
	bool inShadow) const {
	/* CSE 386 - todo  */
	return black;
}

/**
* @fn	void setDir (double dx, double dy, double dz)
* @brief	Sets the direction of the spotlight.
* @param	dx		x component of the direction
* @param	dy		y component of the direction
* @param	dz		z component of the direction
*/

void SpotLight::setDir(double dx, double dy, double dz) {
	spotDir = glm::normalize(dvec3(dx, dy, dz));
}

/**
* @fn	SpotLight::isInSpotlightCone(const dvec3& spotPos, const dvec3& spotDir, double spotFOV, const dvec3& intercept)
* @brief	Determines if an intercept point falls within a spotlight's cone.
* @param	spotPos		where the spotlight is positioned
* @param	spotDir		normalized direction of spotlight's pointing direction
* @param	spotFOV		spotlight's field of view, which is 2X of the angle from the viewing axis
* @param	intercept	the position of the intercept.
*/

bool SpotLight::isInSpotlightCone(const dvec3& spotPos,
	const dvec3& spotDir,
	double spotFOV,
	const dvec3& intercept) {
	/* CSE 386 - todo  */
	return false;
}
