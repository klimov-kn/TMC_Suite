#TMC_RT_H
! ==========================================================================
! Self-contained learning template for the H-mode kernel (tmc_rth.exe).
! Parallel-plate waveguide: two metal strips form a channel, a port on the
! left (INPUT_X) launches a wave, absorbers line the borders to suppress
! reflections. Small grid (200x100, delta=1mm) so the run finishes quickly.
! All comments are in English on purpose (ASCII only) to keep the file
! preprocessor-safe. Open this file in the kernel and press Run.
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
! lower metal wall of the channel
 BLOCK 1;
  RECT_STAT METAL; 0.0; L_box*0.6; W_box/2-W_chan/2-W_delta; W_box/2-W_chan/2;
 END_B
! upper metal wall of the channel
 BLOCK 2;
  RECT_STAT METAL; 0.0; L_box*0.6; W_box/2+W_chan/2; W_box/2+W_chan/2+W_delta;
 END_B
! excitation port on the left, spanning the channel mouth
 BLOCK 3;
  INPUT_X ; W_box/2-W_chan/2; W_box/2+W_chan/2; 0; 1000;
 END_B
! absorbing frame: left, right, bottom, top
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
#END_TOPOLOGY
#LINK_LIST
 T 1; 0.0; 0.0;
 T 2; 0.0; 0.0;
 T 3; 0.0; 0.0;
 T 4; 0.0; 0.0;
 T 5; 0.0; 0.0;
 T 6; 0.0; 0.0;
 T 7; 0.0; 0.0;
#END_LINK
#OUTPUT
 FILE demo_h;
 FIELD_DISTRIBUTION_M demo_h; 30.0; 7; 8;
#END_OUTPUT
#END_STEP
#EOF
