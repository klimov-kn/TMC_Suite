// TmcGpuX.h - the time loop of the X-mode kernel on a video card (OpenCL 1.2, double).
//
// The same scheme as TmcGpuH.h (see there), for 6 components per node and 4
// passes per tact. On the card: the node update (RunScatteringNode..3) with the
// plasma terms, METAL / ABSORBER / MAGNETIC blocks (RunBlockNode..3) and the
// restore of the port nodes after the update (RunRestoreInputs, BUG12 portfix).
// On the CPU: ports (ExciteInputs..3 + RunSaveInputs) and all file output.
// The formulas and their order are those of the CPU code, FP_CONTRACT OFF.
//
// Switched on by the key /GPU (/GPU:n) or TMC_GPU=1 (TMC_GPU=n+1); only win64.

#ifndef _TMC_GPU_X_H
#define _TMC_GPU_X_H

#include "TmcOcl.h"
#include <algorithm>

// below this number of nodes /GPU stays on the CPU (measured, see research/gpu); TMC_GPU_MIN_NODES overrides
#ifndef TMC_GPU_X_MIN_NODES
#define TMC_GPU_X_MIN_NODES	60000	// RTX 5070 Ti, 22.10.2026: GPU wins from 40..63 thousand nodes (threshold.csv)
#endif

static const char *g_szTmcGpuXSrc =
"#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n"
"#pragma OPENCL FP_CONTRACT OFF\n"
// RunScatteringNode1line..3: w - written, s - read at the neighbours, o1 = w+1, o2 = w+2 (mod 4),
// a - auxiliary component read and rewritten (5 for passes 0, 2; 4 for 1, 3), b - the other auxiliary one
"__kernel void xpass( __global double *w, __global const double *s, __global const double *o1,\n"
"	__global const double *o2, __global double *a, __global const double *b,\n"
"	__global const double *y0, __global const double *y1, __global const double *y2,\n"
"	__global const double *y3, __global const double *y4, __global const double *y5, int nX, int nY )\n"
"{\n"
"	int x = (int)get_global_id(0) + 1;\n"
"	int y = (int)get_global_id(1) + 1;\n"
"	if( x > nX - 2 || y > nY - 2 ) return;\n"
"	long n = (long)y*nX + x;\n"
"	double r = s[n-1] + s[n+1] + s[n-nX] + s[n+nX] + y1[n]*s[n] + y2[n]*o2[n];\n"
"	w[n] = -w[n] - 2*o2[n] + y4[n]*(o1[n] + s[n])\n"
"		+ y0[n]*( r + a[n] - y4[n]*( b[n] ) + y3[n]*o2[n] - y5[n]*( s[n] + o1[n] ) );\n"
"	a[n] = r;\n"
"}\n"
"__kernel void xmetal( __global double *w, __global const long *idx, long nOff, long nNum )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	w[ idx[nOff + i] ] = 0.0;\n"
"}\n"
// RunBlockNode..3, ABSORBER and MAGNETIC: w - written component, o - the one read (w+3 mod 4)
"__kernel void xabs( __global double *w, __global const double *o, __global const double *y0,\n"
"	__global const long *idx, __global const int *typ, __global const double *ry, __global double *ru,\n"
"	long nOff, long nNum, long nX, long nArr )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	i += nOff;\n"
"	long n = idx[i];\n"
"	if( ((6*(n -  1) + 5) > 0)&&((6*(n -  1) + 5) < nArr)&&\n"
"		((6*(n + nX) + 5) > 0)&&((6*(n + nX) + 5) < nArr)&&\n"
"		((6*(n +  1) + 5) > 0)&&((6*(n +  1) + 5) < nArr)&&\n"
"		((6*(n - nX) + 5) > 0)&&((6*(n - nX) + 5) < nArr)&&\n"
"		((6*n + 5) > 0)&&((6*n + 5) < nArr) )\n"
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
"__kernel void xgather( __global const double *u0, __global const double *u1, __global const double *u2,\n"
"	__global const double *u3, __global const double *u4, __global const double *u5,\n"
"	__global const long *idx, __global double *pk, long nNum )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	long n = idx[i];\n"
"	pk[6*i] = u0[n]; pk[6*i+1] = u1[n]; pk[6*i+2] = u2[n]; pk[6*i+3] = u3[n]; pk[6*i+4] = u4[n]; pk[6*i+5] = u5[n];\n"
"}\n"
// pos - the place of the node in pk: the window around the ports (PortsUp) or the port nodes only (RunRestoreInputs)
"__kernel void xscatter( __global double *u0, __global double *u1, __global double *u2,\n"
"	__global double *u3, __global double *u4, __global double *u5,\n"
"	__global const long *idx, __global const long *pos, __global const double *pk, long nNum )\n"
"{\n"
"	long i = get_global_id(0);\n"
"	if( i >= nNum ) return;\n"
"	long n = idx[i], p = pos[i];\n"
"	u0[n] = pk[6*p]; u1[n] = pk[6*p+1]; u2[n] = pk[6*p+2]; u3[n] = pk[6*p+3]; u4[n] = pk[6*p+4]; u5[n] = pk[6*p+5];\n"
"}\n";

