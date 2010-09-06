class Midi
  PATH_TO_MIDIS = "#{Rails.root}/lib/midis"

  def self.all
    Dir.chdir PATH_TO_MIDIS
    files = Dir.glob('*.mid')
    files.map! {|f| new(f)}
  end

  def self.find(param)
    param.gsub!(/.mid/, '')
    Dir.chdir PATH_TO_MIDIS
    file = new("#{param}.mid")
  end

  def name
    File.basename(@file)
  end

  def initialize(midi)
    @file = File.new(midi)
    @seq = MIDI::Sequence.new
    @seq.read(@file)
  end

  def bpm
    @seq.bpm
  end

  def tracks
    tracks = []
    @seq.each do |t|
      track = {:name => t.name, :instrument => t.instrument}
      tracks << track
    end
  tracks
  end

  def mpq
    MIDI::Tempo.bpm_to_mpq(self.bpm)
  end

  def measures
    @seq.get_measures
  end
end