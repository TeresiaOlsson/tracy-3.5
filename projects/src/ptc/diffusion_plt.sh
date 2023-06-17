#!/bin/sh

prm1=${1-0}

gnuplot << EOP

ps = $prm1

file_name = "diffusion"

f_s = 36
l_w = 2

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
  ext = "png"

set grid

set style line 1 lw 1 lc rgb "red"
set style line 2 lw 1 lc rgb "dark-orange"
set style line 3 lw 1 lc rgb "blue"
set style line 4 lw 1 lc rgb "dark-green"
set style line 5 lw 1 lc rgb "purple"
set style line 6 lw 1 lc rgb "cyan"

palette_file_name = "`echo $TRACY_LIB`/gnuplot/jet.dat"
# Load 64-color palette for Jet
set palette model RGB file palette_file_name using (\$1/255):(\$2/255):(\$3/255)
set palette negative

set cbrange [0:1]
set noztics
unset clabel
set view map
# To set y-axis to left side and avoid compression of color box.
unset pm3d

set contour
set pm3d
set colorbox

if (ps) set output file_name.".".(ext)

set title "{/Symbol \266 f}_2/d{/Symbol \266 f}"
set xlabel "x [mm]"
set ylabel "y [mm]"
splot file_name.".out" using 1:2:(\$3*\$4) notitle lt palette z

if (!ps) pause mouse "click on graph to cont.\n"
