require "optparse"
require "fileutils"

options = {}
OptionParser.new do |opts|
  opts.on("-t", "--tests", "Run tests") do
    options[:tests] = true
  end

  opts.on("-c", "--clean", "Clear the build before build") do
    options[:clean] = true
  end

  opts.on("-i", "--install", "Install ASL") do
    options[:install] = true
  end

  opts.on("-g", "--debug", "Debug build") do
    options[:debug] = true
  end

  opts.on("-a", "--asan", "Address Sanizer") do
    options[:asan] = true
  end

  opts.on("-p", "--prefix PREFIX", "Sets the installation prefix") do |prefix|
    options[:prefix] = prefix
  end

  opts.on("-h", "--help", "Shows help") do
    puts opts
    exit 1
  end
end.parse!

def run(s)
  system s or abort "#{s} failed"
end

case RbConfig::CONFIG["host_os"]
  when /android/i
    options[:prefix]= ENV["PREFIX"] || ENV["HOME"] || "./"
end

build_dir = "./.build"
if not File.exist?(build_dir) or options[:clean]
  FileUtils.rm_r(build_dir) if File.exist?(build_dir)

  prefix = options[:prefix]
  setup_cmd = "meson setup #{build_dir}"
  setup_cmd << " -Dprefix=#{prefix}"
  setup_cmd << " -Dbuildtype=debug" if options[:debug]
  setup_cmd << " -Db_sanitize=address,undefined" if options[:asan]
  run setup_cmd
end

FileUtils.cp_r("#{build_dir}/compile_commands.json", ".")

run "meson compile -C #{build_dir}"

if options[:install]
  run "meson install -C #{build_dir}"
end


if options[:tests]
  run "kpp compile examples/hello_world" if options[:install]
  run ".build/kpp compile examples/hello_world" unless options[:install]
end
