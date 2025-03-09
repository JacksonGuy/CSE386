/****************************************************
 * 2016-2024 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

#pragma once
#include "defs.h"
#include "colorandmaterials.h"

 /**
  * @struct	VertexData
  * @brief	A vertex data. Used for Pipeline graphics.
  */

struct VertexData {
	dvec4 pos;			//!< Processed coordinate.
	dvec3 normal;		//!< transformed normal vector.
	dvec3 worldPos;		//!< Saved world position, for lighting calculations.
	Material material;	//!< This vertex's material.
	dvec2 textCoord;	//!< Texture coordinate.
	double w;			//!< Perspective correct interpolation factor.

	VertexData(const dvec4& pos, const dvec3& norm,
		const Material& mat, const dvec3& worldPos, const dvec2& textCoord = dvec2(0.0, 0.0));

	VertexData(const dvec4& pos)
		: VertexData(pos, dvec4(0, 0, 1, 0), bronze, ORIGIN3D) { }

	VertexData(const dvec4& pos, const dvec3& norm, const Material& mat, const dvec2& textCoord = dvec2(0.0, 0.0))
		: VertexData(pos, norm, mat, ORIGIN3D, textCoord) {	}

	static VertexData genInterpolatedVertex(const VertexData& vd1, const VertexData& vd2, const double t);

	static void addTriVertsAndComputeNormal(vector<VertexData>& verts,
		const dvec4& V1, const dvec4& V2, const dvec4& V3,
		const Material& mat,
		const dvec2& t1 = dvec2(0.0, 0.0), const dvec2& t2 = dvec2(0.0, 0.0), const dvec2& t3 = dvec2(0.0, 0.0));

	VertexData operator + (const VertexData& other) const;
};

VertexData operator * (double scalar, const VertexData& V1);
