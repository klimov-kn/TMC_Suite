// TmcGpuH.h - the time loop of the H-mode kernel on a video card (OpenCL 1.2, double).
//
// On the card: the node update of the whole grid (RunScatteringNode /
// RunScatteringNode1) and the METAL, ABSORBER, MAGNETIC blocks (RunBlockNode /
// RunBlockNode1). The formulas are the same, in the same order, FP_CONTRACT OFF:
// the result is bit-identical to the CPU calculation.
// On the CPU (original code): ports (ExciteInputs) and all file output. Before a
// port is excited the nodes around it are copied from the card and back after it
// (one small transfer each way). FIELDS output and the integration window of the
// directional pattern copy the whole grid from the card.
//
// Switched on by the key /GPU (/GPU:n - card number n) or TMC_GPU=1 (TMC_GPU=n+1).
// Only win64: RTX 50 cards have no 32-bit OpenCL at all.
// Arrays on the card are per component (u0, u1, y0, y1): one OpenCL buffer may
// be at most 1/4 of the card memory, so the interleaved CPU array would not fit.

#ifndef _TMC_GPU_H_H
#define _TMC_GPU_H_H

#include "TmcOcl.h"
#include <algorithm>

// below this number of nodes /GPU stays on the CPU (measured, see research/gpu); TMC_GPU_MIN_NODES overrides
#ifndef TMC_GPU_H_MIN_NODES
#define TMC_GPU_H_MIN_NODES	150000	// RTX 5070 Ti, 22.10.2026: GPU wins from 90..160 thousand nodes (threshold.csv)
#endif

