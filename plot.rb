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

set logscale x
set logscale y
set output "suffix-searchtime-keylength-fast.eps"
plot '#{datafile}' index 2 using 2:7 ti "Suffix Automaton(random)" with lp ls 1, \
     '#{datafile}' index 2 using 2:11 ti "Suffix Array(random)" with lp ls 3, \
     '#{datafile}' index 3 using 2:11 ti "Suffix Array(existing)" with lp ls 4

set xlabel "Length of targeted strings [bytes]"
set ylabel "Index construction time [sec]"

set output "suffix-indextime-targetedlength.eps"
plot '#{datafile}' index 0 using 1:5 ti "Suffix Automaton" with lp, \
     '#{datafile}' index 0 using 1:9 ti "Suffix Array" with lp
EOS
  scriptfile = Tempfile.new("gp")
  scriptfile.puts script
  scriptfile.fsync
  system("gnuplot", scriptfile.path)
end

if __FILE__ == $0
  main(ARGV.dup)
end
