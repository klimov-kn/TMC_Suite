#TMC_RT_H
! =======================================================================
! horn.tpl -- излучающий планарный рупор (ќЅў≈≈ задание дл€ всех вариантов).
! ѕитающий канал (две металлические стенки) плавно раскрываетс€ в апертуру
! и излучает в открытое пространство, ограниченное рамкой-поглотителем
! ABSORBER. ‘айл открываетс€ и считаетс€ ќЅќ»ћ» €драми: tmc_rth.exe (H) и
! tmc_rtx.exe (X) -- сигнатура #TMC_RT_H одна на оба (отдельной #TMC_RT_X нет).
!
! ƒл€ своего варианта мен€ют ќƒЌќ: рабочую частоту F (строка ниже).
!  омментарии (строки с !) сохран€йте в кодировке ANSI (Windows-1251) --
! именно так их понимает препроцессор, как в рабочих файлах комплекса.
!
!  акие файлы получаютс€ (питают  ј∆ƒ”ё программу пакета):
!   horn.t              -> TMCROS   (временной сигнал на порту)
!   horn.tt, horn.ex... -> FieldView (топологи€ + поле: кнопка View>Field)
!   horn.s              -> TMCGROUT (матрица рассе€ни€, отражение от порта)
!   horn.AMP/.FAZ/.DAT  -> TMC_DN   (апертурное поле -> диаграмма направленности)
! =======================================================================

#define L_box   @ ( 240.0 )   ! длина расчЄтной коробки по X, мм
#define W_box   @ ( 200.0 )   ! ширина расчЄтной коробки по Y, мм
#define W_delta @ (   1.0 )   ! шаг сетки (размер €чейки), мм
#define W_chan  @ (  20.0 )   ! ширина питающего канала, мм
#define L_feed  @ (  60.0 )   ! длина пр€мого питающего участка, мм
#define L_flare @ (  50.0 )   ! длина раскрыва, мм
#define D_aper  @ (  80.0 )   ! ширина апертуры (усть€), мм
#define F       @ (  10.0 )   ! <<<< –јЅќ„јя „ј—“ќ“ј, √√ц -- впишите —¬ќё f
#define yc      @ ( W_box/2 )         ! ось канала (центр коробки по Y)
#define ylo     @ ( yc - W_chan/2 )   ! нижн€€ кромка канала
#define yhi     @ ( yc + W_chan/2 )   ! верхн€€ кромка канала

#STEP
#PARM
 ANGLE_UNIT radian;
 FREQ_UNIT  GHz;
 LONG_UNIT  mm;
 TIME_UNIT  ns;
 DELTA  W_delta;
 TIME   0.; 3.;
 X_MIN  0.0;
 X_MAX  L_box;
 Y_MIN  0.0;
 Y_MAX  W_box;
 FREQ   F;
#END_PARM

#TOPOLOGY
! ---- питающий канал: две пр€мые металлические стенки (толщиной в €чейку) ----
 BLOCK 1;
  RECT_STAT METAL; 0.0; L_feed; ylo-W_delta; ylo;
 END_B
 BLOCK 2;
  RECT_STAT METAL; 0.0; L_feed; yhi; yhi+W_delta;
 END_B
! ---- стенки раскрыва: наклонные металлические линии (вырожденный треугольник = лини€) ----
 BLOCK 3;
  POLYGON_STAT METAL;
   L L_feed;          ylo;
   L L_feed+L_flare;  yc-D_aper/2;
   L L_feed+L_flare;  yc-D_aper/2;
 END_B
 BLOCK 4;
  POLYGON_STAT METAL;
   L L_feed;          yhi;
   L L_feed+L_flare;  yc+D_aper/2;
   L L_feed+L_flare;  yc+D_aper/2;
 END_B
! ---- возбуждающий порт в устье канала слева ----
! INPUT_X ; y1; y2; t1; t2;   (ѕ≈–¬ќ≈ поле ѕ”—“ќ≈ -- не материал и не функци€).
!   ядро само возбуждает основной тип волны поперЄк линии порта y1..y2.
!   t1..t2 -- окно возбуждени€ по времени; t2 больше Tмакс (здесь 1000 > 3)
!   означает непрерывную накачку весь расчЄт. ќдному порту рупора амплитуда
!   не нужна (в 2-портовом волноводе VOLN.TPL 6-е поле A1/A2 включает вход).
 BLOCK 5;
  INPUT_X ; ylo; yhi; 0; 1000;
 END_B
! ---- рамка-поглотитель = открытое пространство справа / снизу / сверху ----
 BLOCK 6;
  RECT_STAT ABSORBER; L_box-5*W_delta; L_box-2*W_delta; 2*W_delta; W_box-2*W_delta;
 END_B
 BLOCK 7;
  RECT_STAT ABSORBER; 2*W_delta; L_box-2*W_delta; 2*W_delta; 5*W_delta;
 END_B
 BLOCK 8;
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
 T 8; 0.0; 0.0;
#END_LINK

#OUTPUT
 FILE horn;
 TOPOLOGY;
 FIELDS;
 SMATRIX horn; F; 1.5; 2.5;
 FIELD_DISTRIBUTION_M horn; F; 1.5; 2.5;
#END_OUTPUT
#END_STEP
#EOF
