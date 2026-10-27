#TMC_RT_H
! ==========================================================================
! Ћ–1: пр€моугольный волновод с диэлектрической вставкой (H-мода, tmc_rth.exe)
! ѕример готового файла дл€ варианта 3 (волновод R40 / WR-229, a = 58.17 мм,
! диапазон 3.30..4.90 √√ц). ѕорт 1 активный, порт 2 пассивный, вставка eps=2.
! –азвЄртка по частоте: вывод пол€/топологии только на 1-м шаге (3.3 √√ц).
!
! ƒл€ своего варианта измените a, d и диапазон частот (таблица 1 задани€).
! „тобы получить ѕ”—“ќ… волновод Ч закомментируйте блок 14 и строку T 14.
! „тобы сменить активный порт Ч помен€йте местами окна/амплитуды портов 7 и 12.
! ==========================================================================
#define a   @ ( 58.17 )  ! ширина канала (ѕќ ¬ј–»јЌ“”)
#define d   @ (  1.0 )   ! шаг сетки (ѕќ ¬ј–»јЌ“”)
#define EPS @ (  2.0 )   ! проницаемость вставки: EPS-1 = Delta eps (EPS=10 -> почти полное отражение)
#define L1  @ ( a )
#define L2  @ ( 2*a/3 )
#define L3  @ ( a )
#define Lt  @ ( L1+L2+L3 )
#define W   @ ( 4*a/3 )
#define ylo @ ( (W-a)/2 )
#define yhi @ ( (W+a)/2 )
#define PI  @ ( 3.141592653589 )

! ===== STEP: FREQ = 3.3 GHz =====  (1-й шаг: с выводом пол€ и топологии)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.3;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 TOPOLOGY;
 FIELDS;
 SMATRIX wg; 3.3; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.4 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.4;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 3.4; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.5 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.5;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 3.5; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.6 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.6;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 3.6; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.7 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.7;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 3.7; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.8 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.8;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 3.8; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.9 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.9;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 3.9; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.0 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.0;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.0; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.1 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.1;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.1; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.2 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.2;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.2; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.3 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.3;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.3; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.4 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.4;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.4; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.5 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.5;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.5; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.6 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.6;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.6; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.7 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.7;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.7; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.8 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.8;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.8; 100; 110;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.9 GHz =====  (без вывода пол€/топологии Ч быстро)
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115;
 X_MIN  0.0;
 X_MAX  Lt;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.9;
#END_PARM

#TOPOLOGY
 BLOCK 14;
  RECT_STAT EPS-1; L1; L1+L2; ylo; yhi;     ! диэлектрическа€ вставка (закомментируйте дл€ пустого волновода)
 END_B
 BLOCK 1;
  RECT_STAT METAL; 0; Lt; ylo-d; ylo;       ! нижн€€ стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; Lt; yhi; yhi+d;       ! верхн€€ стенка
 END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;      ! порт 1 (ј “»¬Ќџ…)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;       ! порт 2 (пассивный: окно вне расчЄта)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 14; 0.0; 0.0;
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; Lt;  0.0;
#END_LINK

#OUTPUT
 FILE wg;
 SMATRIX wg; 4.9; 100; 110;
#END_OUTPUT
#END_STEP

#EOF
