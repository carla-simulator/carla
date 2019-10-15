#include "Kernel.cuh"

using namespace std;

#define gpuErrchk(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line, bool abort=true)
{
	if (code != cudaSuccess)
	{
		fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
		if (abort) exit(code);
	}
}

__global__ void processPixels (cudaSurfaceObject_t inputTexture) {
	int x = blockIdx . x * blockDim . x + threadIdx . x ;
	int y = blockIdx . y * blockDim . y + threadIdx . y ;

	uchar4 pixel = tex2D<uchar4>(inputTexture, x, y);
	pixel.x = pixel.x ^ 0xFFu;
	pixel.y = pixel.y ^ 0xFFu;
	pixel.z = pixel.z ^ 0xFFu;
	surf2Dwrite(pixel, inputTexture, x * sizeof(uchar4), y);
}
extern "C" {
	void negatif_parallel(unsigned int textureId, int width, int height) {
		cudaGraphicsResource *inputGraphicsResource;

		cudaTextureDesc inputTextureDesc;
		memset(&inputTextureDesc, 0, sizeof(inputTextureDesc));
		inputTextureDesc.addressMode[0] = cudaAddressModeClamp;
		inputTextureDesc.addressMode[1] = cudaAddressModeClamp;

		cudaGraphicsGLRegisterImage(&inputGraphicsResource, textureId, GL_TEXTURE_2D, cudaGraphicsMapFlagsReadOnly);

		cudaArray_t inputCudaArray;
		cudaGraphicsMapResources(1, &inputGraphicsResource);
		cudaGraphicsSubResourceGetMappedArray(&inputCudaArray, inputGraphicsResource, 0, 0);
		cudaResourceDesc inputResourceDesc{cudaResourceTypeArray, inputCudaArray};
		cudaTextureObject_t inputTexture;
		cudaCreateTextureObject(&inputTexture, &inputResourceDesc, &inputTextureDesc, NULL);

		const dim3 BLOCK_DIM(32, 32, 1);
		dim3 grid(width / BLOCK_DIM.x, height / BLOCK_DIM.y, 1);
		processPixels << < grid, BLOCK_DIM >> > (inputTexture);
		cudaDeviceSynchronize();

		cudaDestroyTextureObject(inputTexture);
		cudaGraphicsUnmapResources(1, &inputGraphicsResource);
	}
}
