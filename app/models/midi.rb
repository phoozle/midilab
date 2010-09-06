class Midi
  PATH_TO_MIDIS = "#{Rails.root}/lib/midis"

  def self.all
    Dir.chdir PATH_TO_MIDIS
    files = Dir.glob('*.mid')
    files.map! {|f| File.new f}
  end

  def self.find(param)
    param.gsub!(/.mid/, '')
    Dir.chdir PATH_TO_MIDIS
    file = File.new "#{param}.mid"
  end
end
