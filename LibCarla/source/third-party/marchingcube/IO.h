#pragma once
#include <string>
#include "DataStructs.h"

namespace MeshReconstruction
{
	/// Writes a mesh to a file in <a href="https://de.wikipedia.org/wiki/Wavefront_OBJ">obj format</a>.
	void WriteObjFile(Mesh const& mesh, std::string const& file);


}

using namespace std;

void MeshReconstruction::WriteObjFile(Mesh const& mesh, string const& fileName)
{
	// FILE faster than streams.

	FILE* file;
	auto status = fopen_s(&file, fileName.c_str(), "w");
	if (status != 0)
	{
		throw runtime_error("Could not write obj file.");
	}

	// write stats
	fprintf(file, "# %d vertices, %d triangles\n\n",
		static_cast<int>(mesh.vertices.size()),
		static_cast<int>(mesh.triangles.size()));

	// vertices
	for (auto vi = 0; vi < mesh.vertices.size(); ++vi)
	{
		auto const& v = mesh.vertices.at(vi);
		fprintf(file, "v %f %f %f\n", v.x, v.y, v.z);
	}

	// vertex normals
	fprintf(file, "\n");
	for (auto ni = 0; ni < mesh.vertices.size(); ++ni)
	{
		auto const& vn = mesh.vertexNormals.at(ni);
		fprintf(file, "vn %f %f %f\n", vn.x, vn.y, vn.z);
	}

	// triangles (1-based)
	fprintf(file, "\n");
	for (auto ti = 0; ti < mesh.triangles.size(); ++ti)
	{
		auto const& t = mesh.triangles.at(ti);
		fprintf(file, "f %d//%d %d//%d %d//%d\n",
			t[0] + 1, t[0] + 1,
			t[1] + 1, t[1] + 1,
			t[2] + 1, t[2] + 1);
	}

	fclose(file);
}