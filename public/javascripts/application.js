// Place your application-specific JavaScript functions and classes here
// This file is automatically included by javascript_include_tag :defaults

$(document).ready(function() {
  // Expander class show/hide
  $('.expander').each(function() {
    $(this).append(' ▼');
    var element = $(this).attr('data-selector')
    $(element).hide()
    $(this).click(function() {
      $(element).slideToggle("slow");
    });
  });

  popup_status = 0
  $('[data-popup-toggle]').each(function() {
    var info_id = $(this).attr('data-information-id')
    $(this).click(function(){
      if(popup_status == 0) {
        $.getScript('/information/' + info_id + '.js')
        $('#popup_background').fadeTo('fast', 0.85)
        $('#popup_holder').fadeIn('fast');
        popup_status = 1
      }
      else {
        $('#popup_background').fadeOut('fast')
        $('#popup_holder').fadeOut('fast')
        popup_status = 0
      }
    });
  });
});
