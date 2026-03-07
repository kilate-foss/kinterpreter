require "fileutils"
require "timeout"

def run(cmd)
  system(cmd) or abort("Failed to run #{cmd}")
end

module Kilate
  module Std
    module Native
      PREFIX = ENV["PREFIX"] || "/usr/local"
      LIBS_DIR = "#{PREFIX}/kilate/native_libs/"

      def self.build(name, src_dir, include_dir, kilate_include_dir)
        FileUtils.mkdir_p("build")
        files = Dir.glob("#{src_dir}/**/*.c")
        args = [
          "-shared",
          "-fPIC",
          "-o build/lib#{name}.so",
          "-I#{include_dir}",
          "-I#{kilate_include_dir}",
          "#{files.join(' ')}"
        ].join(" ")
        run("gcc #{args}")
      end

      def self.install(name)
        so = "build/lib#{name}.so"
        puts "Installing #{so} in #{LIBS_DIR}..."
        FileUtils.mkdir_p(LIBS_DIR) unless Dir.exist?(LIBS_DIR)
        FileUtils.cp(so, LIBS_DIR)
        puts "#{so} successfully installed!"
      end
    end
  end
end