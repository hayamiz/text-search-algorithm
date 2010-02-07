#!/usr/bin/env ruby

require 'optparse'
require 'tempfile'

def parse_args(argv)
  opt = Hash.new
  parser = OptionParser.new
  
  parser.parse!(argv)
  opt
end

def main(argv)
  opt = parse_args(argv)
  
  if argv.empty?
    $stderr.puts "Data file required."
    exit(1)
  end
  datafile = argv.first

  script = <<EOS
set term postscript enhanced color
set size 0.8,0.4
# set logscale x
# set logscale y
set grid
set key left top

set xlabel "Length of targeted strings [bytes]"
set ylabel "Search time [sec]"

set output "searchtime-randkey-targetedlength.eps"
plot '#{datafile}' index 0 using 1:3 ti "Poor" with lp, \
     '#{datafile}' index 0 using 1:7 ti "Suffix Automaton" with lp, \
     '#{datafile}' index 0 using 1:11 ti "Suffix Array" with lp

set output "searchtime-existingkey-targetedlength.eps"
plot '#{datafile}' index 1 using 1:3 ti "Poor" with lp, \
     '#{datafile}' index 1 using 1:7 ti "Suffix Automaton" with lp, \
     '#{datafile}' index 1 using 1:11 ti "Suffix Array" with lp

set line style 1 lt 1 pt 5 lc 1
set line style 2 lt 2 pt 6 lc 2
set line style 3 lt 1 pt 7 lc 3
set line style 4 lt 2 pt 4 lc 4

set size 1.0,0.5
# set yrange[6e-8:1e-6]
set output "suffix-searchtime-targetedlength.eps"
plot '#{datafile}' index 0 using 1:7 ti "Suffix Automaton(random)" with lp ls 1, \
     '#{datafile}' index 1 using 1:7 ti "Suffix Automaton(existing)" with lp ls 2, \
     '#{datafile}' index 0 using 1:11 ti "Suffix Array(random)" with lp ls 3, \
     '#{datafile}' index 1 using 1:11 ti "Suffix Array(existing)" with lp ls 4

set xlabel "Length of key strings [bytes]"
set output "suffix-searchtime-keylength.eps"
plot '#{datafile}' index 2 using 2:7 ti "Suffix Automaton(random)" with lp ls 1, \
     '#{datafile}' index 3 using 2:7 ti "Suffix Automaton(existing)" with lp ls 2, \
     '#{datafile}' index 2 using 2:11 ti "Suffix Array(random)" with lp ls 3, \
     '#{datafile}' index 3 using 2:11 ti "Suffix Array(existing)" with lp ls 4

set output "suffix-searchtime-keylength-fast.eps"
plot '#{datafile}' index 2 using 2:7 ti "Suffix Automaton(random)" with lp ls 1, \
     '#{datafile}' index 2 using 2:11 ti "Suffix Array(random)" with lp ls 3, \
     '#{datafile}' index 3 using 2:11 ti "Suffix Array(existing)" with lp ls 4

set xlabel "Length of targeted strings [bytes]"
set ylabel "Index construction time [sec]"
set output "suffix-indextime-targetedlength.eps"
plot '#{datafile}' index 0 using 1:5 ti "Suffix Automaton" with lp, \
     '#{datafile}' index 0 using 1:9 ti "Suffix Array" with lp

set xlabel "Length of targeted strings [bytes]"
set ylabel "Data structure size / string size"
set yrange [0:140]
set output "suffix-size.eps"
plot '#{datafile}' index 4 using 1:4 ti "Suffix Automaton(ratio)" with lp, \
     '#{datafile}' index 4 using 1:5 ti "Suffix Array(ratio)" with lp

set xlabel "Length of targeted strings [bytes]"
set ylabel "Average # of next states"
set yrange [0:3]
set output "suffix-automaton-trans.eps"
plot '#{datafile}' index 5 using 1:2 ti "Suffix Automaton" with lp

set key right top
set xrange [0:27]
set yrange [1:10000000]
set logscale y
set xlabel "# of next states"
set ylabel "Frequency"
set boxwidth 0.2
set output "suffix-automaton-tran-hist.eps"
plot '#{datafile}' ind 6 usi ($1-.25):2 ti "N=2**10" with imp lw 10 lt 1 lc 1, \
     '#{datafile}' ind 6 usi        1:3 ti "N=2**20" with imp lw 10 lt 1 lc 2, \
     '#{datafile}' ind 6 usi ($1+.25):4 ti "N=2**22" with imp lw 10 lt 1 lc 3
EOS
  scriptfile = Tempfile.new("gp")
  scriptfile.puts script
  scriptfile.fsync
  system("gnuplot", scriptfile.path)
end

if __FILE__ == $0
  main(ARGV.dup)
end