static const char *g_szTmcGpuHSrc =
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"#pragma OPENCL FP_CONTRACT OFF\n"
// RunScatteringNode1line / 1line1: d - written component, s - component read at the neighbours
"__kernel void hpass( __global double *d, __global const double *s,\n"
"	__global const double *y0, __global const double *y1, int nX, int nY )\n"
"{\n"
"	int x = (int)get_global_id(0) + 1;\n"
"	int y = (int)get_global_id(1) + 1;\n"
"	if( x > nX - 2 || y > nY - 2 ) return;\n"
"	long n = (long)y*nX + x;\n"
"	d[n] = y1[n]*( s[n-1] + s[n+1] + s[n-nX] + s[n+nX] + s[n]*y0[n] ) - d[n];\n"
"}\n"
// RunBlockNode / RunBlockNode1, METAL
"__kernel void hmetal( __global double *w, __global const long *idx, long nOff, long nNum )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	w[ idx[nOff + i] ] = 0.0;\n"
"}\n"
// RunBlockNode / RunBlockNode1, ABSORBER and MAGNETIC: w - written component, o - the other one
"__kernel void habs( __global double *w, __global const double *o, __global const double *y0,\n"
"	__global const long *idx, __global const int *typ, __global const double *ry, __global double *ru,\n"
"	long nOff, long nNum, long nX, long nArr )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	i += nOff;\n"
"	long n = idx[i];\n"
"	if( ((2*(n -  1) + 1) > 0)&&((2*(n -  1) + 1) < nArr)&&\n"
"		((2*(n + nX) + 1) > 0)&&((2*(n + nX) + 1) < nArr)&&\n"
"		((2*(n +  1) + 1) > 0)&&((2*(n +  1) + 1) < nArr)&&\n"
"		((2*(n - nX) + 1) > 0)&&((2*(n - nX) + 1) < nArr)&&\n"
"		((2*n + 1) > 0)&&((2*n + 1) < nArr) )\n"
"	{\n"
"		double u1 = o[n - 1], u2 = o[n + nX], u3 = o[n + 1], u4 = o[n - nX];\n"
"		double ut1 = o[n], ut2 = ru[i], y = ry[i], y5 = y0[n];\n"
"		switch( typ[i] )\n"
"		{\n"
"		case 16: w[n] = 0.0; break;\n"
"		case 3:  y5 *= 0.75; w[n] = (2./(3+fabs(y5)+y/2))*(u3+u4+0.5*(u2+u1)+y5*ut1+y/2*ut2) - ut2; break;\n"
"		case 6:  y5 *= 0.75; w[n] = (2./(3+fabs(y5)+y/2))*(u1+u4+0.5*(u2+u3)+y5*ut1+y/2*ut2) - ut2; break;\n"
"		case 7:  y5 *= 0.5;  w[n] = (2./(2+fabs(y5)+y))*(u4+0.5*(u1+u3)+y5*ut1+y*ut2) - ut2; break;\n"
"		case 9:  y5 *= 0.75; w[n] = (2./(3+fabs(y5)+y/2))*(u3+u2+0.5*(u1+u4)+y5*ut1+y/2*ut2) - ut2; break;\n"
"		case 11: y5 *= 0.5;  w[n] = (2./(2+fabs(y5)+y))*(u3+0.5*(u2+u4)+y5*ut1+y*ut2) - ut2; break;\n"
"		case 12: y5 *= 0.75; w[n] = (2./(3+fabs(y5)+y/2))*(u1+u2+0.5*(u4+u3)+y5*ut1+y/2*ut2) - ut2; break;\n"
"		case 13: y5 *= 0.5;  w[n] = (2./(2+fabs(y5)+y))*(u2+0.5*(u1+u3)+y5*ut1+y*ut2) - ut2; break;\n"
"		case 14: y5 *= 0.5;  w[n] = (2./(2+fabs(y5)+y))*(u1+0.5*(u2+u4)+y5*ut1+y*ut2) - ut2; break;\n"
"		case 15: w[n] = 0.0; break;\n"
"		case 21: y5 *= 0.25; w[n] = (2./(1+fabs(y5)+1.5*y))*(0.5*(u1+u2)+y5*ut1+1.5*y*ut2) - ut2; break;\n"
"		case 22: y5 *= 0.25; w[n] = (2./(1+fabs(y5)+1.5*y))*(0.5*(u2+u3)+y5*ut1+1.5*y*ut2) - ut2; break;\n"
"		case 23: y5 *= 0.25; w[n] = (2./(1+fabs(y5)+1.5*y))*(0.5*(u3+u4)+y5*ut1+1.5*y*ut2) - ut2; break;\n"
"		case 24: y5 *= 0.25; w[n] = (2./(1+fabs(y5)+1.5*y))*(0.5*(u1+u4)+y5*ut1+1.5*y*ut2) - ut2; break;\n"
"		default: break;\n"
"		}\n"
"	}\n"
"	ru[i] = o[n];\n"
"}\n"
// nodes around the ports: card <-> CPU
"__kernel void hgather( __global const double *u0, __global const double *u1,\n"
"	__global const long *idx, __global double *pk, long nNum )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	long n = idx[i];\n"
"	pk[2*i] = u0[n]; pk[2*i + 1] = u1[n];\n"
"}\n"
"__kernel void hscatter( __global double *u0, __global double *u1,\n"
"	__global const long *idx, __global const double *pk, long nNum )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	long n = idx[i];\n"
"	u0[n] = pk[2*i]; u1[n] = pk[2*i + 1];\n"
"}\n";

class CTmcGpuH
{
public:
	CTmcGpuH() : bOn( false ), ctx( NULL ), idDev( NULL ), q( NULL ), prg( NULL ), kPass( NULL ), kMetal( NULL ), kAbs( NULL ),
		kGather( NULL ), kScatter( NULL ), mU0( NULL ), mU1( NULL ), mY0( NULL ), mY1( NULL ), mBIdx( NULL ),
		mBTyp( NULL ), mBRy( NULL ), mBRu( NULL ), mPIdx( NULL ), mPPk( NULL ), nX( 0 ), nY( 0 ), nN( 0 ), nPort( 0 ) {}
	~CTmcGpuH() {}	// the end of the process frees the card (no OpenCL calls from static destructors)

