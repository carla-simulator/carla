#ifndef MC_CPP_INCLUDED
#define MC_CPP_INCLUDED

#include <vector>
#include <cmath>

namespace MC
{
	// Optionally define double precision
#ifdef MC_CPP_USE_DOUBLE_PRECISION
	typedef double MC_FLOAT;
#else
	typedef float MC_FLOAT;
#endif

	typedef unsigned int muint;

	typedef struct mcVec3f
	{
	public:
		union 
		{
			MC_FLOAT v[3];
			struct 
			{
				MC_FLOAT x, y, z;
			};
		};
		inline mcVec3f& operator+=(const mcVec3f& r)
		{
			x += r.x; y += r.y; z += r.z;
			return *this;
		}
		inline MC_FLOAT& operator[](int i)
		{
			return v[i];
		}
	} mcVec3f;

	static inline MC_FLOAT mc_internalLength2(const mcVec3f& v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}
	static inline MC_FLOAT mc_internalLength(const mcVec3f& v)
	{
		return std::sqrt(mc_internalLength2(v));
	}
	static inline mcVec3f mc_internalNormalize(const mcVec3f& v)
	{
		MC_FLOAT vv = mc_internalLength(v);
		return mcVec3f({ v.x / vv, v.y / vv, v.z / vv });
	}
	static inline mcVec3f mc_internalCross(const mcVec3f& v1, const mcVec3f& v2)
	{
		return mcVec3f({ v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x });
	}
	inline mcVec3f operator-(const mcVec3f& l, const mcVec3f r)
	{
		return mcVec3f({ l.x - r.x, l.y - r.y, l.z - r.z });
	}

	typedef struct mcVec3i
	{
	public:
		union 
		{
			muint v[3];
			struct 
			{
				muint x, y, z;
			};
		};
		inline muint& operator[](int i) { return v[i]; }
	} mcVec3i;

	typedef struct mcMesh
	{
	public:
		std::vector<mcVec3f> vertices;
		std::vector<mcVec3f> normals;
		std::vector<muint> indices;
	} mcMesh;

	void marching_cube(MC_FLOAT* field, muint nx, muint ny, muint nz, mcMesh& outputMesh);
	void setDefaultArraySizes(muint vertSize, muint normSize, muint triSize);
}


#endif

namespace MC
{
#ifdef MC_IMPLEM_ENABLE
	static muint defaultVerticeArraySize	= 100000;
	static muint defaultNormalArraySize		= 100000;
	static muint defaultTriangleArraySize	= 400000;

	static inline muint mc_internalToIndex1D(muint i, muint j, muint k, const mcVec3i& size)
	{
		return (k * size.y + j) * size.x + i;
	}

	static inline muint mc_internalToIndex1DSlab(muint i, muint j, muint k, const mcVec3i& size)
	{
		return size.x * size.y * (k % 2) + j * size.x + i;
	}

