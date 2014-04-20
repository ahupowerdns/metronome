"use strict";

function coordinateTransform(series, factor)
{
    var ret=[];
    $.each(series, function(a, b) {    
	ret.push({x: b[0], y: factor * b[1]});
    });
    return ret;
}
function percentalizer(r, d)
{ 
    if(d[0] > 0 && d[1] > 0) 
	return d[0]*100.0/(d[0] +d[1]); 
    else
	return 0;
}

function listMetricsAt(metrics)
{
  console.log(arguments);
  var ref=metrics;
  for(var i = 1; i < arguments.length; ++i) {
    var tmp = ref[arguments[i]];
    ref = tmp;
  }
  var ret=[];
  $.each(ref, function(key, val) {
      ret.push(key);
    });
  ret.sort();
  console.log("resultaat", ret);
  return ret;
}

function getServers(comconfig, destination) 
{
    var qstring =comconfig.url+"?do=get-metrics&callback=?&name";

    $.getJSON(qstring, 
	      function(data) {	      
		  var theservers={};
		  var hier={};
		  $.each(data.metrics, function(a, b) {
		      var parts = b.split('.');
		      var name = parts.slice(0,3).join('.');
		      theservers[name]=1;

		      for(var i = 0; i < parts.length ; ++i) {
			  var ref = hier;
			  for(var j = 0 ; j < i; ++j) {
			      if(ref[parts[j]]==undefined)
				  ref[parts[j]]={};
			      ref = ref[parts[j]];
			  }
		      }
		  });
		  var ret=[];
		  $.each(theservers, function(a,b) {
		      ret.push(a);
		  });
		  ret.sort();
		  destination(ret, hier);
	      });
}
	     
function showStuff(comconfig, configs)
{
    $.each(configs, function(key, val) {
	showGraph(comconfig, val);
    });
}

function showGraph(comconfig, config) {
    var items = config.items;

    var qstring =comconfig.url+"?do=retrieve&callback=?&name=";
    var metrics=[];
    for(var item in items) {
	if(items[item].name != undefined)
	    metrics.push(items[item].name);
	if(items[item].metrics != undefined) {
	    $.each(items[item].metrics, function(key, value) {
		metrics.push(value);
	    });
	}
    }

    qstring+= metrics.join(',');

    var epoch = (new Date).getTime()/1000;
    qstring+="&begin="+(epoch+comconfig.beginTime)+"&end="+(epoch)+"&datapoints="+comconfig.datapoints;


    $.getJSON(qstring, 
	      function(fullseries) {	      
		  var toplot=[];
		  var grouped={};
		  
		  $.each(metrics, function(num, metric) {
		      $.each(fullseries.raw[metric], function(key, value) {
			  if(grouped[value[0]] == undefined) {
			      grouped[value[0]] = {};
			      grouped[value[0]].raw = {};
			      grouped[value[0]].derivative = {};
			  }
			  grouped[value[0]].raw[num]=value[1];
		      });
		      
		      $.each(fullseries.derivative[metric], function(key, value) {
			  grouped[value[0]].derivative[num]=value[1];
		      });
		  });
		  //		      console.log("Grouped", grouped);
		  for(var num in items) {
		      var series;
		      if(items[num].kind=="gauge")
			  series = fullseries.raw;
		      else
			  series = fullseries.derivative;

		      var factor = 1;
		      if(items[num].bytesToBits != undefined)
			  factor = 8;
		      if(items[num].formula == undefined) 
			  toplot[num] = coordinateTransform(series[items[num].name], factor);
		      
		  }

		  for(num in items) {
		      if(items[num].formula != undefined) {
			  toplot[num]=[];
			  //			      console.log("Going to do formula");
			  $.each(grouped, function(key, value) {
			      toplot[num].push({x: 1.0*key, y: items[num].formula(value.raw, value.derivative) });
			  });
			  //			      console.log(toplot[num]);
		      }
		  }


		  //		      console.log(grouped);		      

		  var plotseries=[];
		  var colors=['red', 'steelblue', 'green', 'orange', 'purple', 'black', 'yellow'];

		  for(num in items) {
		      plotseries.push( { color: colors[num], data: toplot[num], name: items[num].legend, renderer: 'line'});
		  }
		  config.div.html('<div class="chart_container"><div class="y_axis"></div><div class="chart"></div><div class="legend"></div>');

		  var graph = new Rickshaw.Graph( {
		      element: config.div.find(".chart")[0], 
		      width: 550, 
		      height: 250, 
                      renderer: config.renderer || 'multi',
		      padding: { top: 0.05 },
		      series: plotseries
                      
		  });
		  
		  var axes = new Rickshaw.Graph.Axis.Time( {
		      graph: graph,
		      orientation: 'bottom',
                      timeFixture: new Rickshaw.Fixtures.Time.Local()
		  } );
		  
		  var y_ticks = new Rickshaw.Graph.Axis.Y( {
		      graph: graph,
		      orientation: 'left',
		      tickFormat:
		      Rickshaw.Fixtures.Number.formatKMBT,
		      element: config.div.find(".y_axis")[0]
		  } );
		  
		  var legend = new Rickshaw.Graph.Legend( {
                      graph: graph,
                      element: config.div.find(".legend")[0]
                  } );		      
	
		  graph.render();

		  var hoverDetail = new Rickshaw.Graph.HoverDetail( {
		      graph: graph,

		      formatter: function(series, x, y) {			  
			  var swatch = '<span class="detail_swatch" style="background-color: ' + series.color + '"></span>';
			  var content = swatch + series.name + ": " + y.toFixed(2);
			  return content;
		      },
		      xFormatter: function(x) {
			  return new Date( x * 1000 ).toString();
		      }
		  } );
	      });	
}

function setupMetronomeHTML(where, configs)
{  
  var ret=[];
  $(where).html("");
  for(var a in configs) {
    configs[a].div = $('<div style="height: 300px;"/>');
    $(where).append(configs[a].div);
    ret.push(configs[a]);
  }
  return ret;
}
     
