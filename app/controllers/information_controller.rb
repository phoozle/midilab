class InformationController < ApplicationController
  def index
    @subjects = Subject.all
  end

  def show
    @information = Information.where(:title => params[:id]).first
    fail_message = {:title => 'Sorry :(' ,:content => 'No information available.'}
    @information_js = Information.where(:midi_class_name => params[:id]).first || fail_message

    respond_to(:html, :js)
  end
end