	CString csInfo;		// "GPU 0: <name> ..." or why the CPU is used
	CString csError;	// error during the loop

	bool IsOn( void ) { return bOn; }

	// /GPU, /GPU:n (a separate word, not inside "quotes") or TMC_GPU=1..n+1
	// T-13: /CPU (a separate word, not inside "quotes") - the card is not used, whatever /GPU or TMC_GPU say
	static bool IsCpuKey( void )
	{
		const char *c = GetCommandLineA();
		bool bQuote = false;
		for( int i = 0; c[i] != '\0'; i++ )
		{
			if( c[i] == '"' ) { bQuote = !bQuote; continue; }
			if( bQuote ) continue;
			if( ( c[i] != '/' )&&( c[i] != '-' ) ) continue;
			if( ( i > 0 )&&( c[i-1] != ' ' )&&( c[i-1] != '\t' ) ) continue;
			if( _strnicmp( c + i + 1, "CPU", 3 ) != 0 ) continue;
			char e = c[i+4];
			if( ( e == '\0' )||( e == ' ' )||( e == '\t' ) ) return true;
		}
		return false;
	}

	static bool IsRequested( int *pnDev )
	{
		*pnDev = 0;
		if( IsCpuKey() ) return false;	// T-13: /CPU is stronger than /GPU
		const char *psz = getenv( "TMC_GPU" );
		if( ( psz != NULL )&&( atoi( psz ) > 0 ) ) { *pnDev = atoi( psz ) - 1; return true; }
		const char *c = GetCommandLineA();
		bool bQuote = false;
		for( int i = 0; c[i] != '\0'; i++ )
		{
			if( c[i] == '"' ) { bQuote = !bQuote; continue; }
			if( bQuote ) continue;
			if( ( c[i] != '/' )&&( c[i] != '-' ) ) continue;
			if( ( i > 0 )&&( c[i-1] != ' ' )&&( c[i-1] != '\t' ) ) continue;
			if( _strnicmp( c + i + 1, "GPU", 3 ) != 0 ) continue;
			char e = c[i+4];
			if( ( e == '\0' )||( e == ' ' )||( e == '\t' ) ) return true;
			if( ( e == ':' )||( e == '=' ) ) { *pnDev = atoi( c + i + 5 ); return true; }
		}
		return false;
	}