	// Look-up table for triangle configurations
	static const unsigned long long mc_internalMarching_cube_tris[256] = 
	{
		0ULL, 33793ULL, 36945ULL, 159668546ULL,
		18961ULL, 144771090ULL, 5851666ULL, 595283255635ULL,
		20913ULL, 67640146ULL, 193993474ULL, 655980856339ULL,
		88782242ULL, 736732689667ULL, 797430812739ULL, 194554754ULL,
		26657ULL, 104867330ULL, 136709522ULL, 298069416227ULL,
		109224258ULL, 8877909667ULL, 318136408323ULL, 1567994331701604ULL,
		189884450ULL, 350847647843ULL, 559958167731ULL, 3256298596865604ULL,
		447393122899ULL, 651646838401572ULL, 2538311371089956ULL, 737032694307ULL,
		29329ULL, 43484162ULL, 91358498ULL, 374810899075ULL,
		158485010ULL, 178117478419ULL, 88675058979ULL, 433581536604804ULL,
		158486962ULL, 649105605635ULL, 4866906995ULL, 3220959471609924ULL,
		649165714851ULL, 3184943915608436ULL, 570691368417972ULL, 595804498035ULL,
		124295042ULL, 431498018963ULL, 508238522371ULL, 91518530ULL,
		318240155763ULL, 291789778348404ULL, 1830001131721892ULL, 375363605923ULL,
		777781811075ULL, 1136111028516116ULL, 3097834205243396ULL, 508001629971ULL,
		2663607373704004ULL, 680242583802939237ULL, 333380770766129845ULL, 179746658ULL,
		42545ULL, 138437538ULL, 93365810ULL, 713842853011ULL,
		73602098ULL, 69575510115ULL, 23964357683ULL, 868078761575828ULL,
		28681778ULL, 713778574611ULL, 250912709379ULL, 2323825233181284ULL,
		302080811955ULL, 3184439127991172ULL, 1694042660682596ULL, 796909779811ULL,
		176306722ULL, 150327278147ULL, 619854856867ULL, 1005252473234484ULL,
		211025400963ULL, 36712706ULL, 360743481544788ULL, 150627258963ULL,
		117482600995ULL, 1024968212107700ULL, 2535169275963444ULL, 4734473194086550421ULL,
		628107696687956ULL, 9399128243ULL, 5198438490361643573ULL, 194220594ULL,
		104474994ULL, 566996932387ULL, 427920028243ULL, 2014821863433780ULL,
		492093858627ULL, 147361150235284ULL, 2005882975110676ULL, 9671606099636618005ULL,
		777701008947ULL, 3185463219618820ULL, 482784926917540ULL, 2900953068249785909ULL,
		1754182023747364ULL, 4274848857537943333ULL, 13198752741767688709ULL, 2015093490989156ULL,
		591272318771ULL, 2659758091419812ULL, 1531044293118596ULL, 298306479155ULL,
		408509245114388ULL, 210504348563ULL, 9248164405801223541ULL, 91321106ULL,
		2660352816454484ULL, 680170263324308757ULL, 8333659837799955077ULL, 482966828984116ULL,
		4274926723105633605ULL, 3184439197724820ULL, 192104450ULL, 15217ULL,
		45937ULL, 129205250ULL, 129208402ULL, 529245952323ULL,
		169097138ULL, 770695537027ULL, 382310500883ULL, 2838550742137652ULL,
		122763026ULL, 277045793139ULL, 81608128403ULL, 1991870397907988ULL,
		362778151475ULL, 2059003085103236ULL, 2132572377842852ULL, 655681091891ULL,
		58419234ULL, 239280858627ULL, 529092143139ULL, 1568257451898804ULL,
		447235128115ULL, 679678845236084ULL, 2167161349491220ULL, 1554184567314086709ULL,
		165479003923ULL, 1428768988226596ULL, 977710670185060ULL, 10550024711307499077ULL,
		1305410032576132ULL, 11779770265620358997ULL, 333446212255967269ULL, 978168444447012ULL,
		162736434ULL, 35596216627ULL, 138295313843ULL, 891861543990356ULL,
		692616541075ULL, 3151866750863876ULL, 100103641866564ULL, 6572336607016932133ULL,
		215036012883ULL, 726936420696196ULL, 52433666ULL, 82160664963ULL,
		2588613720361524ULL, 5802089162353039525ULL, 214799000387ULL, 144876322ULL,
		668013605731ULL, 110616894681956ULL, 1601657732871812ULL, 430945547955ULL,
		3156382366321172ULL, 7644494644932993285ULL, 3928124806469601813ULL, 3155990846772900ULL,
		339991010498708ULL, 10743689387941597493ULL, 5103845475ULL, 105070898ULL,
		3928064910068824213ULL, 156265010ULL, 1305138421793636ULL, 27185ULL,
		195459938ULL, 567044449971ULL, 382447549283ULL, 2175279159592324ULL,
		443529919251ULL, 195059004769796ULL, 2165424908404116ULL, 1554158691063110021ULL,
		504228368803ULL, 1436350466655236ULL, 27584723588724ULL, 1900945754488837749ULL,
		122971970ULL, 443829749251ULL, 302601798803ULL, 108558722ULL,
		724700725875ULL, 43570095105972ULL, 2295263717447940ULL, 2860446751369014181ULL,
		2165106202149444ULL, 69275726195ULL, 2860543885641537797ULL, 2165106320445780ULL,
		2280890014640004ULL, 11820349930268368933ULL, 8721082628082003989ULL, 127050770ULL,
		503707084675ULL, 122834978ULL, 2538193642857604ULL, 10129ULL,
		801441490467ULL, 2923200302876740ULL, 1443359556281892ULL, 2901063790822564949ULL,
		2728339631923524ULL, 7103874718248233397ULL, 12775311047932294245ULL, 95520290ULL,
		2623783208098404ULL, 1900908618382410757ULL, 137742672547ULL, 2323440239468964ULL,
		362478212387ULL, 727199575803140ULL, 73425410ULL, 34337ULL,
		163101314ULL, 668566030659ULL, 801204361987ULL, 73030562ULL,
		591509145619ULL, 162574594ULL, 100608342969108ULL, 5553ULL,
		724147968595ULL, 1436604830452292ULL, 176259090ULL, 42001ULL,
		143955266ULL, 2385ULL, 18433ULL, 0ULL,
	};

