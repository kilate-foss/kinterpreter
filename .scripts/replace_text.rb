require "find"

if ARGV.length < 1
  puts "Usage: ruby file.rb <base_dir> <old_text> <new_text>"
  exit 1
end

$directory = ARGV[0]
old_text = ARGV[1] || ""
new_text = ARGV[2] || ""
$read_each_line = false
$read_each_line_filename = ""
use_line_break = false

ARGV.each do |arg|
  case arg
    when "-rel", "--read-each-line"
      $read_each_line = true
    when "-ulb", "--use-line-break"
      use_line_break = true
  end
end

# Check if old_text arg starts with @
# if so try read text from file
if old_text.start_with?("@")
  old_text_filename = old_text[1..]
  if File.exist?(old_text_filename)
    if $read_each_line
      $read_each_line_filename = old_text_filename
    else
      old_text = File.read(old_text_filename)
    end
  end
end

# Check if new_text arg starts with @
# if so try read text from file
if not $read_each_line
  if new_text.start_with?("@")
    new_text_filename = new_text[1..]
    if File.exist?(new_text_filename)
      new_text = File.read(new_text_filename)
    end
  end
end

$excluded_dirs = [".git", ".scripts" ]

def replace(old, new)
  Find.find($directory) do |path|
    if FileTest.directory?(path)
      if $excluded_dirs.include?(File.basename(path))
        Find.prune
      else
        next
      end
    else
      next unless File.file?(path)

      if $read_each_line && File.expand_path(path) == File.expand_path($read_each_line_filename)
        next
      end

      begin
        content = File.read(path)
        if content.include?(old)
          new_content = content.gsub(old, new)
          File.write(path, new_content)
          puts "#{old} replaced in: #{path}"
        end
      rescue => e
        puts "Failed to process #{path}: #{e.message}"
      end
    end
  end
end

if $read_each_line
  lines = File.readlines($read_each_line_filename)
  lines.each do |line|
    next if line.empty? || line.start_with?("%") || !line.include?(":")

    old, new = line.split(/\s*:\s*/, 2)
    old = old.strip unless use_line_break
    new = new.strip unless use_line_break
    replace(old, new)
  end
else
  replace(old_text, new_text)
end

puts "Replacement completed!"