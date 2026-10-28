// tmc_ocl.h - OpenCL loaded at run time (LoadLibrary), no link dependency.
//
// A machine without OpenCL.dll still starts the program: TmcOcl::Load()
// simply returns false and the caller stays on the CPU.
// Header-only; include in ONE translation unit per module with
// TMC_OCL_IMPLEMENT defined before the include.

#ifndef _TMC_OCL_H
#define _TMC_OCL_H

#define CL_TARGET_OPENCL_VERSION 120
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include <windows.h>
#include <string>
#include <vector>

#define TMC_OCL_FUNCS(X) \
	X(clGetPlatformIDs) X(clGetPlatformInfo) X(clGetDeviceIDs) X(clGetDeviceInfo) \
	X(clCreateContext) X(clReleaseContext) X(clCreateCommandQueue) X(clReleaseCommandQueue) \
	X(clCreateBuffer) X(clReleaseMemObject) X(clEnqueueReadBuffer) X(clEnqueueWriteBuffer) \
	X(clEnqueueReadBufferRect) X(clEnqueueWriteBufferRect) X(clEnqueueFillBuffer) \
	X(clCreateProgramWithSource) X(clBuildProgram) X(clGetProgramBuildInfo) X(clReleaseProgram) \
	X(clCreateKernel) X(clReleaseKernel) X(clSetKernelArg) X(clEnqueueNDRangeKernel) \
	X(clFinish) X(clFlush) X(clWaitForEvents) X(clReleaseEvent) X(clGetEventProfilingInfo)

struct TmcOcl
{
#define TMC_OCL_DECL(f) decltype(&::f) f;
	TMC_OCL_FUNCS(TMC_OCL_DECL)
#undef TMC_OCL_DECL
	HMODULE hLib;

	TmcOcl() { memset( this, 0, sizeof(*this) ); }

	bool Load()
	{
		if( hLib != NULL ) return true;
		{	// T-33: only the system OpenCL.dll (the full path), not a file from the current (task) folder
			char szDll[MAX_PATH + 32];
			UINT n = GetSystemDirectoryA( szDll, MAX_PATH );
			if( ( n == 0 )||( n >= MAX_PATH ) ) return false;
			strcat( szDll, "\\OpenCL.dll" );
			hLib = LoadLibraryA( szDll );
		}
		if( hLib == NULL ) return false;
#define TMC_OCL_GET(f) f = (decltype(&::f))GetProcAddress( hLib, #f ); if( f == NULL ) { FreeLibrary( hLib ); hLib = NULL; return false; }
		TMC_OCL_FUNCS(TMC_OCL_GET)
#undef TMC_OCL_GET
		return true;
	}
};

// one usable device (double precision supported)
struct TmcOclDevice
{
	cl_platform_id idPlatform;
	cl_device_id   idDevice;
	std::string    sName, sVendor, sDriver, sVersion, sPlatform;
	cl_ulong       nGlobalMem, nMaxAlloc;
	cl_uint        nUnits, nClockMHz;
	bool           bFP64;
};

inline std::string TmcOclDevStr( TmcOcl &ocl, cl_device_id d, cl_device_info what )
{
	size_t n = 0;
	if( ocl.clGetDeviceInfo( d, what, 0, NULL, &n ) != CL_SUCCESS || n == 0 ) return "";
	std::string s( n, '\0' );
	ocl.clGetDeviceInfo( d, what, n, &s[0], NULL );
	while( !s.empty() && ( s.back() == '\0' || s.back() == ' ' ) ) s.pop_back();
	size_t k = s.find_first_not_of( ' ' );
	return ( k == std::string::npos ) ? "" : s.substr( k );
}

