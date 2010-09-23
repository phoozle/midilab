class MidisController < ApplicationController
  def index
    @midis = Midi.all
  end

  def show
    @midi = Midi.find(params[:id])
  end

  def new
  end

  def create
    midi = params[:file]
    name = midi.original_filename
    file = File.new("#{Rails.root}/lib/midis/#{name}", 'w')
    file.write(midi.read)
  end
end
