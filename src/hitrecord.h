/****************************************************
 * 2016-2024 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

#pragma once

#include <vector>
#include "defs.h"
#include "colorandmaterials.h"
#include "image.h"
#include "utilities.h"

struct HitRecord {
	double t;				//!< the t value where the intersection took place.
	dvec3 interceptPt;		//!< the (x,y,z) value where the intersection took place.
	dvec3 normal;			//!< the normal vector at the intersection point.

	HitRecord() {
		t = FLT_MAX;
	}
};

enum RAY_STATUS { ENTERING, LEAVING };
	
/**
 * @struct	OpaqueHitRecord
 * @brief	Stores information regarding a ray-object intersection for solid objects.
 * Used in raytracing.
 */

struct OpaqueHitRecord : HitRecord {
	Material material;		//!< the Material value of the object.
	Image* texture;			//!< the texture associated with this object, if any (nullptr when not textured).
	double u, v;			//!< (u,v) correpsonding to intersection point.

	/**
	 * @fn	static HitRecord getClosest(const vector<HitRecord> &hits)
	 * @brief	Gets a closest, give a vector of hits.
	 * @param	hits	The hits to consider.
	 * @return	The closest hit, that is in front of the camera.
	 */

	static OpaqueHitRecord getClosest(const vector<OpaqueHitRecord>& hits) {
		OpaqueHitRecord theClosestHit;
		for (size_t i = 0; i < hits.size(); i++) {
			if (inRangeExclusive(hits[i].t, 0, theClosestHit.t)) {
				theClosestHit = hits[i];
			}
		}
		return theClosestHit;
	}

    /** @brief	Added to support transparency. Indicates whether if the ray   
	    is enter an enclosed object or leaving it. Assumes all rays original 
	    outside of any transparent object.*/
	
	RAY_STATUS rayStatus = ENTERING;
};

/**
 * @struct	OpaqueHitRecord
 * @brief	Stores information regarding a ray-object intersection involving a transparent
 *			object. Used in raytracing.
 */

struct TransparentHitRecord : HitRecord {
	color transColor;		//!< the color of this transparent material
	double alpha;			//!< the alpha value for this transparent material
};
