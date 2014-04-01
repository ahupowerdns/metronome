
$(document).ready(function() {
    moment().format();

    function nonNegativeDerivative(series)
    {
	var prevval;
	var ret=[];
	$.each(series, function(a, b) {    
	    if(prevval != undefined) {
		ret.push([b[0], (b[1] - prevval[1])/(b[0]-prevval[0])]);
	    }
	    prevval=b;
	});
	return ret;
    }
    
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
	var names=[];
	for(item in items)
	    names.push(items[item].name);
	qstring+= names.join(',');
	qstring+="&begin="+(epoch-3601)+"&end="+(epoch+500);

	$.getJSON(qstring, 
		  function(series) {
		      var toplot=[];
		      for(num in items) {
			  if(items[num].transform != undefined)
			      toplot[num]= coordinateTransform(items[num].transform(series[items[num].name]));
			  else
			      toplot[num] = coordinateTransform(series[items[num].name]);
		      }
		      
		      $(where).html('<div class="chart_container"><div class="y_axis"></div><div class="chart"></div><div class="legend"></div>');
		      
		      var graph = new Rickshaw.Graph( {
			  element: $(where + " .chart")[0], 
			  width: 450, 
			  height: 250, 
                          renderer: config.renderer || 'multi',
			  series: [
                              {
			          color: 'red',
			          data: toplot[0],
                                  name: items[0].legend,
                                  renderer: 'line'
		              },
                              {
			          color: 'steelblue',
			          data: toplot[1],
                                  name: items[1].legend,
                                  renderer: 'line'
			      }                          
                          ]
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
    var config = { items: [ 
        { name: "pdns.localhost.recursor.questions", legend: "Questions/s", transform: nonNegativeDerivative }, 
        { name: "pdns.localhost.recursor.all-outqueries", legend: "All outqueries/s", transform: nonNegativeDerivative}]};

    showStuff(config, "#hier1");

    var config2 ={ renderer: "stack", items: [ 
        { name: "pdns.localhost.recursor.user-msec", legend: "User Msecs/s", transform: nonNegativeDerivative }, 
        { name: "pdns.localhost.recursor.sys-msec", legend: "System Msecs/s", transform: nonNegativeDerivative}]};
    showStuff(config2, "#hier2");

    var config3 ={ renderer: "stack", items: [ 
        { name: "pdns.localhost.recursor.packetcache-hits", legend: "hits/s", transform: nonNegativeDerivative }, 
        { name: "pdns.localhost.recursor.packetcache-misses", legend: "misses/s", transform: nonNegativeDerivative}]};
    showStuff(config3, "#hier3");

    var config4 ={ renderer: "stack", items: [
        { name: "pdns.localhost.recursor.cache-entries", legend: "Cache entries"}, 
        { name: "pdns.localhost.recursor.packetcache-entries", legend: "Packetcache entries"}]};
    showStuff(config4, "#hier4");


//    setInterval(function() { showStuff(items, "#hier1");} ,5000);
    
});
     