	/*!
	\brief Approximates the vertex position of the mesh from the scalar values along an edge (va, vb).
	\param slab_inds slab indices global array
	\param mesh the mesh
	\param va, vb edges values
	\param axis axis index 0/1/2
	\param x, y, z current slab index
	\param size slab indices array size
	*/
	static void mc_internalComputeEdge(mcVec3i* slab_inds, mcMesh& mesh, float va, float vb, int axis, muint x, muint y, muint z, const mcVec3i& size)
	{
		if ((va < 0.0) == (vb < 0.0))
			return;
		mcVec3f v = { MC_FLOAT(x), MC_FLOAT(y), MC_FLOAT(z) };
		v[axis] += va / (va - vb);
		slab_inds[mc_internalToIndex1DSlab(x, y, z, size)][axis] = muint(mesh.vertices.size());
		mesh.vertices.push_back(v);
		mesh.normals.push_back(mcVec3f({ 0, 0, 0 }));
	}

	/*!
	\brief Computes and acumulates the geometric normal of triangle formed by vertices (a, b, c).
	\param mesh the mesh
	\param a, b, c vertex indices
	*/
	static inline void mc_internalAccumulateNormal(mcMesh& mesh, muint a, muint b, muint c)
	{
		mcVec3f& va = mesh.vertices[a];
		mcVec3f& vb = mesh.vertices[b];
		mcVec3f& vc = mesh.vertices[c];
		mcVec3f ab = va - vb;
		mcVec3f cb = vc - vb;
		mcVec3f n = mc_internalCross(cb, ab);
		mesh.normals[a] += n;
		mesh.normals[b] += n;
		mesh.normals[c] += n;
	}


	/*
	\brief Stores the default array sizes for the indexed mesh computed
	by the marching cubes. Useful for speeding-up the marching cubes.
	\param vertSize vertex array size
	\param normSize normal array size
	\param triSize triangle index array size
	*/
	inline void setDefaultArraySizes(muint vertSize, muint normSize, muint triSize)
	{
		defaultVerticeArraySize		= vertSize;
		defaultNormalArraySize		= normSize;
		defaultTriangleArraySize	= triSize;
	}

