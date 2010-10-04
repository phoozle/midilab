class Midi < ActiveRecord::Base

  def after_initialize
    unless self.new_record?
    # Pipe data from database
    data_r, data_w = IO.pipe
    data_w.binmode
    data_w.puts self.data
    data_w.close_write

    @seq = MIDI::Sequence.new
    @seq.read(data_r)
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