	// upload the step to the card; false - the loop stays on the CPU (csInfo says why)
	bool Begin( _ELEM_VAL_RTH *prU, _real *prY, int nX1, int nY1, CTmcRTHNodeDiel *pDiel, int nDiel )
	{
		Release();
		csError = "";
		int nDev = 0;
		if( !IsRequested( &nDev ) ) { csInfo = IsCpuKey() ? "CPU (key /CPU)" : "CPU"; return false; }	// T-13
#ifndef _WIN64
		csInfo = "CPU (GPU needs the win64 build)";
		return false;
#else
		nX = nX1; nY = nY1; nN = (TmcIdx)nX*nY;
		// small grids: the card waits for the ports every pass and loses to the CPU
		{
			TmcIdx nMin = TMC_GPU_H_MIN_NODES;
			const char *pszMin = getenv( "TMC_GPU_MIN_NODES" );
			if( pszMin != NULL ) nMin = (TmcIdx)_atoi64( pszMin );
			if( nN < nMin )
			{
				csInfo.Format( "CPU (grid of %I64d nodes is below the GPU threshold %I64d; TMC_GPU_MIN_NODES=0 forces the card)", (__int64)nN, (__int64)nMin );
				return false;
			}
		}
		if( !ocl.Load() ) { csInfo = "CPU (GPU requested, but OpenCL.dll was not found)"; return false; }
		std::vector<TmcOclDevice> v = TmcOclListGpus( ocl ), vd;
		for( size_t k = 0; k < v.size(); k++ ) if( v[k].bFP64 ) vd.push_back( v[k] );
		if( vd.empty() ) { csInfo = "CPU (GPU requested, but no card with double precision)"; return false; }
		if( ( nDev < 0 )||( nDev >= (int)vd.size() ) )	// T-12: not the card 0 silently
		{
			csInfo.Format( "CPU (GPU card %d not found: %d card(s) with double precision, numbered from 0)", nDev, (int)vd.size() );
			return false;
		}
		dev = vd[nDev];

		// blocks run on the card; anything the CPU code would stop on stays on the CPU
		std::vector<cl_long> vIdx, vPort;
		std::vector<cl_int> vTyp;
		std::vector<double> vRy, vRu;
		vSeg.clear();
		for( int b = 0; b < nDiel; b++ )
		{
			int nT; TmcIdx nNum; sTmcRTHNodeDielOne *p;
			pDiel[b].GpuView( nT, nNum, p );
			if( nT == CTMCRTH_BLCKNTYPE_EPS ) continue;
			if( p == NULL ) { csInfo = "CPU (a block has no nodes)"; return false; }
			if( ( nT == CTMCRTH_BLCKNTYPE_INPXLEFT )||( nT == CTMCRTH_BLCKNTYPE_INPXRIGHT )||
				( nT == CTMCRTH_BLCKNTYPE_INPYTOP )||( nT == CTMCRTH_BLCKNTYPE_INPYBOT ) )
			{
				for( TmcIdx i = 0; i < nNum; i++ ) AddWindow( vPort, p[i].nNodeGlobal );
				continue;
			}
			if( ( nT != CTMCRTH_BLCKNTYPE_METAL )&&( nT != CTMCRTH_BLCKNTYPE_ABSORBER )&&( nT != CTMCRTH_BLCKNTYPE_MAGNETIC ) ) continue;
			if( nNum <= 0 ) continue;
			Seg s;
			s.nBlock = b; s.bMetal = ( nT == CTMCRTH_BLCKNTYPE_METAL ); s.nOff = (TmcIdx)vIdx.size(); s.nNum = nNum;
			std::vector< std::pair<cl_long, TmcIdx> > vChk;
			for( TmcIdx i = 0; i < nNum; i++ )
			{
				TmcIdx n = p[i].nNodeGlobal;
				if( ( n < 0 )||( n >= nN ) ) { csInfo = "CPU (a block node is outside the grid)"; return false; }
				vIdx.push_back( n ); vTyp.push_back( p[i].nType ); vRy.push_back( p[i].rY ); vRu.push_back( p[i].rU );
				vChk.push_back( std::make_pair( (cl_long)n, i ) );
				if( !s.bMetal && IsBadAbsorber( p[i].nType ) && InBounds( n ) )
				{ csInfo = "CPU (bad absorber node type - the CPU reports it)"; return false; }
			}
			// a node twice in a block: every copy reads the same values (the other component),
			// the CPU keeps what the LAST copy wrote; earlier copies only update their rU
			std::sort( vChk.begin(), vChk.end() );
			for( size_t k = 0; k + 1 < vChk.size(); k++ )
				if( vChk[k].first == vChk[k + 1].first ) vTyp[ s.nOff + vChk[k].second ] = -1;
			vSeg.push_back( s );
		}
		if( !vPort.empty() ) AddWindow( vPort, 0 );	// ExciteInputs reads node 0 at the ends of a port
		std::sort( vPort.begin(), vPort.end() );
		vPort.erase( std::unique( vPort.begin(), vPort.end() ), vPort.end() );
		nPort = (TmcIdx)vPort.size();

		// memory
		double dNeed = 32.0*nN + 20.0*vIdx.size() + 24.0*nPort;
		if( 8.0*nN > (double)dev.nMaxAlloc || dNeed > 0.92*dev.nGlobalMem )
		{
			csInfo.Format( "CPU (the grid needs %.2f GB, card %s has %.2f GB)", dNeed/1073741824.0, dev.sName.c_str(), dev.nGlobalMem/1073741824.0 );
			return false;
		}

		cl_int e = CL_SUCCESS;
		if( ( ctx == NULL )||( idDev != dev.idDevice ) )	// the context and the compiled kernels live between the steps
		{
			ReleaseAll();
			cl_context_properties pr[] = { CL_CONTEXT_PLATFORM, (cl_context_properties)dev.idPlatform, 0 };
			ctx = ocl.clCreateContext( pr, 1, &dev.idDevice, NULL, NULL, &e );
			if( e == CL_SUCCESS ) q = ocl.clCreateCommandQueue( ctx, dev.idDevice, 0, &e );
			if( e == CL_SUCCESS ) prg = ocl.clCreateProgramWithSource( ctx, 1, &g_szTmcGpuHSrc, NULL, &e );
			if( e == CL_SUCCESS ) e = ocl.clBuildProgram( prg, 1, &dev.idDevice, "", NULL, NULL );
			if( e != CL_SUCCESS ) { csInfo.Format( "CPU (OpenCL start on %s failed: %s)", dev.sName.c_str(), TmcOclErr( e ) ); ReleaseAll(); return false; }
			kPass = ocl.clCreateKernel( prg, "hpass", &e );
			kMetal = ocl.clCreateKernel( prg, "hmetal", &e );
			kAbs = ocl.clCreateKernel( prg, "habs", &e );
			kGather = ocl.clCreateKernel( prg, "hgather", &e );
			kScatter = ocl.clCreateKernel( prg, "hscatter", &e );
			idDev = dev.idDevice;
		}

		mU0 = Buf( 8*nN ); mU1 = Buf( 8*nN ); mY0 = Buf( 8*nN ); mY1 = Buf( 8*nN );
		if( !vIdx.empty() )
		{
			mBIdx = Buf( 8*vIdx.size(), &vIdx[0] ); mBTyp = Buf( 4*vTyp.size(), &vTyp[0] );
			mBRy = Buf( 8*vRy.size(), &vRy[0] ); mBRu = Buf( 8*vRu.size(), &vRu[0] );
		}
		if( nPort > 0 ) { mPIdx = Buf( 8*nPort, &vPort[0] ); mPPk = Buf( 16*nPort ); vPk.resize( 2*nPort ); vPortIdx = vPort; }
		if( csError.IsEmpty() ) Upload( prU, mU0, mU1 );
		if( csError.IsEmpty() ) Upload( prY, mY0, mY1 );
		if( csError.IsEmpty() ) { e = ocl.clFinish( q ); if( e != CL_SUCCESS ) csError.Format( "upload: %s", TmcOclErr( e ) ); }
		if( !csError.IsEmpty() )
		{
			csInfo.Format( "CPU (memory on %s: %s)", dev.sName.c_str(), (LPCTSTR)csError );
			csError = "";
			Release();
			return false;
		}
		csInfo.Format( "GPU %d: %s (%s, driver %s, %.1f GB)", nDev, dev.sName.c_str(), dev.sVersion.c_str(), dev.sDriver.c_str(), dev.nGlobalMem/1073741824.0 );
		bOn = true;
		return true;
#endif
	}

