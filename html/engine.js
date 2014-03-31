
$(document).ready(function() {
    moment().format();

    function showStuff() {
        var epoch = (new Date).getTime()/1000;
        var series={};
	$.getJSON("http://localhost:8000/?do=retrieve&callback=?&name=rxbytes,txbytes&begin="+(epoch-3600)+"&end=2000000000", 
		  function(series) {
		      console.log(series);
		      var prevval;
		      var rxtraffic=[], txtraffic=[];
		      $.each(series["rxbytes"], function(a, b) {
			  if(prevval != undefined) {
			      rxtraffic.push({x: b[0], y: 8*(b[1] - prevval[1])/(b[0]-prevval[0])  });
			  }
			  prevval=b;
		      });
		      prevval = undefined;

		      $.each(series["txbytes"], function(a, b) {
			  if(prevval != undefined) {
			      txtraffic.push({x: b[0], y: 8*(b[1] - prevval[1])/(b[0]-prevval[0])});
			  }
			  prevval=b;
		      });
		      var graph = new Rickshaw.Graph( {
			  element: document.querySelector("#chart"), 
			  width: 450, 
			  height: 250, 
                          renderer: 'multi',
			  series: [
                              {
			          color: 'red',
			          data: txtraffic,
                                  name: "TX Traffic, bits/s",
                                  renderer: 'stack'
		              },
                              {
			          color: 'steelblue',
			          data: rxtraffic,
                                  name: "RX Traffic, bits/s",
                                  renderer: 'stack'
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
    showStuff();
});
     
