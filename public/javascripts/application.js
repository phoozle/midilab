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
  $('.popup_toggle').each(function() {
    var about_data = $(this).attr('data-about')
    $(this).click(function(){
      if(popup_status == 0) {
        $('#popup_background').fadeTo('slow', 0.5)
        $('#popup_holder').fadeIn('slow');
        popup_status = 1
      }
      else {
        $('#popup_background').fadeOut('slow')
        $('#popup_holder').fadeOut('slow')
        popup_status = 0
      }
    });
  });
});