	// grid and absorber states back into the CPU arrays, card released
	void End( _ELEM_VAL_RTH *prU, CTmcRTHNodeDiel *pDiel )
	{
		if( !bOn ) return;
		if( csError.IsEmpty() ) Download( prU );
		if( csError.IsEmpty() && ( mBRu != NULL ) )
		{
			std::vector<double> vRu( Total() );
			cl_int e = ocl.clEnqueueReadBuffer( q, mBRu, CL_TRUE, 0, 8*vRu.size(), &vRu[0], 0, NULL, NULL );
			if( e == CL_SUCCESS )
				for( size_t k = 0; k < vSeg.size(); k++ )
				{
					int nT; TmcIdx nNum; sTmcRTHNodeDielOne *p;
					pDiel[ vSeg[k].nBlock ].GpuView( nT, nNum, p );
					if( !vSeg[k].bMetal ) for( TmcIdx i = 0; i < nNum; i++ ) p[i].rU = vRu[ vSeg[k].nOff + i ];
				}
			else csError.Format( "read of absorber states: %s", TmcOclErr( e ) );
		}
		Release();
	}

	// node update of the grid: nHalf 0 - RunScatteringNode, 1 - RunScatteringNode1
	bool Pass( int nHalf )
	{
		cl_mem d = nHalf ? mU1 : mU0, s = nHalf ? mU0 : mU1;
		Arg( kPass, 0, d ); Arg( kPass, 1, s ); Arg( kPass, 2, mY0 ); Arg( kPass, 3, mY1 );
		ocl.clSetKernelArg( kPass, 4, sizeof(int), &nX ); ocl.clSetKernelArg( kPass, 5, sizeof(int), &nY );
		size_t lc[2] = { 64, 4 };
		size_t gl[2] = { ( (size_t)( nX - 2 ) + 63 )/64*64, ( (size_t)( nY - 2 ) + 3 )/4*4 };
		return Check( ocl.clEnqueueNDRangeKernel( q, kPass, 2, NULL, gl, lc, 0, NULL, NULL ), "node update" );
	}

