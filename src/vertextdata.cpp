/****************************************************
 * 2016-2024 Eric Bachmann and Mike Zmuda
 * All Rights Reserved.
 * NOTICE:
 * Dissemination of this information or reproduction
 * of this material is prohibited unless prior written
 * permission is granted.
 ****************************************************/

#include "vertexdata.h"
#include "utilities.h"
#include "ishape.h"

 /**
  * @fn	VertexData::VertexData(const dvec4 &P, const dvec4 &norm,
								 const Material &mat, const dvec3 &WP)
  * @brief	Constructor
  * @param	P			Current coordinate.
  * @param	norm		Normal vector
  * @param	mat			Material
  * @param	WP			World position.
  * @param	textCoord	Texture coordinate		
  */

VertexData::VertexData(const dvec4& P,
	const dvec3& norm,
	const Material& mat,
	const dvec3& WP,
	const dvec2& textCoord)
	: pos(P), normal(glm::normalize(norm)), material(mat), worldPos(WP), textCoord(textCoord), w(1.0)
{
}


/**
 * @fn	VertexData VertexData::genInterpolatedVertex(const VertexData& vd1, const VertexData& vd2, const double t)
 *
 * @brief	Called when a new vertex it generated during the clipping process.
 * 			Generates an interpolated vertex based on the parameter t
 * 			using linear interpolation to determine the NDC coordinates
 * 			for the new vertex and perspective-correct interpolation
 * 			to the determine the values of the other attributes.
 *
 * @param 	vd1	Vertex on the line segment on which the new vertex is being generated.
 * @param 	vd2	Vertex on the line segment on which the new vertex is being generated.
 * @param 	t  	parameter for the point of intersection.
 *
 * @returns	The interpolated vertex.
 */
VertexData VertexData::genInterpolatedVertex(const VertexData& vd1, const VertexData& vd2, const double t)
{
	// Generate the ndc position for the point of intersection using linear interpolation
	dvec4 ndcPos = (1.0 - t) * vd1.pos + t * vd2.pos;
	
	// Perspective-correct interpolation for all *other* attributes
	// using the stored clip-space scalar values
	double alpha = (1.0 - t) / vd1.w;
	double beta = t / vd2.w;
	double denom = alpha + beta;

	// Weighted sum of VertexData
	VertexData I = alpha * vd1 + beta * vd2;

	// Normalize the weighted sum
	I = (1.0 / denom) * I;

	// Update the new VertexData's w to the perspective-correct value
	I.w = 1.0 / denom;

	return I;

}



/**
 * @fn	void VertexData::addTriVertsAndComputeNormal(vector<VertexData>& verts, 
 * 		const dvec4& V1, const dvec4& V2, const dvec4& V3, 
 * 		const Material& mat, 
 * 		const dvec2& t1, const dvec2 & t2, const dvec2 & t3)
 * @brief	Adds a triangle vertices and computes normal, adding vertices to end of verts.
 *          Vertices are specified in counterclockwise order.
 * @param [in,out]	verts	The vector of vertices.
 * @param 		  	V1   	The first vertice
 * @param 		  	V1   	The first vertice.
 * @param 		  	V2   	The second vertice.
 * @param 		  	V3   	The third vertice.
 * @param 		  	mat  	Material.
 * @param 		  	t1   	Texture coordinate for V1.
 * @param 		  	t2   	Texture coordinate for V2.
 * @param 		  	t3   	Texture coordinate for V3.

 */

void VertexData::addTriVertsAndComputeNormal(vector<VertexData>& verts,
	const dvec4& V1,
	const dvec4& V2,
	const dvec4& V3,
	const Material& mat,
	const dvec2& t1,
	const dvec2& t2,
	const dvec2& t3)
{
	dvec3 n = normalFrom3Points(V1.xyz(), V2.xyz(), V3.xyz());
	dvec4 nHomogenous(n.x, n.y, n.z, 0);
	verts.push_back(VertexData(V1, nHomogenous, mat, t1));
	verts.push_back(VertexData(V2, nHomogenous, mat, t2));
	verts.push_back(VertexData(V3, nHomogenous, mat, t3));
}

/**
 * @fn	VertexData operator* (double scalar, const VertexData &data)
 * @brief	Multiplication operator for VertexData objects
 *  * @param	scalar   	The scalar multiplier.
 * @param	data	Vertex data to scale.
 * @return	The scaled Vertex data.
 */
VertexData operator * (double scalar, const VertexData& data) {
	VertexData result(scalar * data.pos, scalar * data.normal, scalar * data.material, scalar * data.worldPos, scalar * data.textCoord);
	return result;
}


/**
 * @fn	VertexData VertexData::operator+ (const VertexData &other) const
 * @brief	Addition operator for VertexData objects
 * @param	other	The 2nd VertexData object.
 * @return	The raw summation of the two VertexData objects
 */

VertexData VertexData::operator + (const VertexData& other) const {
	VertexData result(*this);
	result.material += other.material;
	result.normal += other.normal;
	result.pos += other.pos;
	result.worldPos += other.worldPos;
	result.textCoord += other.textCoord;
	return result;
}
