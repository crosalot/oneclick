$(document).ready(function() {
  var domain = Drupal.settings.domain;
  var site_id = Drupal.settings.site_id;
  var root_site= Drupal.settings.root_site;
  var checksite = setInterval(function() {
    // TODO: Build url in client site and return waiting = false
    $.getJSON('http://' + domain + '/waiting?callback=?', function(resp1) {
      if(!resp1.waiting){
        window.location.href = 'http://' + domain;
      }
    });
  }, 3*1000);
});