	// METAL / ABSORBER / MAGNETIC blocks in the CPU order: nHalf 0 - RunBlockNode, 1 - RunBlockNode1
	bool Blocks( int nHalf )
	{
		cl_mem w = nHalf ? mU1 : mU0, o = nHalf ? mU0 : mU1;
		cl_long nXl = nX, nArr = 2*(cl_long)nN;
		for( size_t k = 0; k < vSeg.size(); k++ )
		{
			cl_long nOff = vSeg[k].nOff, nNum = vSeg[k].nNum;
			size_t lc = 64, gl = ( (size_t)nNum + 63 )/64*64;
			cl_int e;
			if( vSeg[k].bMetal )
			{
				Arg( kMetal, 0, w ); Arg( kMetal, 1, mBIdx );
				ocl.clSetKernelArg( kMetal, 2, 8, &nOff ); ocl.clSetKernelArg( kMetal, 3, 8, &nNum );
				e = ocl.clEnqueueNDRangeKernel( q, kMetal, 1, NULL, &gl, &lc, 0, NULL, NULL );
			}
			else
			{
				Arg( kAbs, 0, w ); Arg( kAbs, 1, o ); Arg( kAbs, 2, mY0 ); Arg( kAbs, 3, mBIdx );
				Arg( kAbs, 4, mBTyp ); Arg( kAbs, 5, mBRy ); Arg( kAbs, 6, mBRu );
				ocl.clSetKernelArg( kAbs, 7, 8, &nOff ); ocl.clSetKernelArg( kAbs, 8, 8, &nNum );
				ocl.clSetKernelArg( kAbs, 9, 8, &nXl ); ocl.clSetKernelArg( kAbs, 10, 8, &nArr );
				e = ocl.clEnqueueNDRangeKernel( q, kAbs, 1, NULL, &gl, &lc, 0, NULL, NULL );
			}
			if( !Check( e, "blocks" ) ) return false;
		}
		return true;
	}

	// nodes around the ports: card -> CPU array (before ExciteInputs)
	bool PortsDown( _ELEM_VAL_RTH *prU )
	{
		if( nPort == 0 ) return true;
		cl_long n = nPort;
		Arg( kGather, 0, mU0 ); Arg( kGather, 1, mU1 ); Arg( kGather, 2, mPIdx ); Arg( kGather, 3, mPPk );
		ocl.clSetKernelArg( kGather, 4, 8, &n );
		size_t lc = 64, gl = ( (size_t)n + 63 )/64*64;
		if( !Check( ocl.clEnqueueNDRangeKernel( q, kGather, 1, NULL, &gl, &lc, 0, NULL, NULL ), "ports" ) ) return false;
		if( !Check( ocl.clEnqueueReadBuffer( q, mPPk, CL_TRUE, 0, 16*(size_t)n, &vPk[0], 0, NULL, NULL ), "ports read" ) ) return false;
		for( TmcIdx i = 0; i < nPort; i++ ) { prU[ 2*vPortIdx[i] ] = vPk[2*i]; prU[ 2*vPortIdx[i] + 1 ] = vPk[2*i + 1]; }
		return true;
	}

