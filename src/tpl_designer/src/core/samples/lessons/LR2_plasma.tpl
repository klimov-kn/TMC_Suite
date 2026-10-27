#TMC_RT_H
! ==========================================================================
! ЛР2: волновод с квадратиком замагниченной плазмы (X-мода, ядро tmc_rtx.exe)
! Небольшой квадрат плазмы (сторона s) в центре волновода + магнитное поле B0
! в той же области = замагниченная (гиротропная) плазма. Смотрим прохождение
! волны. Порт 1 активный, порт 2 пассивный. Один шаг на частоте F.
!
! ВАЖНО (по опыту отладки):
!  * Начинайте с режима fp < F (плазма "разрежённая", волна ПРОХОДИТ, счёт
!    устойчив). Здесь Ne=1.1e+17 даёт fp~3 ГГц при F=5 ГГц.
!  * Режим fp = F (напр. Ne=3.1e+17 при F=5) — плазменный резонанс (эффективная
!    eps -> 0), счёт на грани устойчивости; включайте его только после того, как
!    убедились, что разрежённый случай считается.
!  * Квадрат (RECT_STAT_N) надёжнее круга. Круг: CIRCLE_STAT_N Ne; 0; Rp; 0; 6.28;
!    (НЕ ставьте ровно 2*PI — из-за округления сектор может "схлопнуться").
!  * В окне статистики блок RECT_STAT_N показывает nType=0 и пустые координаты —
!    это НОРМАЛЬНО (тип eps=0; окно не умеет показывать блоки с суффиксом _N/_B).
!    Проверяйте не координаты, а строку трассировки "Set block 20; points:N" —
!    N должно быть > 0 (значит квадрат лёг на сетку).
! ==========================================================================
#define a   @ ( 60.0 )
#define d   @ (  1.0 )
#define F   @ (  5.0 )     ! частота сигнала, ГГц           (ПО ВАРИАНТУ)
#define s   @ ( 2*a/3 )     ! сторона квадратика плазмы, мм  (ПО ВАРИАНТУ)
#define Ne  @ ( 3.1e+17 )  ! концентрация электронов, 1/м^3 (fp~3 ГГц < F; знак + обязателен!)
#define B0  @ ( 0.071 )    ! магнитное поле, Тл             (fc~2 ГГц)
#define L   @ ( 4*a )
#define W   @ ( 4*a/3 )
#define ylo @ ( (W-a)/2 )
#define yhi @ ( (W+a)/2 )
#define xl  @ ( L/2-s/2 )  ! левая граница квадрата
#define xr  @ ( L/2+s/2 )  ! правая граница
#define yd  @ ( W/2-s/2 )  ! нижняя граница
#define yu  @ ( W/2+s/2 )  ! верхняя граница
#define PI  @ ( 3.141592653589 )


! ===== STEP: FREQ = 3.0 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.0;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 TOPOLOGY;
 FIELDS;
 SMATRIX plasma; 3.0; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.1 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.1;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 3.1; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.2 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.2;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 3.2; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.3 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.3;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 3.3; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.4 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.4;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 3.4; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.5 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.5;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 3.5; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.6 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.6;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 3.6; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.7 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.7;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 3.7; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.8 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.8;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 3.8; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 3.9 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   3.9;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 3.9; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.0 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.0;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.0; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.1 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.1;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.1; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.2 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.2;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.2; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.3 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.3;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.3; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.4 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.4;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.4; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.5 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.5;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.5; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.6 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.6;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.6; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.7 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.7;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.7; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.8 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.8;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.8; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 4.9 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   4.9;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 4.9; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.0 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.0;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.0; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.1 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.1;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.1; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.2 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.2;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.2; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.3 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.3;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.3; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.4 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.4;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.4; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.5 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.5;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.5; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.6 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.6;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.6; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.7 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.7;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.7; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.8 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.8;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.8; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 5.9 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   5.9;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 5.9; 100.0; 110.0;
#END_OUTPUT
#END_STEP

! ===== STEP: FREQ = 6.0 GHz =====
#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  d;
 TIME   0.; 115.0;
 X_MIN  0.0;
 X_MAX  L;
 Y_MIN  0.0;
 Y_MAX  W;
 FREQ   6.0;
#END_PARM

#TOPOLOGY
 BLOCK 1;
  RECT_STAT METAL; 0; L; ylo-d; ylo;      ! нижняя стенка
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0; L; yhi; yhi+d;      ! верхняя стенка
 END_B
 BLOCK 20;
  RECT_STAT_N Ne; xl; xr; yd; yu;         ! квадрат плазмы: концентрация электронов Ne
 END_B
! BLOCK 21;
!  RECT_STAT_B B0; xl; xr; yd; yu;         ! магнитное поле B0 в той же области
! END_B
 BLOCK 7;
  INPUT_X ; ylo; yhi; 0; 115; 1; 0;       ! активный порт (слева)
 END_B
 BLOCK 12;
  INPUT_X ; ylo; yhi; 300; 400; 0; 0;     ! пассивный порт (справа)
 END_B
#END_TOPOLOGY

#LINK_LIST
 T 1;  0.0; 0.0;
 T 2;  0.0; 0.0;
 T 20; 0.0; 0.0;
! T 21; 0.0; 0.0;
 T 7;  0.0; 0.0;
 T 12; L;   0.0;
#END_LINK

#OUTPUT
 FILE plasma;
 SMATRIX plasma; 6.0; 100.0; 110.0;
#END_OUTPUT
#END_STEP

#EOF
