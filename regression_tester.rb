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
def perform_test(exepath,machine,test,args)
        output_path=File.join(TESTS_DIR,"outputs",machine)
        test_file = File.join(TEST_INPUT,machine,test)
        log_name = File.join(output_path,test)
        joined_args = args.join(' ')
        result = `#{exepath} -P #{Dir.pwd} -o #{output_path} #{joined_args} #{test_file} >#{log_name+".stdout"} 2>#{log_name+".stderr"}`
	cmdline = "-P #{Dir.pwd} -o #{output_path} #{joined_args} #{test_file} >#{log_name+'.stdout'} 2>#{log_name+'.stderr'}"
	STDOUT << ($?.success?() ? '.' : '!')
        return [$?.success?,cmdline]
end
if(File.exists?(File.join(TESTS_DIR,"outputs_prev")))
    FileUtils.rm_r(File.join(TESTS_DIR,"outputs_prev"),{:force=>true,:secure=>true})
end
if(File.exists?(File.join(TESTS_DIR,"outputs")))
    FileUtils.mv(File.join(TESTS_DIR,"outputs"),File.join(TESTS_DIR,"outputs_prev"), {:force => true,:verbose=>true} )
end
#exit(1)
#sh -c "./boomerang -o functest $4 test/$1/$2 2>/dev/null >/dev/null"
crashes = {}
Dir.open(TEST_INPUT).each() {|f|
        next if f=="." or f==".."
        machine_dir = File.join(TEST_INPUT,f)
        next if not File::directory?(machine_dir)
        machine = f
        Dir.open(machine_dir).each() {|test|
            next if test=="." or test==".."
            test_path = File.join(machine_dir,test)
            FileUtils.mkdir_p(File.join(TESTS_DIR,"outputs",machine,test))
	    test_res = perform_test(ARGV[0],machine,test,ARGV[1..-1])
            if( not test_res[0])
		crashes[machine] ||= []
                crashes[machine] << [test,test_res[1]]
            end
            FileUtils.mv(File.join(TESTS_DIR,"outputs",machine,"log"),File.join(TESTS_DIR,"outputs",machine,test+".log"))
        }
}
crashes.each {|machine,crash_list|
        puts "\nEncountered #{crash_list.size} program failures for #{machine}\n"        
	crash_list.each {|test|
		puts("Decompiler failed on #{machine}/#{test[0]} - #{test[1]}\n")
	}
}
#Dir.open(TESTS_DIR+"/inputs").each() {|f|
#        next if f=="." or f==".."
#        FileUtils.mv(TESTS_DIR+"/inputs/"+f,TESTS_DIR+"/outputs/"+f) if f.end_with?(".b")
#}
#puts "**************************************\n"
