require "fileutils"
require "./nativelib.rb"

install_arg = false
run_arg = false
termux_arg = false
asan_arg = false
clean_arg = false
lib_so_arg = false
nostd_arg = false
compile_commands_json_arg = false

YELLOW = "\e[33m"
LGREEN = "\e[92m"
BLUE = "\e[34m"
LMAGENTA = "\e[95m"
RESET = "\e[0m"

def run(*cmd)
  puts *cmd
  system(*cmd) or abort("Failed to run #{cmd.join(' ')}.")
end

def print_help_and_close
  puts "#{BLUE} Script to compile & install kilate#{RESET}"
  puts
  puts "Usage Make.rb <option>"
  puts "Options:"
  puts "#{LGREEN}-i    or --install               #{LMAGENTA}| Compile and install."
  puts "#{LGREEN}-r    or --run                   #{LMAGENTA}| Compile and run."
  puts "#{LGREEN}-t    or --termux                #{LMAGENTA}| [USE WITH -r] Compile and run it fixing termux restrictions."
  puts "#{LGREEN}-h    or --help                  #{LMAGENTA}| Prints help."
  puts "#{LGREEN}-as   or --asan                  #{LMAGENTA}| Enables Address Sanitizer."
  puts "#{LGREEN}-c    or --clean                 #{LMAGENTA}| Cleanup build before build again."
  puts "#{LGREEN}-lso  or --libso                 #{LMAGENTA}| Build shared libraries for Android ABIs."
  puts "#{LGREEN}-free or  -nostd                 #{LMAGENTA}| Don't build the KStandardLibrary."
  puts "#{LGREEN}-ccj  or --compile-commands-json #{LMAGENTA}| Export compile_commands.json."
  puts
  puts "#{YELLOW}WARNING"
  puts "Don't use install and run command together.#{RESET}"
  exit
end

ARGV.each do |arg|
  case arg
    when "-i", "--install"
      install_arg = true
    when "-r", "--run"
      run_arg = true
    when "-t", "--termux"
      termux_arg = true
    when "-as", "--asan"
      asan_arg = true
    when "-c", "--clean"
      clean_arg = true
    when "-lso", "--libso"
      lib_so_arg = true
    when "-free", "--nostd"
      nostd_arg = true
    when "-ccj", "--compile-commands-json"
      compile_commands_json_arg = true
    when "-h", "--help"
      print_help_and_close
    else
      print_help_and_close
  end
end

ENV["ASAN"] = "ON" if asan_arg

FileUtils.rm_rf("build") if clean_arg
FileUtils.mkdir_p("build")

# Build Std Native
Kilate::Std::Native.build(
  "knative",
  "std/native/",
  "std/native/include",
  "include"
) unless nostd_arg

if lib_so_arg
  abis = ["armeabi-v7a", "arm64-v8a", "x86", "x86_64"]
  abis.each do |abi|
    build_dir = "build/#{abi}"
    run(
      "cmake -B #{build_dir} " \
      "-DASAN=#{asan_arg ? "ON" : "OFF"} " \
      "-DINSTALL=#{install_arg ? "ON" : "OFF"} " \
      "-DCMAKE_EXPORT_COMPILE_COMMANDS=#{compile_commands_json_arg ? "ON" : "OFF"} " \
      "-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake " \
      "-DANDROID_ABI=#{abi} " \
      "-DANDROID_PLATFORM=android-26 " \
      "-DCMAKE_BUILD_TYPE=#{asan_arg ? "Debug" : "Release"}"
    )
    run("cmake --build #{build_dir}")
  end
else
  run(
    "cmake -B build " \
    "-DASAN=#{asan_arg ? "ON" : "OFF"} " \
    "-DINSTALL=#{install_arg ? "ON" : "OFF"} " \
    "-DCMAKE_EXPORT_COMPILE_COMMANDS=#{compile_commands_json_arg ? "ON" : "OFF"} " \
    "-DCMAKE_INSTALL_PREFIX=$HOME/../usr " \
    "-DCMAKE_BUILD_TYPE=#{asan_arg ? "Debug" : "Release"}"
  )
  run("cmake --build build")
end

if compile_commands_json_arg
  FileUtils.cp("build/compile_commands.json", "./")
end

if install_arg
  run("cmake --install build")
  # Install Std Native
  Kilate::Std::Native.install("knative") unless nostd_arg
end

if !install_arg && run_arg
  if termux_arg
    FileUtils.cp("kilate", ENV["HOME"])
    Dir.chdir(ENV["HOME"]) do
      FileUtils.chmod("+x", "kilate")
      exec("./kilate run main.klt")
    end
  else
    FileUtils.chmod("+x", "kilate")
    exec("./kilate run main.klt")
  end
end