	// ... and back to the card (after ExciteInputs)
	bool PortsUp( _ELEM_VAL_RTH *prU )
	{
		if( nPort == 0 ) return true;
		for( TmcIdx i = 0; i < nPort; i++ ) { vPk[2*i] = prU[ 2*vPortIdx[i] ]; vPk[2*i + 1] = prU[ 2*vPortIdx[i] + 1 ]; }
		cl_long n = nPort;
		if( !Check( ocl.clEnqueueWriteBuffer( q, mPPk, CL_FALSE, 0, 16*(size_t)n, &vPk[0], 0, NULL, NULL ), "ports write" ) ) return false;
		Arg( kScatter, 0, mU0 ); Arg( kScatter, 1, mU1 ); Arg( kScatter, 2, mPIdx ); Arg( kScatter, 3, mPPk );
		ocl.clSetKernelArg( kScatter, 4, 8, &n );
		size_t lc = 64, gl = ( (size_t)n + 63 )/64*64;
		return Check( ocl.clEnqueueNDRangeKernel( q, kScatter, 1, NULL, &gl, &lc, 0, NULL, NULL ), "ports" );
	}

	// whole grid card -> CPU array (FIELDS, directional pattern window)
	bool Download( _ELEM_VAL_RTH *prU )
	{
		const TmcIdx nChunk = 4*1048576;
		std::vector<double> a( (size_t)__min( nChunk, nN ) ), b( a.size() );
		for( TmcIdx c = 0; c < nN; c += nChunk )
		{
			TmcIdx k, n = __min( nChunk, nN - c );
			if( !Check( ocl.clEnqueueReadBuffer( q, mU0, CL_FALSE, 8*(size_t)c, 8*(size_t)n, &a[0], 0, NULL, NULL ), "download" ) ) return false;
			if( !Check( ocl.clEnqueueReadBuffer( q, mU1, CL_TRUE, 8*(size_t)c, 8*(size_t)n, &b[0], 0, NULL, NULL ), "download" ) ) return false;
			for( k = 0; k < n; k++ ) { prU[ 2*(c + k) ] = a[k]; prU[ 2*(c + k) + 1 ] = b[k]; }
		}
		return true;
	}

	CString DeviceName( void ) { return CString( dev.sName.c_str() ); }

private:
	struct Seg { int nBlock; bool bMetal; TmcIdx nOff, nNum; };

	TmcOcl ocl;
	TmcOclDevice dev;
	bool bOn;
	cl_context ctx;
	cl_device_id idDev;	// the device ctx was made for
	cl_command_queue q;
	cl_program prg;
	cl_kernel kPass, kMetal, kAbs, kGather, kScatter;
	cl_mem mU0, mU1, mY0, mY1, mBIdx, mBTyp, mBRy, mBRu, mPIdx, mPPk;
	int nX, nY;
	TmcIdx nN, nPort;
	std::vector<Seg> vSeg;
	std::vector<cl_long> vPortIdx;
	std::vector<double> vPk;

	TmcIdx Total( void ) { return vSeg.empty() ? 0 : vSeg.back().nOff + vSeg.back().nNum; }

	static bool IsBadAbsorber( int t ) { return ( t == 1 )||( t == 2 )||( t == 4 )||( t == 5 )||( t == 8 )||( t == 10 ); }

