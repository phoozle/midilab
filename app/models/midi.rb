class Midi < ActiveRecord::Base

  def after_initialize
    unless self.new_record?
      @seq = MIDI::Sequence.new
      file = File.new("#{Rails.root}/tmp/#{self.name}", "w:ASCII-8BIT")
      data = self.data
      file.write(data)
      file.close
      file = File.new("#{Rails.root}/tmp/#{self.name}", "r:ASCII-8BIT")
      @seq.read(file)
    end
  end

  def bpm
    @seq.bpm
  end

  def tracks
    @tracks = []
    @seq.each do |t|
      track = {:name => t.name, :instrument => t.instrument}
      @tracks << track
    end
    @tracks
  end

  def tracks_to_array
    @n = []
    @seq.each {|t| t.each {|e| e.print_note_names = true; @n << e}}
    @n
  end
end
