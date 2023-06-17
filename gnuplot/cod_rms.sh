#!/bin/sh

prm1=${1-0}

gnuplot << EOP

ps = $prm1;

file_name = "cod_rms.out";

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
  set terminal pdf enhanced color solid lw l_w font "Times-Roman f_s"; \
  ext = "pdf"; \
else if (ps == 4) \
  set term pngcairo enhanced color solid lw l_w font "Times-Roman f_s"; \
  ext = "png";

set grid;

set style line 1 lt 1 lw 1 lc rgb "blue";
set style line 2 lt 1 lw 1 lc rgb "green";
set style line 3 lt 1 lw 1 lc rgb "red";

if (ps) set output "cod_rms_1.".(ext)
set title "Horizontal RMS Orbit \n \
({/Symbol D}x_{rms}=100 {/Symbol m}m, {/Symbol D}y_{rms}=100 {/Symbol m}m)";
set xlabel "s [m]"; set ylabel "[mm]";
#set yrange [-0.004:*];
set y2range [-1.5:20];
plot file_name using 1:2 axis x1y2 notitle with fsteps lt 1 lw 1 \
     lc rgb "black", \
     file_name using 1:3:3:5 notitle with errorbars ls 1;
if (!ps) pause mouse "click on graph to cont.\n";

if (ps) set output "cod_rms_2.".(ext)
set title "Vertical RMS Orbit \n \
({/Symbol D}x_{rms}=100 {/Symbol m}m, {/Symbol D}y_{rms}=100 {/Symbol m}m)";
set xlabel "s [m]"; set ylabel "[mm]";
#set yrange [-0.015:*];
set y2range [-1.5:20];
plot file_name using 1:2 axis x1y2 notitle with fsteps lt 1 lw 1 \
     lc rgb "black", \
     file_name using 1:6:6:8 notitle with errorbars ls 3;
if (!ps) pause mouse "click on graph to cont.\n";

EOP
