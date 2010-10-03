class MidisController < ApplicationController

  def index
    @midis = Midi.all
  end

  def show
    @midi = Midi.where(:name => params[:id]).first
    
    respond_to do |format|
      format.html
      format.mid { send_data @midi.data, :type => 'audo/midi', :disposition => 'inline' }
    end
  end

  def new
  end

  def create
    data = params[:file]
    name = data.original_filename

    midifile = Midi.new
    midifile.name = name.gsub('.mid', '')
    midifile.data = data.read
    midifile.save

    redirect_to midis_path
  end

  def destroy
  end
end
