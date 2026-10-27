#TMC_RT_H
! ==========================================================================
! Self-contained learning template for the X-mode kernel (tmc_rtx.exe).
! The .tpl format and the #TMC_RT_H signature are identical to the H kernel
! (there is no separate #TMC_RT_X signature), so this same file also opens
! in the H kernel. Geometry: a parallel-plate channel with a left port,
! framed by absorbers. Small grid for a quick run.
!
! X-mode specific features (moving blocks RECT_MOVE / CIRCLE_MOVE /
! POLYGON_MOVE, the second port INPUT_Y, and dielectric distributions given
! as an expression E(x,y)) are shown below as COMMENTED examples - remove the
! leading '!' on a block to experiment. They are left disabled so the file is
! guaranteed to compute as-is.
! ==========================================================================

#define L_box   @ ( 200.0 )
#define W_box   @ ( 100.0 )
#define W_delta @ (   1.0 )
#define W_chan  @ (  20.0 )

#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT GHz;
 LONG_UNIT mm;
 TIME_UNIT ns;
 DELTA W_delta;
 TIME 0.; 3.;
 X_MIN 0.0;
 X_MAX L_box;
 Y_MIN 0.0;
 Y_MAX W_box;
 FREQ 30.0;
#END_PARM
#TOPOLOGY
! lower / upper metal walls of the channel
 BLOCK 1;
  RECT_STAT METAL; 0.0; L_box*0.6; W_box/2-W_chan/2-W_delta; W_box/2-W_chan/2;
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0.0; L_box*0.6; W_box/2+W_chan/2; W_box/2+W_chan/2+W_delta;
 END_B
! excitation port along the X axis (launches the wave to the right)
 BLOCK 3;
  INPUT_X ; W_box/2-W_chan/2; W_box/2+W_chan/2; 0; 1000;
 END_B
! absorbing frame
 BLOCK 4;
  RECT_STAT ABSORBER; 2*W_delta; 5*W_delta; 2*W_delta; W_box-2*W_delta;
 END_B
 BLOCK 5;
  RECT_STAT ABSORBER; L_box-5*W_delta; L_box-2*W_delta; 2*W_delta; W_box-2*W_delta;
 END_B
 BLOCK 6;
  RECT_STAT ABSORBER; 2*W_delta; L_box-2*W_delta; 2*W_delta; 5*W_delta;
 END_B
 BLOCK 7;
  RECT_STAT ABSORBER; 2*W_delta; L_box-2*W_delta; W_box-5*W_delta; W_box-2*W_delta;
 END_B
!
! --- OPTIONAL X-mode demos (disabled). Uncomment a whole block to try. ---
! Moving metal obstacle (Vx;Vy;w at the end): a scatterer drifting in +X.
! BLOCK 8;
!  RECT_MOVE METAL; 90.0; 95.0; 45.0; 55.0; 5.0; 0.0; 0.0;
! END_B
! Static dielectric lens given as a distribution E(x,y) instead of a material:
! BLOCK 9;
!  RECT_STAT - ( ((x-120)^2+(y-50)^2)/400 ) ; 100.0; 140.0; 35.0; 65.0;
! END_B
! Second port along the Y axis (excitation U(x) across an x-range):
! BLOCK 10;
!  INPUT_Y ; 90.0; 110.0; 0; 1000;
! END_B
#END_TOPOLOGY
#LINK_LIST
 T 1; 0.0; 0.0;
 T 2; 0.0; 0.0;
 T 3; 0.0; 0.0;
 T 4; 0.0; 0.0;
 T 5; 0.0; 0.0;
 T 6; 0.0; 0.0;
 T 7; 0.0; 0.0;
! T 8; 0.0; 0.0;
! T 9; 0.0; 0.0;
! T 10; 0.0; 0.0;
#END_LINK
#OUTPUT
 FILE demo_x;
 FIELD_DISTRIBUTION_M demo_x; 30.0; 7; 8;
#END_OUTPUT
#END_STEP
#EOF
