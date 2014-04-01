
$(document).ready(function() {
    moment().format();
    
    function coordinateTransform(series)
    {
	var ret=[];
	$.each(series, function(a, b) {    
	    ret.push({x: b[0], y: b[1]});
	});
	return ret;
    }

    function showStuff(config, where) {
        var epoch = (new Date).getTime()/1000;
	var items = config.items;

	var qstring ="http://xs.powerdns.com:8000/?do=retrieve&callback=?&name=";
	var metrics=[];
	for(item in items) {
	    if(items[item].name != undefined)
		metrics.push(items[item].name);
	    if(items[item].metrics != undefined) {
		$.each(items[item].metrics, function(key, value) {
		    metrics.push(value);
		});
	    }
	}

	
	qstring+= metrics.join(',');
	qstring+="&begin="+(epoch-52601)+"&end="+(epoch+500);

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
		      for(num in items) {
			  var series;
			  if(items[num].kind=="gauge")
			      series = fullseries.raw;
			  else
			      series = fullseries.derivative;

			  
			  if(items[num].formula == undefined) 
			      toplot[num] = coordinateTransform(series[items[num].name]);
			  
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
		      $(where).html('<div class="chart_container"><div class="y_axis"></div><div class="chart"></div><div class="legend"></div>');

		      var plotseries=[];
		      var colors=['red', 'steelblue', 'green', 'yellow', 'purple', 'orange', 'black'];

		      for(num in items) {
			  plotseries.push( { color: colors[num], data: toplot[num], name: items[num].legend, renderer: 'line'});
		      }
		      
		      var graph = new Rickshaw.Graph( {
			  element: $(where + " .chart")[0], 
			  width: 450, 
			  height: 250, 
                          renderer: config.renderer || 'multi',
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
			  element: $(where+" .y_axis")[0]
		      } );
		
		      var legend = new Rickshaw.Graph.Legend( {
                          graph: graph,
                          element: $(where+ " .legend")[0]
                      } );		      
		      graph.render();		    
		  });	
    } 
    $.ajaxSetup({ cache: false });


    var config1 = { items: [ 
        { name: "pdns.localhost.recursor.questions", legend: "Questions/s" }, 
        { name: "pdns.localhost.recursor.all-outqueries", legend: "All outqueries/s"}]};


    var config2 = { renderer: 'stack', items: [ 
        { name: "pdns.localhost.recursor.answers-slow", legend: "Slow answers/s"},
	{ name: "pdns.localhost.recursor.packetcache-hits", legend: "0ms answers/s"},
        { name: "pdns.localhost.recursor.answers0-1", legend: "<1 ms answers/s"},
        { name: "pdns.localhost.recursor.answers1-10", legend: "1-10ms answers/s"},
        { name: "pdns.localhost.recursor.answers10-100", legend: "10-100ms answers/s"},
        { name: "pdns.localhost.recursor.answers100-1000", legend: "100-1000ms answers/s"}

    ]};

    var config3 ={ renderer: "stack", items: [ 
        { metrics: ["pdns.localhost.recursor.user-msec"], legend: "User CPU%",
	  formula: function(r,d) { return d[0]/10.0; }
	}, 
        { metrics: ["pdns.localhost.recursor.sys-msec"], legend: "System CPU%",
	  formula: function(r,d) { return d[0]/10.0; }
	}]};


    var config4 = { items: [ 
        { 
	    metrics: ["pdns.localhost.recursor.packetcache-hits","pdns.localhost.recursor.packetcache-misses"], 
	    legend: "% cache hitrate", 
	    formula: function(r, d) 
	    { 
		if(d[0] > 0 && d[1] > 0) 
		    return d[0]*100.0/(d[0] +d[1]); 
		else
		    return 0;
	    }
	}]};    


    var config5 ={ renderer: "stack", items: [
        { name: "pdns.localhost.recursor.cache-entries", legend: "Cache entries", kind: "gauge"}, 
        { name: "pdns.localhost.recursor.packetcache-entries", legend: "Packetcache entries", kind: "gauge"}]};


    function showAll()
    {
	showStuff(config1, "#hier1");
	showStuff(config2, "#hier2");
	showStuff(config3, "#hier3");
	showStuff(config4, "#hier4");
	showStuff(config5, "#hier5");
    }
    showAll();

    setInterval(showAll() ,5000);
    
});
     
