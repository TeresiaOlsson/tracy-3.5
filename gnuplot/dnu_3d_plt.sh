#!/bin/sh

prm1=${1-""}
prm2=${2-0}
gnuplot << EOP

home_dir = "$prm1"; ps = $prm2;

file  = (home_dir)."dnu_dA_x_delta_pert.out";

f_s = 24; l_w = 2;
if (ps == 0) \
  set terminal qt 0 font "Sans, 9"; \
else if (ps == 1) \
  set terminal postscript enhanced color solid lw l_w "Times-Roman" f_s; \
  ext = "ps"; \
else if (ps == 2) \
  set terminal postscript eps enhanced color solid lw l_w "Times-Roman" f_s; \
  ext = "eps"; \
else if (ps == 3) \
  set terminal pdf enhanced color solid linewidth l_w font "Times-Roman f_s"; \
  ext = "pdf"; \
else if (ps == 4) \
  set term pngcairo enhanced color solid lw l_w font "Times-Roman f_s"; \
  ext = "png"; \
else if (ps == 5) \
  set term svg enhanced font "Times-Roman,f_s"; \
  ext = "svg";

N = 6; N_x = 10; N_y = 3;

set grid;

set style line 1 lt 1 lw 1 lc rgb "blue";
set style line 2 lt 1 lw 1 lc rgb "green";
set style line 3 lt 1 lw 1 lc rgb "red";

set palette rgbformulae 22, 13, -31 negative;
set clabel "%3.1f"; set key right;
#unset clabel;
#unset colorbox;
#set cbrange [0:1];

#set pm3d;
set surface;
set contour base;
#set cntrparam level 75;

# x <-> horizontal, y <-> vertical, z <-> perpendicular to screen
set view 48, 51, 1, 1;
#set view 0, 0, 1, 1;

set cntrparam level 75;

if (ps) set output (home_dir)."dnu_3d_1.".(ext);

set title "{/Symbol n}_x(x, {/Symbol d})";
set xlabel "A_x [mm]"; set ylabel "{/Symbol d} [%]";
set zlabel "";

splot file using 1:(1e2*\$2):(N*(\$5+N_x)) notitle w lines lt palette z;

if (!ps) pause mouse "click on graph to cont.\n";

if (ps) set output (home_dir)."dnu_3d_2.".(ext);

set title "{/Symbol n}_y(x, {/Symbol d})";
set xlabel "A_x [mm]"; set ylabel "{/Symbol d} [%]";
set zlabel "";

splot file using 1:(1e2*\$2):(N*(\$6+N_y)) notitle w lines lt palette z;

if (!ps) pause mouse "click on graph to cont.\n";


set clabel "%3.1e"; set key right;

if (ps) set output (home_dir)."dnu_3d_3.".(ext);

set title "J_x(x, {/Symbol d}) [mm{/Symbol \327}rad]";
set xlabel "A_x [mm]"; set ylabel "{/Symbol d} [%]";
set zlabel "";

splot file using 1:(1e2*\$2):(1e6*\$3) notitle w lines lt palette z;

if (!ps) pause mouse "click on graph to cont.\n";

EOP
