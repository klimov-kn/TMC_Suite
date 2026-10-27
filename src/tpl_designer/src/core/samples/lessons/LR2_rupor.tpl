#TMC_RT_H
! ==========================================================================
! Ћ–2: рупор (X-мода, €дро tmc_rtx.exe).
! –упор, излучающий в открытое пространство. ѕитающий волновод шириной a
! плавно раскрываетс€ до апертуры Ap; за раскрывом Ч свободна€ область,
! ограниченна€ поглотителем ABSORBER (имитаци€ открытого пространства).
! ќдин активный порт слева. ƒл€ диаграммы направленности откройте TMC_DN.
! ==========================================================================
#define a   @ ( 30 )    ! ширина питающего волновода, мм
#define Ap  @ ( 90 )   ! ширина раскрыва (апертуры), мм
#define Lf  @ ( 40.0 )   ! длина питающего волновода, мм
#define Lr  @ ( 100.0 )  ! длина раскрыва, мм
#define Lo  @ ( 100.0 )  ! длина открытого пространства за раскрывом, мм
#define d   @ (  1.0 )   ! шаг сетки, мм
#define F   @ ( 8 )    ! рабоча€ частота, √√ц
#define ab  @ ( 20.0 )   ! толщина поглотител€ по кра€м, мм
#define Lt  @ ( Lf+Lr+Lo )
#define W   @ ( Ap+80 )
#define yc  @ ( W/2 )
#define yf1 @ ( yc-a/2 )   ! низ апертуры питани€
#define yf2 @ ( yc+a/2 )   ! верх апертуры питани€
#define ya1 @ ( yc-Ap/2 )  ! низ раскрыва
#define ya2 @ ( yc+Ap/2 )  ! верх раскрыва
#define PI  @ ( 3.141592653589 )

#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 60;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   F;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; Lf; yf1-d; yf1;     ! нижн€€ стенка питающего волновода
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lf; yf2; yf2+d;     ! верхн€€ стенка питающего волновода
 END_B
 BLOCK 3;
  POLYGON_STAT METAL;
   L Lf; yf1;
   L Lf+Lr; ya1;
   L Lf+Lr; ya1-d;
   L Lf; yf1-d;
 END_B
 BLOCK 4;
  POLYGON_STAT METAL;
   L Lf; yf2;
   L Lf+Lr; ya2;
   L Lf+Lr; ya2+d;
   L Lf; yf2+d;
 END_B
 BLOCK 8;
  RECT_STAT ABSORBER; 0; Lt; 0; ab;          ! поглотитель снизу
 END_B
 BLOCK 9;
  RECT_STAT ABSORBER; 0; Lt; W-ab; W;        ! поглотитель сверху
 END_B
 BLOCK 10;
  RECT_STAT ABSORBER; Lt-ab; Lt; 0; W;       ! поглотитель справа (открытое пространство)
 END_B
 BLOCK 7;
  INPUT_X ; yf1; yf2; 0; 60; 1; 0;    ! активный порт (питание рупора)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 3;  0.0; 0.0;
 T 4;  0.0; 0.0;
 T 8;  0.0; 0.0;
 T 9;  0.0; 0.0;
 T 10; 0.0; 0.0;
 T 7;  0.0; 0.0;
#END_LINK

#OUTPUT
 FILE horn2;
 TOPOLOGY;
 FIELDS;
 SMATRIX horn2; F; 40; 50;
 FIELD_DISTRIBUTION_M horn2; F; 40; 50;   ! интегрирует поле в пам€ти -> horn2.DAT дл€ TMC_DN (после расчЄта: Config -> Directional Pattern)
#END_OUTPUT
#END_STEP
#EOF
