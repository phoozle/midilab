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
});