	/*!
	\brief Computes the mesh representing the zero isosurface of a 3D scalarfield and
	 outputs it to an indexed mesh.
	\param field scalarfield of real values in C-contiguous order: the value for grid point
	 (ix, iy, iz) is stored at index (iz*ny + iy)*nx + ix
	\param nx, ny, nz grid dimension
	\param outputMesh indexed mesh returned.
	*/
	void marching_cube(MC_FLOAT* field, muint nx, muint ny, muint nz, mcMesh& outputMesh)
	{
		outputMesh.vertices.reserve(defaultVerticeArraySize);
		outputMesh.normals.reserve(defaultNormalArraySize);
		outputMesh.indices.reserve(defaultTriangleArraySize);

		const mcVec3i size = { nx, ny, nz };
		mcVec3i* slab_inds = new mcVec3i[nx * ny * 2];
		MC_FLOAT vs[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
		muint edge_indices[12];
		for (muint z = 0; z < nz - 1; z++)
		{
			for (muint y = 0; y < ny - 1; y++)
			{
				for (muint x = 0; x < nx - 1; x++)
				{
					vs[0] = field[mc_internalToIndex1D(x, y, z, size)];
					vs[1] = field[mc_internalToIndex1D(x + 1, y, z, size)];
					vs[2] = field[mc_internalToIndex1D(x, y + 1, z, size)];
					vs[3] = field[mc_internalToIndex1D(x + 1, y + 1, z, size)];
					vs[4] = field[mc_internalToIndex1D(x, y, z + 1, size)];
					vs[5] = field[mc_internalToIndex1D(x + 1, y, z + 1, size)];
					vs[6] = field[mc_internalToIndex1D(x, y + 1, z + 1, size)];
					vs[7] = field[mc_internalToIndex1D(x + 1, y + 1, z + 1, size)];

					const int config_n =
						((vs[0] < 0) << 0) |
						((vs[1] < 0) << 1) |
						((vs[2] < 0) << 2) |
						((vs[3] < 0) << 3) |
						((vs[4] < 0) << 4) |
						((vs[5] < 0) << 5) |
						((vs[6] < 0) << 6) |
						((vs[7] < 0) << 7);
					if (config_n == 0 || config_n == 255)
						continue;

					if (y == 0 && z == 0)
						mc_internalComputeEdge(slab_inds, outputMesh, vs[0], vs[1], 0, x, y, z, size);
					if (z == 0)
						mc_internalComputeEdge(slab_inds, outputMesh, vs[2], vs[3], 0, x, y + 1, z, size);
					if (y == 0)
						mc_internalComputeEdge(slab_inds, outputMesh, vs[4], vs[5], 0, x, y, z + 1, size);
					mc_internalComputeEdge(slab_inds, outputMesh, vs[6], vs[7], 0, x, y + 1, z + 1, size);

					if (x == 0 && z == 0)
						mc_internalComputeEdge(slab_inds, outputMesh, vs[0], vs[2], 1, x, y, z, size);
					if (z == 0)
						mc_internalComputeEdge(slab_inds, outputMesh, vs[1], vs[3], 1,x + 1, y, z, size);
					if (x == 0)
						mc_internalComputeEdge(slab_inds, outputMesh, vs[4], vs[6], 1, x, y, z + 1, size);
					mc_internalComputeEdge(slab_inds, outputMesh, vs[5], vs[7], 1, x + 1, y, z + 1, size);

					if (x == 0 && y == 0)
						mc_internalComputeEdge(slab_inds, outputMesh, vs[0], vs[4], 2, x, y, z, size);
					if (y == 0)
						mc_internalComputeEdge(slab_inds, outputMesh, vs[1], vs[5], 2, x + 1, y, z, size);
					if (x == 0)
						mc_internalComputeEdge(slab_inds, outputMesh, vs[2], vs[6], 2, x, y + 1, z, size);
					mc_internalComputeEdge(slab_inds, outputMesh, vs[3], vs[7], 2, x + 1, y + 1, z, size);

					edge_indices[0] = slab_inds[mc_internalToIndex1DSlab(x, y, z, size)].x;
					edge_indices[1] = slab_inds[mc_internalToIndex1DSlab(x, y + 1, z, size)].x;
					edge_indices[2] = slab_inds[mc_internalToIndex1DSlab(x, y, z + 1, size)].x;
					edge_indices[3] = slab_inds[mc_internalToIndex1DSlab(x, y + 1, z + 1, size)].x;
					edge_indices[4] = slab_inds[mc_internalToIndex1DSlab(x, y, z, size)].y;
					edge_indices[5] = slab_inds[mc_internalToIndex1DSlab(x + 1, y, z, size)].y;
					edge_indices[6] = slab_inds[mc_internalToIndex1DSlab(x, y, z + 1, size)].y;
					edge_indices[7] = slab_inds[mc_internalToIndex1DSlab(x + 1, y, z + 1, size)].y;
					edge_indices[8] = slab_inds[mc_internalToIndex1DSlab(x, y, z, size)].z;
					edge_indices[9] = slab_inds[mc_internalToIndex1DSlab(x + 1, y, z, size)].z;
					edge_indices[10] = slab_inds[mc_internalToIndex1DSlab(x, y + 1, z, size)].z;
					edge_indices[11] = slab_inds[mc_internalToIndex1DSlab(x + 1, y + 1, z, size)].z;

					const uint64_t& config = mc_internalMarching_cube_tris[config_n];
					const size_t n_triangles = config & 0xF;
					const size_t n_indices = n_triangles * 3;
					const size_t& indexBase = outputMesh.indices.size();
					int offset = 4;
					for (size_t i = 0; i < n_indices; i++)
					{
						const int edge = (config >> offset) & 0xF;
						outputMesh.indices.push_back(edge_indices[edge]);
						offset += 4;
					}
					for (size_t i = 0; i < n_triangles; i++)
					{
						mc_internalAccumulateNormal(outputMesh,
							outputMesh.indices[indexBase + i * 3 + 0],
							outputMesh.indices[indexBase + i * 3 + 1],
							outputMesh.indices[indexBase + i * 3 + 2]);
					}
				}
			}
		}
		for (size_t i = 0; i < outputMesh.normals.size(); i++)
			outputMesh.normals[i] = mc_internalNormalize(outputMesh.normals[i]);
		delete[] slab_inds;
	}

#endif
}
