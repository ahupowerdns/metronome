
$(document).ready(function() {
    moment().format();

    function showStuff() {
        var epoch = (new Date).getTime()/1000;
        var series={};
        var items = [ 
          { name: "pdns.localhost.recursor.questions", legend: "Questions/s" }, 
          { name: "pdns.localhost.recursor.all-outqueries", legend: "All outqueries/s"}];
        
	$.getJSON("http://xs.powerdns.com:8000/?do=retrieve&callback=?&name="+items[0].name+","+items[1].name+"&begin="+(epoch-3601)+"&end=2000000000", 
		  function(series) {
//		      console.log(series);
//		      console.log(epoch);
		      var prevval = undefined;
		      var rxtraffic=[], txtraffic=[];
		      $.each(series[items[0].name], function(a, b) {
			      console.log("b[1]: "+b[1]);
			      if(prevval != undefined) {
				  rxtraffic.push({x: b[0], y: (b[1] - prevval[1])/(b[0]-prevval[0])  });
				  console.log((b[1] - prevval[1]) +" and " +(b[0]-prevval[0]));
			      }
			      else 
				  console.log("Skipped..");
			      prevval=b;
			  });
		      prevval = undefined;

		      $.each(series[items[1].name], function(a, b) {
			  if(prevval != undefined) {
			      txtraffic.push({x: b[0], y: (b[1] - prevval[1])/(b[0]-prevval[0])});
			  }
			  prevval=b;
		      });
		      
	              $("#chart").html("");
                      $("#y_axis").html("");
                      $("#legend").html("");
		      
		      var graph = new Rickshaw.Graph( {
			  element: document.querySelector("#chart"), 
			  width: 450, 
			  height: 250, 
                          renderer: 'multi',
			  series: [
                              {
			          color: 'red',
			          data: rxtraffic,
                                  name: items[0].legend,
                                  renderer: 'line'
		              },
                              {
			          color: 'steelblue',
			          data: txtraffic,
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
			  element:
			  document.getElementById('y_axis'),
		      } );
		
		      var legend = new Rickshaw.Graph.Legend( {
                          graph: graph,
                          element: document.getElementById('legend')
                      } );		      
		      graph.render();		    
		  });	
    } 
    $.ajaxSetup({ cache: false });
    showStuff();
    setInterval(showStuff,5000);
    
});
     