// all GPU devices of all platforms, in a stable order (platform, device)
inline std::vector<TmcOclDevice> TmcOclListGpus( TmcOcl &ocl )
{
	std::vector<TmcOclDevice> v;
	cl_uint np = 0;
	if( ocl.clGetPlatformIDs( 0, NULL, &np ) != CL_SUCCESS || np == 0 ) return v;
	std::vector<cl_platform_id> pl( np );
	ocl.clGetPlatformIDs( np, &pl[0], NULL );
	for( cl_uint p = 0; p < np; p++ )
	{
		char szPl[256] = "";
		ocl.clGetPlatformInfo( pl[p], CL_PLATFORM_NAME, sizeof(szPl), szPl, NULL );
		cl_uint nd = 0;
		if( ocl.clGetDeviceIDs( pl[p], CL_DEVICE_TYPE_GPU, 0, NULL, &nd ) != CL_SUCCESS || nd == 0 ) continue;
		std::vector<cl_device_id> dv( nd );
		ocl.clGetDeviceIDs( pl[p], CL_DEVICE_TYPE_GPU, nd, &dv[0], NULL );
		for( cl_uint d = 0; d < nd; d++ )
		{
			TmcOclDevice x;
			x.idPlatform = pl[p];
			x.idDevice   = dv[d];
			x.sPlatform  = szPl;
			x.sName      = TmcOclDevStr( ocl, dv[d], CL_DEVICE_NAME );
			x.sVendor    = TmcOclDevStr( ocl, dv[d], CL_DEVICE_VENDOR );
			x.sDriver    = TmcOclDevStr( ocl, dv[d], CL_DRIVER_VERSION );
			x.sVersion   = TmcOclDevStr( ocl, dv[d], CL_DEVICE_VERSION );
			std::string sExt = TmcOclDevStr( ocl, dv[d], CL_DEVICE_EXTENSIONS );
			cl_device_fp_config fp = 0;
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_DOUBLE_FP_CONFIG, sizeof(fp), &fp, NULL );
			x.bFP64 = ( sExt.find( "cl_khr_fp64" ) != std::string::npos ) || ( fp != 0 );
			x.nGlobalMem = 0; x.nMaxAlloc = 0; x.nUnits = 0; x.nClockMHz = 0;
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &x.nGlobalMem, NULL );
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &x.nMaxAlloc, NULL );
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &x.nUnits, NULL );
			ocl.clGetDeviceInfo( dv[d], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &x.nClockMHz, NULL );
			v.push_back( x );
		}
	}
	return v;
}

inline const char *TmcOclErr( cl_int e )
{
	switch( e )
	{
		case CL_SUCCESS: return "CL_SUCCESS";
		case CL_DEVICE_NOT_FOUND: return "CL_DEVICE_NOT_FOUND";
		case CL_MEM_OBJECT_ALLOCATION_FAILURE: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
		case CL_OUT_OF_RESOURCES: return "CL_OUT_OF_RESOURCES";
		case CL_OUT_OF_HOST_MEMORY: return "CL_OUT_OF_HOST_MEMORY";
		case CL_BUILD_PROGRAM_FAILURE: return "CL_BUILD_PROGRAM_FAILURE";
		case CL_INVALID_VALUE: return "CL_INVALID_VALUE";
		case CL_INVALID_BUFFER_SIZE: return "CL_INVALID_BUFFER_SIZE";
		case CL_INVALID_KERNEL_ARGS: return "CL_INVALID_KERNEL_ARGS";
		case CL_INVALID_WORK_GROUP_SIZE: return "CL_INVALID_WORK_GROUP_SIZE";
		case CL_INVALID_WORK_ITEM_SIZE: return "CL_INVALID_WORK_ITEM_SIZE";
		case CL_INVALID_GLOBAL_WORK_SIZE: return "CL_INVALID_GLOBAL_WORK_SIZE";
		case CL_INVALID_COMMAND_QUEUE: return "CL_INVALID_COMMAND_QUEUE";
		case CL_INVALID_MEM_OBJECT: return "CL_INVALID_MEM_OBJECT";
		default: return "CL_ERROR";
	}
}

#endif
