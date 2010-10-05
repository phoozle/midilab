class Midi < ActiveRecord::Base

  def after_initialize
    unless new_record?
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

  def format
    format = case @seq.format
      when 0 then '0 (single-track)'
      when 1 then '1 (multiple tracks, synchronous)'
      when 2 then '2 (multiple tracks, asynchronous)'
    end
  end

  def tracks
    @seq.tracks
  end

  def tracks_to_array
    @n = []
    @seq.each {|t| t.each {|e| e.print_decimal_numbers = false; @n << [e, e.class]}}
    @n
  end
end
