variant = "MA3_-1"

build_dir = `ls -1c .. | grep build_ | head -n1`.strip
exe = "../#{build_dir}/#{variant}"

if ARGV.length < 5
	puts "this is a wrapper for #{variant}"
	puts "Usage: ruby wrapper.rb <instance_relname> <instance_specifics> <cutoff_time> <cutoff_length> <seed> <params to be passed on>."
    puts "The instance_specifics must be \"\"."
	exit 1
end
instance_filename = ARGV[0]
#instance_specifics = ARGV[1]
cutoff_time = ARGV[2].to_i
#cutoff_length = ARGV[3].to_i
seed = ARGV[4].to_i

# ParamILS passes -arg instead of --arg
paramstring = ARGV[5...ARGV.length].collect{|x| x.sub(/^-/, "--")}.join(" ")

# double the number of individuals, as they need to be even
paramstring[/--half_populationsize '?(\d+)'?/] or raise("regex error")
paramstring[/--half_populationsize '?(\d+)'?/] = "--populationsize " + (paramstring[/--half_populationsize '?(\d+)'?/, 1].to_i * 2).to_s

static_params = "-d#{cutoff_time}"

#=== Build algorithm command and execute it.
cmd = "#{exe} --seed #{seed} #{static_params} #{paramstring} #{instance_filename}"
output = `#{cmd} 2>&1`
lines = output.split("\n")
treewidth = lines[-1]
puts lines[0...-1].join("\n")

status = "SAT" # nil
runtime = cutoff_time
runlength = cutoff_time
quality = treewidth

puts "Result of algorithm run: #{status}, #{runtime}, #{runlength}, #{quality}, #{seed}"
