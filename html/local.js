"use strict";
var metronomeServer="http://127.0.0.1:8000/";

$(document).ready(function(){
  $("#server").on('change', function(){ window.updateFromForm(); }).on('keyup', function(){ window.updateFromForm(); });
  $("#id").on('change', function(){ window.updateFromForm(); }).on('keyup', function(){ window.updateFromForm(); });
});