class CTmcGpuX
{
public:
	CTmcGpuX() : bOn( false ), ctx( NULL ), idDev( NULL ), q( NULL ), prg( NULL ), kPass( NULL ), kMetal( NULL ), kAbs( NULL ),
		kGather( NULL ), kScatter( NULL ), mBIdx( NULL ), mBTyp( NULL ), mBRy( NULL ), mBRu( NULL ),
		mPIdx( NULL ), mPPos( NULL ), mRIdx( NULL ), mRPos( NULL ), mPPk( NULL ), nX( 0 ), nY( 0 ), nN( 0 ), nPort( 0 ), nRest( 0 )
	{ for( int c = 0; c < 6; c++ ) { mU[c] = NULL; mY[c] = NULL; } }
	~CTmcGpuX() {}	// the end of the process frees the card (no OpenCL calls from static destructors)

	CString csInfo;
	CString csError;

	bool IsOn( void ) { return bOn; }

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
			TmcIdx nMin = TMC_GPU_X_MIN_NODES;
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

		std::vector<cl_long> vIdx, vWin, vPortNodes;
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
				for( TmcIdx i = 0; i < nNum; i++ )
				{
					if( ( p[i].nNodeGlobal < 0 )||( p[i].nNodeGlobal >= nN ) ) { csInfo = "CPU (a port node is outside the grid)"; return false; }
					AddWindow( vWin, p[i].nNodeGlobal );
					vPortNodes.push_back( p[i].nNodeGlobal );
				}
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
		if( !vWin.empty() ) AddWindow( vWin, 0 );
		std::sort( vWin.begin(), vWin.end() );
		vWin.erase( std::unique( vWin.begin(), vWin.end() ), vWin.end() );
		nPort = (TmcIdx)vWin.size();
		std::sort( vPortNodes.begin(), vPortNodes.end() );
		vPortNodes.erase( std::unique( vPortNodes.begin(), vPortNodes.end() ), vPortNodes.end() );
		nRest = (TmcIdx)vPortNodes.size();
		std::vector<cl_long> vWinPos( nPort ), vRestPos( nRest );
		for( TmcIdx i = 0; i < nPort; i++ ) vWinPos[i] = i;
		for( TmcIdx i = 0; i < nRest; i++ )
			vRestPos[i] = std::lower_bound( vWin.begin(), vWin.end(), vPortNodes[i] ) - vWin.begin();

