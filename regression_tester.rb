#!/usr/bin/env ruby
# Boomerang test functional test runner
# ARGV[0] test_executable
# ARGV[1] platform
# ARGV[2] test
# ARGV[3] test-set
# ARGV[4] options
# ARGV[5] parameters to the recompiled executable

require 'fileutils'

TESTS_DIR="./tests"
TEST_INPUT=File.join(TESTS_DIR,"inputs")

print("Regression tester 0.0.1\n")
FAILED_COMMANDLINES=""
def perform_test(exepath,test_file,output_path,args)
        log_name = output_path
        joined_args = args.join(' ')
        file_size = File.size?(test_file)
	upper_dir = output_path.split(File::SEPARATOR)[0..-2].join(File::SEPARATOR)
        cmdline = "-P #{Dir.pwd} -o #{upper_dir} #{joined_args} #{test_file}\n"
        start_t = Time.now
        result = `#{exepath} -P #{Dir.pwd} -o #{upper_dir} #{joined_args} #{test_file} >#{log_name+".stdout"} 2>#{log_name+".stderr"}`
        end_t = Time.now
        STDOUT << ($?.success?() ? '.' : '!')
        return [$?.success?,cmdline,test_file,file_size.to_f()/(end_t-start_t)]
end
if(File.exists?(File.join(TESTS_DIR,"outputs_prev")))
    FileUtils.rm_r(File.join(TESTS_DIR,"outputs_prev"),{:force=>true,:secure=>true})
end
if(File.exists?(File.join(TESTS_DIR,"outputs")))
    FileUtils.mv(File.join(TESTS_DIR,"outputs"),File.join(TESTS_DIR,"outputs_prev"), {:force => true,:verbose=>true} )
end
#exit(1)
#sh -c "./boomerang -o functest $4 test/$1/$2 2>/dev/null >/dev/null"
$crashes = {}
$times = {}

def test_all_inputs_in(base_dir,dirname="")
  STDOUT << "\nTesting in #{File.join(base_dir,"inputs",dirname)}:" if dirname!=""
  current_dir = File.join(base_dir,dirname)
  input_dir = File.join(base_dir,"inputs",dirname)
  output_dir = File.join(base_dir,"outputs",dirname)
  machine = ""
  if(dirname!="")
    machine = dirname.split(File::SEPARATOR)[1] # assumption here is that inputs are always in /inputs/<machine_name>
  end
  Dir.open(input_dir).each() {|f|
    next if f=="." or f==".."
    source = File.join(base_dir,"inputs",dirname,f)
    if File::directory?(source)
      test_all_inputs_in(base_dir,File.join(dirname,f)) # recurse
    else
      test_path = source
      result_path = File.join(base_dir,"outputs",dirname,f)
      FileUtils.mkdir_p(result_path)
      test_res = perform_test(ARGV[0],source,result_path,ARGV[1..-1])
      FileUtils.mv(File.join(output_dir,"log"),File.join(output_dir,f+".log"))
      if( not test_res[0])
        $crashes[machine] ||= []
        $crashes[machine] << [source,test_res[1]]
      else
        $times[test_res[2]] = test_res[3] if test_res[3]!=nil
      end
    end
  }
end

test_all_inputs_in(TESTS_DIR)
$crashes.each {|machine,crash_list|
        puts "\nEncountered #{crash_list.size} program failures for #{machine}"
        crash_list.each {|test|
                puts("Decompiler failed on #{test[0]} - #{test[1]}")
        }
}
sorted_times = $times.to_a.sort {|a,b| a[1] <=> b[1] }
puts "Slowest run in bytes/sec #{sorted_times.first[0]} - #{sorted_times.first[1]} bytes/sec"
#Dir.open(TESTS_DIR+"/inputs").each() {|f|
#        next if f=="." or f==".."
#        FileUtils.mv(TESTS_DIR+"/inputs/"+f,TESTS_DIR+"/outputs/"+f) if f.end_with?(".b")
#}
#puts "**************************************\n"
