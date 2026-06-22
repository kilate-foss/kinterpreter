def run(*cmd)
  puts *cmd
  system(*cmd) or abort("Failed to run #{cmd.join(' ')}.")
end

module Experiments
  module KrayLib
    def self.build(is_termux)
      Dir.chdir "kraylib" do
        flags = ""
        flags += "-Dprefix=#{ENV['PREFIX']}" if is_termux
        run "meson setup build #{flags}" if not File.exist? "build"
        run "meson compile -C build"
      end
    end

    def self.install
      Dir.chdir "kraylib" do
        run "meson compile -C build"
      end
    end

    def self.test
      Dir.chdir "kraylib" do
        run "kilate run tests/main.klt"
      end
    end
  end
end
