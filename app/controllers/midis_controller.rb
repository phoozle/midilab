class MidisController < ApplicationController
  def index
    @midis = Midi.all
  end

  def show
    @midi = Midi.where(:name => params[:id]).first
  end

  def new
  end

  def create
    data = params[:file]
    name = data.original_filename

    midifile = Midi.new
    midifile.name = name.gsub('.mid', nil)
    midifile.data = data.read
    midifile.save
  end

  def destroy
  end
end
