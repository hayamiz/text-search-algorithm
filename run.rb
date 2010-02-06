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

  opt[:trial_num] = 10
  parser.on('-t', '--trial-num NUM') do |num|
    opt[:trial_num] = num.to_i
  end

  opt[:search_num] = 1000
  parser.on('-T', '--search-num NUM') do |num|
    opt[:search_num] = num.to_i
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

  searchstr_len_set = (10..24).map{|pow| 1 << pow}
  keystr_len_set = (3..20).map{|pow| 1 << pow}
  searchstr_len = 1 << 20

  data_file = if opt[:data_file]
                File.open(opt[:data_file], "w")
              else
                Tempfile.new("text-search")
              end

  keystr_len = 32
  data_file.puts(`#{opt[:command]} -v -n 0`.lines.first) # print labels
  searchstr_len_set.each do |len|
    command = "#{opt[:command]} -t #{opt[:trial_num]} -T #{opt[:search_num]} -n #{len} -m #{keystr_len}"
    puts "Running command: #{command}"
    IO.popen(command) do |io|
      data_file.puts(io.read)
    end
    data_file.flush()
  end
  
  data_file.puts() # data delimiter
  data_file.puts() # data delimiter

  # ensure key existence
  data_file.puts(`#{opt[:command]} -v -n 0`.lines.first) # print labels
  searchstr_len_set.each do |len|
    command = "#{opt[:command]} -t #{opt[:trial_num]} -T #{opt[:search_num]} -e -n #{len} -m #{keystr_len}"
    puts "Running command: #{command}"
    IO.popen(command) do |io|
      data_file.puts(io.read)
    end
    data_file.flush()
  end
  
  data_file.puts() # data delimiter
  data_file.puts() # data delimiter

  data_file.puts(`#{opt[:command]} -v -n 0`.lines.first) # print labels
  keystr_len_set.each do |len|
    command = "#{opt[:command]} -t #{opt[:trial_num]} -T #{opt[:search_num]} -n #{searchstr_len} -m #{len}"
    puts "Running command: #{command}"
    IO.popen(command) do |io|
      data_file.puts(io.read)
    end
    data_file.flush()
  end
  
  data_file.puts() # data delimiter
  data_file.puts() # data delimiter

  # ensure key existence
  data_file.puts(`#{opt[:command]} -v -n 0`.lines.first) # print labels
  keystr_len_set.each do |len|
    command = "#{opt[:command]} -t #{opt[:trial_num]} -T #{opt[:search_num]} -e -n #{searchstr_len} -m #{len}"
    puts "Running command: #{command}"
    IO.popen(command) do |io|
      data_file.puts(io.read)
    end
    data_file.flush()
  end

  data_file.puts() # data delimiter
  data_file.puts() # data delimiter
  
  data_file.puts(`#{opt[:command]} -a check-size -v -n 0`.lines.first) # print labels
  searchstr_len_set.each do |len|
    command = "#{opt[:command]} -a check-size -t #{opt[:trial_num]} -T #{opt[:search_num]} -n #{len} -m #{keystr_len}"
    puts "Running command: #{command}"
    IO.popen(command) do |io|
      data_file.puts(io.read)
    end
    data_file.flush()
  end

  data_file.puts() # data delimiter
  data_file.puts() # data delimiter
  
  data_file.puts(`#{opt[:command]} -a check-tran -v -n 0`.lines.first) # print labels
  searchstr_len_set.each do |len|
    command = "#{opt[:command]} -a check-tran -t #{opt[:trial_num]} -T #{opt[:search_num]} -n #{len} -m #{keystr_len}"
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
