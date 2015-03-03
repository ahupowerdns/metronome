"use strict";

function Metronome(comconfig)
{
    if(!(this instanceof Metronome))
	return new Metronome();
    this.config=[];
    this.hierarchy={};
    this.servers=[];
    this.comconfig=comconfig;
}

/* The ritual: 
   0) Have a 'div' where you want your graphs
   1) make a Metronome object -> new Metronome({url: "http://xs.powerdns.com:8080", datapoints: 150}) 
   2) call its getAllMetrics method with a startup function
   3) Your startup function gets called with metronome object which is ready & includes a metric hierarchy & server list
   4) In startup function, create a graphing config based on hierarchy & server list, and submit it setupHTML("#yourdiv", configs),
      which populates your div with empty graphs of the right size
   5) Call the updateGraphs method to actually retrieve data & draw graphs
   6) Call updateGraphs periodically (every m.getNaturalInterval() milliseconds if you want smooth transitions)
   7) Recall setupGraphs if you change what graphs you want plotted, will clear your div

The graph config, as passed to setupGraphs:
  an array of graphs, each with an 'items' field containing an array of metrics to be graphed.
  Each metric has a 'name', which says which metric is to be graphed, plus a legend which is the human friendly name
  Example:
  		configs.push({items: [ 
		    { name: "system.xs.interfaces.eth0.tx_packets", legend: "eth0 TX packets/s"},
		    { name: "system.xs.interfaces.eth0.rx_packets", legend: "eth0 RX packets/s"},
		    ]});
  By default, we plot the nonNegativeDerivative of the metric. 
  To turn bits into bytes, add 'bitsToBytes: true'.
  
  To stack a graph, set 'renderer: stack'.
  
  To do fancy things, instead of a name, add a 'metrics' field with an array of metrics. 
  Also, set a function as the formula field. This function gets passed all the metrics you specified (in order),
  plus their nonNegativeDerivative.  (so, we call formula(raw, derived)).
  
  We deliver one stock formula, 'Metronome.percentalizer', which can be used to calculate one rate as a percentage
  of two rates. So, cache hitrates for example:
  
  	configs.push({ items: [ 
            { 
		metrics: [servername+".cache-hits",servername+".cache-misses"], 
		legend: "% cache hitrate", 
		formula: m.percentalizer
	    });    
*/

// Pass this to 'formula' to get two rates as a percentage of their sum rate
Metronome.prototype.percentalizer=function(r, d)
{ 
    if(d[0] > 0 && d[1] > 0) 
	return d[0]*100.0/(d[0] +d[1]); 
    else
	return 0;
}

// Call this to get array of all metrics at a certain level (listMetricaAt("system", "server1", "network", "interfaces"))
Metronome.prototype.listMetricsAt=function()
{
  var ref=this.hierarchy;
  for(var i = 0; i < arguments.length; ++i) {
    var tmp = ref[arguments[i]];
    ref = tmp;
  }
  var ret=[];
  $.each(ref, function(key, val) {
      ret.push(key);
    });
  ret.sort();
  return ret;
}

// the startup function, 
Metronome.prototype.getAllMetrics=function(destination) 
{
    var qstring = this.comconfig.url+"?do=get-metrics&callback=?&name";
    var that=this;
    var alerter = window.setTimeout(function(){ alert("Could not contact Metronome statistics server at "+that.comconfig.url+". This is either due to a connectivity problem or a intervening firewall, or otherwise a timeout."); }, 1500);
    $.getJSON(qstring, 
	      function(data) {
	          window.clearTimeout(alerter);	      
		  var theservers={};
		  that.hierarchy={};
		  if(data.messages) {
		    $("#message").append(data.messages[0]);
		  }
		  $.each(data.metrics, function(a, b) {
		      var parts = b.split('.');
		      var name = parts.slice(0,3).join('.');
		      theservers[name]=1;

		      for(var i = 0; i < parts.length ; ++i) {
			  var ref = that.hierarchy;
			  for(var j = 0 ; j < i; ++j) {
			      if(ref[parts[j]]==undefined)
				  ref[parts[j]]={};
			      ref = ref[parts[j]];
			  }
		      }
		  });
		  that.servers=[];
		  $.each(theservers, function(a,b) {
		      that.servers.push(a);
		  });
		  that.servers.sort();
		  destination(that);
	      });
}
	     
Metronome.prototype.updateGraphs=function()
{
    var that=this;
    $.each(this.configs, function(key, val) {
	that._showGraph(val);
    });
}

Metronome.prototype._showGraph=function(config) {
    var items = config.items;

    var qstring = this.comconfig.url+"?do=retrieve&callback=?&name=";
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
    qstring+="&begin="+(epoch+this.comconfig.beginTime)+"&end="+(epoch)+"&datapoints="+this.comconfig.datapoints;

    var that=this;
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
			  toplot[num] = that._coordinateTransform(series[items[num].name], factor);
		      
		  }

		  for(num in items) {
		      if(items[num].formula != undefined) {
			  toplot[num]=[];
			  $.each(grouped, function(key, value) {
			      toplot[num].push({x: 1.0*key, y: items[num].formula(value.raw, value.derivative) });
			  });
		      }
		  }

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

// This interval represents a millisecond timestep that will keep the shape of your graph identical if you call updateGraphs
Metronome.prototype.getNaturalInterval=function()
{
    return -this.comconfig.beginTime*1000/this.comconfig.datapoints;
}


Metronome.prototype.setupGraphs=function(where, configs)
{  
  this.configs=[];
  $(where).html("");
  for(var a in configs) {
    configs[a].div = $('<div style="height: 300px;"/>');
    $(where).append(configs[a].div);
    this.configs.push(configs[a]);
  }
}
     
Metronome.prototype._coordinateTransform=function(series, factor)
{
    var ret=[];
    $.each(series, function(a, b) {    
	ret.push({x: b[0], y: factor * b[1]});
    });
    return ret;
}