	bool InBounds( TmcIdx n )
	{
		TmcIdx a = 2*nN;
		TmcIdx k[5] = { 2*(n - 1) + 1, 2*(n + nX) + 1, 2*(n + 1) + 1, 2*(n - nX) + 1, 2*n + 1 };
		for( int i = 0; i < 5; i++ ) if( ( k[i] <= 0 )||( k[i] >= a ) ) return false;
		return true;
	}

	// a port node and every node within 4 nodes of it (what ExciteInputs may touch)
	void AddWindow( std::vector<cl_long> &v, TmcIdx n )
	{
		int x0 = (int)( n % nX ), y0 = (int)( n / nX );
		for( int y = y0 - 4; y <= y0 + 4; y++ )
			for( int x = x0 - 4; x <= x0 + 4; x++ )
				if( ( x >= 0 )&&( x < nX )&&( y >= 0 )&&( y < nY ) ) v.push_back( (cl_long)y*nX + x );
	}

	cl_mem Buf( size_t nBytes, const void *pInit = NULL )
	{
		cl_int e;
		cl_mem m = ocl.clCreateBuffer( ctx, CL_MEM_READ_WRITE | ( pInit ? CL_MEM_COPY_HOST_PTR : 0 ), nBytes ? nBytes : 8, (void *)pInit, &e );
		if( e != CL_SUCCESS ) { csError.Format( "buffer of %.2f GB: %s", nBytes/1073741824.0, TmcOclErr( e ) ); return NULL; }
		return m;
	}

	void Arg( cl_kernel k, cl_uint i, cl_mem m ) { ocl.clSetKernelArg( k, i, sizeof(cl_mem), &m ); }

	bool Check( cl_int e, const char *szWhat )
	{
		if( e == CL_SUCCESS ) return true;
		if( csError.IsEmpty() ) csError.Format( "GPU %s: %s", szWhat, TmcOclErr( e ) );
		return false;
	}

	void Upload( const double *pr, cl_mem m0, cl_mem m1 )
	{
		const TmcIdx nChunk = 4*1048576;
		std::vector<double> a( (size_t)__min( nChunk, nN ) ), b( a.size() );
		for( TmcIdx c = 0; c < nN; c += nChunk )
		{
			TmcIdx k, n = __min( nChunk, nN - c );
			for( k = 0; k < n; k++ ) { a[k] = pr[ 2*(c + k) ]; b[k] = pr[ 2*(c + k) + 1 ]; }
			if( !Check( ocl.clEnqueueWriteBuffer( q, m0, CL_TRUE, 8*(size_t)c, 8*(size_t)n, &a[0], 0, NULL, NULL ), "upload" ) ) return;
			if( !Check( ocl.clEnqueueWriteBuffer( q, m1, CL_TRUE, 8*(size_t)c, 8*(size_t)n, &b[0], 0, NULL, NULL ), "upload" ) ) return;
		}
	}

	void Release( void )
	{
		cl_mem *ms[] = { &mU0, &mU1, &mY0, &mY1, &mBIdx, &mBTyp, &mBRy, &mBRu, &mPIdx, &mPPk };
		if( q != NULL ) ocl.clFinish( q );
		for( int i = 0; i < 10; i++ ) if( *ms[i] != NULL ) { ocl.clReleaseMemObject( *ms[i] ); *ms[i] = NULL; }
		bOn = false;	// nPort/nRest are set by Begin (ReleaseAll inside Begin must not clear them)
	}

	void ReleaseAll( void )
	{
		cl_kernel *ks[] = { &kPass, &kMetal, &kAbs, &kGather, &kScatter };
		Release();
		for( int i = 0; i < 5; i++ ) if( *ks[i] != NULL ) { ocl.clReleaseKernel( *ks[i] ); *ks[i] = NULL; }
		if( prg != NULL ) { ocl.clReleaseProgram( prg ); prg = NULL; }
		if( q != NULL ) { ocl.clReleaseCommandQueue( q ); q = NULL; }
		if( ctx != NULL ) { ocl.clReleaseContext( ctx ); ctx = NULL; }
		idDev = NULL;
	}
};

#endif