		double dNeed = 96.0*nN + 20.0*vIdx.size() + 64.0*nPort;
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
			if( e == CL_SUCCESS ) prg = ocl.clCreateProgramWithSource( ctx, 1, &g_szTmcGpuXSrc, NULL, &e );
			if( e == CL_SUCCESS ) e = ocl.clBuildProgram( prg, 1, &dev.idDevice, "", NULL, NULL );
			if( e != CL_SUCCESS ) { csInfo.Format( "CPU (OpenCL start on %s failed: %s)", dev.sName.c_str(), TmcOclErr( e ) ); ReleaseAll(); return false; }
			kPass = ocl.clCreateKernel( prg, "xpass", &e );
			kMetal = ocl.clCreateKernel( prg, "xmetal", &e );
			kAbs = ocl.clCreateKernel( prg, "xabs", &e );
			kGather = ocl.clCreateKernel( prg, "xgather", &e );
			kScatter = ocl.clCreateKernel( prg, "xscatter", &e );
			idDev = dev.idDevice;
		}

		for( int c = 0; c < 6; c++ ) { mU[c] = Buf( 8*nN ); mY[c] = Buf( 8*nN ); }
		if( !vIdx.empty() )
		{
			mBIdx = Buf( 8*vIdx.size(), &vIdx[0] ); mBTyp = Buf( 4*vTyp.size(), &vTyp[0] );
			mBRy = Buf( 8*vRy.size(), &vRy[0] ); mBRu = Buf( 8*vRu.size(), &vRu[0] );
		}
		if( nPort > 0 )
		{
			mPIdx = Buf( 8*nPort, &vWin[0] ); mPPos = Buf( 8*nPort, &vWinPos[0] ); mPPk = Buf( 48*nPort );
			vPk.resize( 6*nPort ); vPortIdx = vWin;
		}
		if( nRest > 0 ) { mRIdx = Buf( 8*nRest, &vPortNodes[0] ); mRPos = Buf( 8*nRest, &vRestPos[0] ); }
		if( csError.IsEmpty() ) Upload( prU, mU );
		if( csError.IsEmpty() ) Upload( prY, mY );
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

	// RunScatteringNode (k = 0) .. RunScatteringNode3 (k = 3)
	bool Pass( int k )
	{
		int nA = ( k & 1 ) ? 4 : 5, nB = ( k & 1 ) ? 5 : 4;
		Arg( kPass, 0, mU[k] ); Arg( kPass, 1, mU[(k + 3)%4] ); Arg( kPass, 2, mU[(k + 1)%4] ); Arg( kPass, 3, mU[(k + 2)%4] );
		Arg( kPass, 4, mU[nA] ); Arg( kPass, 5, mU[nB] );
		for( int c = 0; c < 6; c++ ) Arg( kPass, 6 + c, mY[c] );
		ocl.clSetKernelArg( kPass, 12, sizeof(int), &nX ); ocl.clSetKernelArg( kPass, 13, sizeof(int), &nY );
		size_t lc[2] = { 64, 4 };
		size_t gl[2] = { ( (size_t)( nX - 2 ) + 63 )/64*64, ( (size_t)( nY - 2 ) + 3 )/4*4 };
		return Check( ocl.clEnqueueNDRangeKernel( q, kPass, 2, NULL, gl, lc, 0, NULL, NULL ), "node update" );
	}

	// RunRestoreInputs: the port nodes get back what they had after ExciteInputs
	bool Restore( void )
	{
		if( nRest == 0 ) return true;
		return Scatter( mRIdx, mRPos, nRest );
	}

	// RunBlockNode (k = 0) .. RunBlockNode3 (k = 3)
	bool Blocks( int k )
	{
		cl_mem w = mU[k], o = mU[(k + 3)%4];
		cl_long nXl = nX, nArr = 6*(cl_long)nN;
		for( size_t s = 0; s < vSeg.size(); s++ )
		{
			cl_long nOff = vSeg[s].nOff, nNum = vSeg[s].nNum;
			size_t lc = 64, gl = ( (size_t)nNum + 63 )/64*64;
			cl_int e;
			if( vSeg[s].bMetal )
			{
				Arg( kMetal, 0, w ); Arg( kMetal, 1, mBIdx );
				ocl.clSetKernelArg( kMetal, 2, 8, &nOff ); ocl.clSetKernelArg( kMetal, 3, 8, &nNum );
				e = ocl.clEnqueueNDRangeKernel( q, kMetal, 1, NULL, &gl, &lc, 0, NULL, NULL );
			}
			else
			{
				// B-8: the stub admittance is the slot 1 of prYNode, as RunBlockNode..3 on the CPU (old: mY[0])
				Arg( kAbs, 0, w ); Arg( kAbs, 1, o ); Arg( kAbs, 2, mY[1] ); Arg( kAbs, 3, mBIdx );
				Arg( kAbs, 4, mBTyp ); Arg( kAbs, 5, mBRy ); Arg( kAbs, 6, mBRu );
				ocl.clSetKernelArg( kAbs, 7, 8, &nOff ); ocl.clSetKernelArg( kAbs, 8, 8, &nNum );
				ocl.clSetKernelArg( kAbs, 9, 8, &nXl ); ocl.clSetKernelArg( kAbs, 10, 8, &nArr );
				e = ocl.clEnqueueNDRangeKernel( q, kAbs, 1, NULL, &gl, &lc, 0, NULL, NULL );
			}
			if( !Check( e, "blocks" ) ) return false;
		}
		return true;
	}

	bool PortsDown( _ELEM_VAL_RTH *prU )
	{
		if( nPort == 0 ) return true;
		cl_long n = nPort;
		for( int c = 0; c < 6; c++ ) Arg( kGather, c, mU[c] );
		Arg( kGather, 6, mPIdx ); Arg( kGather, 7, mPPk );
		ocl.clSetKernelArg( kGather, 8, 8, &n );
		size_t lc = 64, gl = ( (size_t)n + 63 )/64*64;
		if( !Check( ocl.clEnqueueNDRangeKernel( q, kGather, 1, NULL, &gl, &lc, 0, NULL, NULL ), "ports" ) ) return false;
		if( !Check( ocl.clEnqueueReadBuffer( q, mPPk, CL_TRUE, 0, 48*(size_t)n, &vPk[0], 0, NULL, NULL ), "ports read" ) ) return false;
		for( TmcIdx i = 0; i < nPort; i++ ) for( int c = 0; c < 6; c++ ) prU[ 6*vPortIdx[i] + c ] = vPk[6*i + c];
		return true;
	}

	bool PortsUp( _ELEM_VAL_RTH *prU )
	{
		if( nPort == 0 ) return true;
		for( TmcIdx i = 0; i < nPort; i++ ) for( int c = 0; c < 6; c++ ) vPk[6*i + c] = prU[ 6*vPortIdx[i] + c ];
		if( !Check( ocl.clEnqueueWriteBuffer( q, mPPk, CL_FALSE, 0, 48*(size_t)nPort, &vPk[0], 0, NULL, NULL ), "ports write" ) ) return false;
		return Scatter( mPIdx, mPPos, nPort );
	}

	bool Download( _ELEM_VAL_RTH *prU )
	{
		const TmcIdx nChunk = 2*1048576;
		std::vector<double> a( (size_t)__min( nChunk, nN ) );
		for( TmcIdx c0 = 0; c0 < nN; c0 += nChunk )
		{
			TmcIdx k, n = __min( nChunk, nN - c0 );
			for( int c = 0; c < 6; c++ )
			{
				if( !Check( ocl.clEnqueueReadBuffer( q, mU[c], CL_TRUE, 8*(size_t)c0, 8*(size_t)n, &a[0], 0, NULL, NULL ), "download" ) ) return false;
				for( k = 0; k < n; k++ ) prU[ 6*(c0 + k) + c ] = a[k];
			}
		}
		return true;
	}

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
	cl_mem mU[6], mY[6], mBIdx, mBTyp, mBRy, mBRu, mPIdx, mPPos, mRIdx, mRPos, mPPk;
	int nX, nY;
	TmcIdx nN, nPort, nRest;
	std::vector<Seg> vSeg;
	std::vector<cl_long> vPortIdx;
	std::vector<double> vPk;

	TmcIdx Total( void ) { return vSeg.empty() ? 0 : vSeg.back().nOff + vSeg.back().nNum; }

	static bool IsBadAbsorber( int t ) { return ( t == 1 )||( t == 2 )||( t == 4 )||( t == 5 )||( t == 8 )||( t == 10 ); }

	bool InBounds( TmcIdx n )
	{
		TmcIdx a = 6*nN;
		TmcIdx k[5] = { 6*(n - 1) + 5, 6*(n + nX) + 5, 6*(n + 1) + 5, 6*(n - nX) + 5, 6*n + 5 };
		for( int i = 0; i < 5; i++ ) if( ( k[i] <= 0 )||( k[i] >= a ) ) return false;
		return true;
	}

	void AddWindow( std::vector<cl_long> &v, TmcIdx n )
	{
		int x0 = (int)( n % nX ), y0 = (int)( n / nX );
		for( int y = y0 - 4; y <= y0 + 4; y++ )
			for( int x = x0 - 4; x <= x0 + 4; x++ )
				if( ( x >= 0 )&&( x < nX )&&( y >= 0 )&&( y < nY ) ) v.push_back( (cl_long)y*nX + x );
	}

	bool Scatter( cl_mem mIdx, cl_mem mPos, TmcIdx nNum )
	{
		cl_long n = nNum;
		for( int c = 0; c < 6; c++ ) Arg( kScatter, c, mU[c] );
		Arg( kScatter, 6, mIdx ); Arg( kScatter, 7, mPos ); Arg( kScatter, 8, mPPk );
		ocl.clSetKernelArg( kScatter, 9, 8, &n );
		size_t lc = 64, gl = ( (size_t)n + 63 )/64*64;
		return Check( ocl.clEnqueueNDRangeKernel( q, kScatter, 1, NULL, &gl, &lc, 0, NULL, NULL ), "ports" );
	}

	cl_mem Buf( size_t nBytes, const void *pInit = NULL )
	{
		cl_int e;
		cl_mem m = ocl.clCreateBuffer( ctx, CL_MEM_READ_WRITE | ( pInit ? CL_MEM_COPY_HOST_PTR : 0 ), nBytes ? nBytes : 8, (void *)pInit, &e );
		if( e != CL_SUCCESS ) { if( csError.IsEmpty() ) csError.Format( "buffer of %.2f GB: %s", nBytes/1073741824.0, TmcOclErr( e ) ); return NULL; }
		return m;
	}

	void Arg( cl_kernel k, cl_uint i, cl_mem m ) { ocl.clSetKernelArg( k, i, sizeof(cl_mem), &m ); }

	bool Check( cl_int e, const char *szWhat )
	{
		if( e == CL_SUCCESS ) return true;
		if( csError.IsEmpty() ) csError.Format( "GPU %s: %s", szWhat, TmcOclErr( e ) );
		return false;
	}

	void Upload( const double *pr, cl_mem *m )
	{
		const TmcIdx nChunk = 2*1048576;
		std::vector<double> a( (size_t)__min( nChunk, nN ) );
		for( TmcIdx c0 = 0; c0 < nN; c0 += nChunk )
		{
			TmcIdx k, n = __min( nChunk, nN - c0 );
			for( int c = 0; c < 6; c++ )
			{
				for( k = 0; k < n; k++ ) a[k] = pr[ 6*(c0 + k) + c ];
				if( !Check( ocl.clEnqueueWriteBuffer( q, m[c], CL_TRUE, 8*(size_t)c0, 8*(size_t)n, &a[0], 0, NULL, NULL ), "upload" ) ) return;
			}
		}
	}

	void Release( void )
	{
		cl_mem *ms[] = { &mBIdx, &mBTyp, &mBRy, &mBRu, &mPIdx, &mPPos, &mRIdx, &mRPos, &mPPk };
		if( q != NULL ) ocl.clFinish( q );
		for( int c = 0; c < 6; c++ )
		{
			if( mU[c] != NULL ) { ocl.clReleaseMemObject( mU[c] ); mU[c] = NULL; }
			if( mY[c] != NULL ) { ocl.clReleaseMemObject( mY[c] ); mY[c] = NULL; }
		}
		for( int i = 0; i < 9; i++ ) if( *ms[i] != NULL ) { ocl.clReleaseMemObject( *ms[i] ); *ms[i] = NULL; }
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
