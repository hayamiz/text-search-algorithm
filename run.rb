#!/usr/bin/env ruby

require 'optparse'
require 'tempfile'

def parse_args(argv)
  opt = Hash.new
  parser = OptionParser.new

  opt[:command] = "./text-search"
  parser.on('-c', '--command COMMAND') do |command|
    opt[:command] = command
  end

  opt[:data_file] = nil
  parser.on('-d', '--data-file FILE') do |file|
    opt[:data_file] = file
  end

  parser.parse!(argv)
  opt
end

def main(argv)
  ENV['CFLAGS'] = '-O3 -DG_DISABLE_ASSERT'
  # ENV['CFLAGS'] = '-O3'
  puts `make clean`
  puts `make`
  opt = parse_args(argv)

  data_file = if opt[:data_file]
                File.open(opt[:data_file], "w")
              else
                Tempfile.new("text-search")
              end

  searchstr_len_set = (10..24).map{|pow| 1 << pow}
  keystr_len = 32
  data_file.puts(`#{opt[:command]} -v -n 0`.split("\n").first) # print labels
  searchstr_len_set.each do |len|
    command = "#{opt[:command]} -n #{len} -m #{keystr_len}"
    puts "Running command: #{command}"
    IO.popen(command) do |io|
      data_file.puts(io.read)
    end
    data_file.flush()
  end
  
  data_file.puts() # data delimiter
  data_file.puts() # data delimiter

  # ensure key existence
  searchstr_len_set = (10..24).map{|pow| 1 << pow}
  keystr_len = 32
  data_file.puts(`#{opt[:command]} -v -n 0`.split("\n").first) # print labels
  searchstr_len_set.each do |len|
    command = "#{opt[:command]} -e -n #{len} -m #{keystr_len}"
    puts "Running command: #{command}"
    IO.popen(command) do |io|
      data_file.puts(io.read)
    end
    data_file.flush()
  end
  
  data_file.puts() # data delimiter
  data_file.puts() # data delimiter

  keystr_len_set = (3..20).map{|pow| 1 << pow}
  searchstr_len = 1 << 20
  data_file.puts(`#{opt[:command]} -v -n 0`.split("\n").first) # print labels
  keystr_len_set.each do |len|
    command = "#{opt[:command]} -n #{searchstr_len} -m #{len}"
    puts "Running command: #{command}"
    IO.popen(command) do |io|
      data_file.puts(io.read)
    end
    data_file.flush()
  end
  
  data_file.puts() # data delimiter
  data_file.puts() # data delimiter

  # ensure key existence
  keystr_len_set = (3..20).map{|pow| 1 << pow}
  searchstr_len = 1 << 20
  data_file.puts(`#{opt[:command]} -v -n 0`.split("\n").first) # print labels
  keystr_len_set.each do |len|
    command = "#{opt[:command]} -e -n #{searchstr_len} -m #{len}"
    puts "Running command: #{command}"
    IO.popen(command) do |io|
      data_file.puts(io.read)
    end
    data_file.flush()
  end
  
  data_file.fsync
  data_file.close
end

if __FILE__ == $0
  main(ARGV.dup)
end
