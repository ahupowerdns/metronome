"use strict";

var updateEverything = function(){}
$(document).ready(function() {
    $.ajaxSetup({ cache: false });
    
    var m = new Metronome( {url: metronomeServer, datapoints: 150 });

    $(window).bind('popstate',  
		   function(event) {
		       if(event.originalEvent.state != undefined) {
			   $("#server").val(event.originalEvent.state.server);
			   $("#duration").val(event.originalEvent.state.beginTime);
			   configAll();
			   updateEverything();
		       }
		   });

    function configAll()
    {
	var servername=$("#server").val();
	if(servername=='')
	    return;

        if(servername=='All') {
            var allqueries=[];
            var servfails=[];
            
            $.each(m.servers, function(key,val) {
                var re=/.auth$/;
                if(re.test(val))
                    allqueries.push(val+".udp-queries");
                else if((/.recursor$/).test(val)) {
                        allqueries.push(val+".questions");
                        servfails.push(val+".servfail-answers");
                    }
                });
                
            var config ={ items: [ 
            { metrics: servfails, legend: "Total servfails/s",
	      formula: function(r,d) { 
	          var ret=0;
	          $.each(d, function(key,val) { if(key<servfails.length) ret+= val; });
	          return ret;
	          }
	    },
	    { metrics: allqueries, legend: "Total queries/s",
	      formula: function(r,d) { 
	          var ret=0;
	          $.each(d, function(key,val) { if(key>=servfails.length) ret+= val; });
	          return ret;
	          }
	    }
            ]};
            
            m.setupGraphs("#graphs", [config]);	
            return;
        }

	var config1 = { items: [ 
            { name: servername+".servfail-answers", legend: "Servfail answers/s"},
            { name: servername+".questions", legend: "Questions/s" }, 
            { name: servername+".policy-drops", legend: "Policy drops/s" },
            { name: servername+".all-outqueries", legend: "All outqueries/s"}
        ]};	
	
        var config1a = { items: [ 
            { name: servername+".qa-latency", legend: "Latency (usec)", kind: "gauge"},
            { name: servername+".x-our-latency", legend: "Non-network latency (usec)", kind: "gauge"}
            ]};


	
	var config2 = { renderer: 'stack', items: [ 
            { name: servername+".answers-slow", legend: "Slow answers/s"},
	    { name: servername+".packetcache-hits", legend: "0ms answers/s"},
            { name: servername+".answers0-1", legend: "<1 ms answers/s"},
            { name: servername+".answers1-10", legend: "1-10ms answers/s"},
            { name: servername+".answers10-100", legend: "10-100ms answers/s"},
            { name: servername+".answers100-1000", legend: "100-1000ms answers/s"}
	    
	]};
	
	var config2a = { renderer: 'stack', items: [ 
            { name: servername+".servfail-answers", legend: "SERVFAIL answers/s"},
            { name: servername+".nxdomain-answers", legend: "NXDOMAIN answers/s"},
            { name: servername+".noerror-answers", legend: "Normal answers/s"}	    
	]};
	
	var config2b = { items: [ 
            { name: servername+".tcp-questions", legend: "TCP/IP questions/s"},
            { name: servername+".tcp-outqueries", legend: "TCP/IP outqueries/s"},
            { name: servername+".tcp-client-overflow", legend: "TCP/IP overflows/s"}	    
	]};

	
	var config3 ={ renderer: "stack", items: [ 
            { metrics: [servername+".sys-msec"], legend: "System CPU%",
	      formula: function(r,d) { return d[0]/10.0; }
	    }, 
            { metrics: [servername+".user-msec"], legend: "User CPU%",
	      formula: function(r,d) { return d[1]/10.0; }
	    }]};

	var config3threads ={items: [ 
            { metrics: [servername+".cpu-msec-thread-0"], legend: "Thread 0 CPU%",
	      formula: function(r,d) { return d[0]/10.0; }
	    }, 
            { metrics: [servername+".cpu-msec-thread-1"], legend: "Thread 1 CPU%",
	      formula: function(r,d) { return d[1]/10.0; }
	    },
            { metrics: [servername+".cpu-msec-thread-2"], legend: "Thread 2 CPU%",
	      formula: function(r,d) { return d[2]/10.0; }
	    },
            { metrics: [servername+".cpu-msec-thread-3"], legend: "Thread 3 CPU%",
	      formula: function(r,d) { return d[3]/10.0; }
	    },
            { metrics: [servername+".cpu-msec-thread-4"], legend: "Thread 4 CPU%",
	      formula: function(r,d) { return d[4]/10.0; }
	    },
            { metrics: [servername+".cpu-msec-thread-5"], legend: "Thread 5 CPU%",
	      formula: function(r,d) { return d[5]/10.0; }
	    },
            { metrics: [servername+".cpu-msec-thread-6"], legend: "Thread 6 CPU%",
	      formula: function(r,d) { return d[6]/10.0; }
	    }
            

        ]};

        var config3threads2 = { items: [ 
            { name: servername+".rebalanced-queries", legend: "Rebalanced/s" }
        ]};

        
	var config3a = { items: [ 
            { name: servername+".outgoing-timeouts", legend: "Out.timeouts/s" }, 	
            { name: servername+".unreachables", legend: "Unreachable/s" }, 	
            { name: servername+".throttled-out", legend: "Throttled/s" }, 
            { name: servername+".resource-limits", legend: "Resource limited/s"},
            { name: servername+".over-capacity-drops", legend: "Capacity drop/s" }
            ]};

	var config3aa = { renderer: 'stack', items: [ 
            { name: servername+".auth4-answers0-1", legend: "<1 ms auth4 answers/s"},
            { name: servername+".auth4-answers1-10", legend: "1-10ms auth4 answers/s"},
            { name: servername+".auth4-answers10-100", legend: "10-100ms auth4 answers/s"},
            { name: servername+".auth4-answers100-1000", legend: "100-1000ms auth4 answers/s"},
            { name: servername+".auth4-answers-slow", legend: "Slow auth4 answers/s"},
            { name: servername+".outgoing4-timeouts", legend: "Outgoing4.timeouts/s" } 	
	]};

	var config3ab = { renderer: 'stack', items: [ 
            { name: servername+".auth6-answers0-1", legend: "<1 ms auth6 answers/s"},
            { name: servername+".auth6-answers1-10", legend: "1-10ms auth6 answers/s"},
            { name: servername+".auth6-answers10-100", legend: "10-100ms auth6 answers/s"},
            { name: servername+".auth6-answers100-1000", legend: "100-1000ms auth6 answers/s"},
            { name: servername+".auth6-answers-slow", legend: "Slow auth6 answers/s"},
            { name: servername+".outgoing6-timeouts", legend: "Out6.timeouts/s" } 	
	]};

	var config3ac = { renderer: 'stack', items: [ 
            { name: servername+".x-ourtime-slow", legend: "Ourtime slow/s" }, 		
            { name: servername+".x-ourtime0-1", legend: "Ourtime 0-1ms/s"},
            { name: servername+".x-ourtime1-2", legend: "Ourtime 1-2ms/s"},
            { name: servername+".x-ourtime2-4", legend: "Ourtime 2-4ms/s"},
            { name: servername+".x-ourtime4-8", legend: "Ourtime 4-8ms/s"},
            { name: servername+".x-ourtime8-16", legend: "Ourtime 8-16ms/s"},
            { name: servername+".x-ourtime16-32", legend: "Ourtime 16-32ms/s"}
	]};


	var config3b ={ items: [
            { name: servername+".concurrent-queries", legend: "Concurrent queries", kind: "gauge"}
        ]};

	
	var config4 = { items: [ 
            { 
		metrics: [servername+".packetcache-hits",servername+".packetcache-misses"], 
		legend: "% packetcache hitrate", 
		formula: m.percentalizer
	    }]};    

	var config4a = { items: [ 
            { 
		metrics: [servername+".variable-responses",servername+".questions"], 
		legend: "% variable responses", 
		formula: m.percentalizer
	    }]};    


	var config5 = { items: [ 
            { 
		metrics: [servername+".cache-hits",servername+".cache-misses"], 
		legend: "% cache hitrate", 
		formula: m.percentalizer
	    }]};    


	var config6 ={ renderer: "stack", items: [
            { name: servername+".cache-entries", legend: "Cache entries", kind: "gauge"}, 
            { name: servername+".packetcache-entries", legend: "Packetcache entries", kind: "gauge"}]};

	var config6a=    { items: [
	        { name: servername+".real-memory-usage", legend: "Memory usage (bytes)", kind: "gauge"}
            ]};

        var config6aa = { items: [
                { name: servername+".fd-usage", legend: "Used filedescriptors", kind: "gauge"}
            ]};

        var config6b = { items: [
                { name: servername+".uptime", legend: "Uptime (seconds)", kind: "gauge"}
            ]};


	var config7 = { items: [ 
            { name: servername+".udp-queries", legend: "UDP Questions/s" }, 
            { name: servername+".udp-answers", legend: "UDP Answers/s"}, 
            { name: servername+".udp6-queries", legend: "UDP6 Questions/s" }, 
            { name: servername+".udp6-answers", legend: "UDP6 Answers/s"}            
            ]};

	var config7a = { items: [ 
            { name: servername+".tcp-queries", legend: "TCP Questions/s" }, 
            { name: servername+".tcp-answers", legend: "TCP Answers/s"}]};

        var config7aa = { items: [ 
            { name: servername+".rd-queries", legend: "RD Questions/s" }, 
            { name: servername+".recursing-answers", legend: "RD Answers/s"},
            { name: servername+".recursing-questions", legend: "RD Questions/s"},
            { name: servername+".recursion-unanswered", legend: "RD Unanswered/s"}            
            
            ]};

        var config7ab = { items: [ 
            { name: servername+".udp-recvbuf-errors", legend: "UDP recvbuf errors/s" }, 
            { name: servername+".udp-sndbuf-errors", legend: "UDP sndbuf errors/s"},
            { name: servername+".udp-noport-errors", legend: "UDP noport errors/s"},
            { name: servername+".udp-in-errors", legend: "UDP in-errors/s"},
            { name: servername+".too-old-drops", legend: "Too-old drops/s"}  
            
            ]};

	var config7ac = { items: [ 
            { name: servername+".ecs-responses", legend: "ECS answers/s from auths"},
            { name: servername+".ecs-queries", legend: "ECS questions/s to auths" }
            
            ]};

	var config7ad = { items: [
            {
		metrics: [servername+".sys-msec",servername+".user-msec", servername+".questions"],
		legend: "QPS per CPU capacity",
		formula: function(r, d) {
		        if(d[0] > 0 && d[1] >0) {
		            return 1000.0*d[2] / (d[0] + d[1]);
                        }
                        return 0;
                    }
	    }]};
            
            

	var config7b = { items: [ 
            { name: servername+".qsize-q", legend: "DB Queue", kind: "gauge"}
            ]};

	var config7c = { items: [ 
            { name: servername+".timedout-packets", legend: "Timedout queries"},
            { name: servername+".servfail-packets", legend: "Servfail answers"},
            ]};

	var config7d = { items: [ 
            { name: servername+".latency", legend: "Latency (usec)", kind: "gauge"}
            ]};


	var config8 = { items: [ 
            { 
		metrics: [servername+".packetcache-hit",servername+".packetcache-miss"], 
		legend: "% packet cache hitrate", 
		formula: m.percentalizer
	    }]};    

	var config9 = { items: [ 
            { 
		metrics: [servername+".query-cache-hit",servername+".query-cache-miss"], 
		legend: "% query cache hitrate", 
		formula: m.percentalizer
 	    }]};    

	var config10 = { items: [ 
		{name: servername+".query-cache-miss", legend: "Database queries/s"}
	]};

	var config10a = { items: [ 
            { 
		metrics: [servername+".query-cache-miss",servername+".udp-answers"], 
		legend: "% query to DB amplification", 
		formula: function(r, d) {
		        if(d[0] > 0 && d[1] >0) {
		                return d[0]*100.0/d[1];
                        }
                        return 0;
                    }
	    }]};    


	var config11 = { items: [ 
		{name: servername+".deferred-cache-inserts", legend: "Deferred cache inserts/s"},
		{name: servername+".deferred-cache-lookup", legend: "Deferred cache lookups/s"},
		{name: servername+".deferred-packetcache-inserts", legend: "Deferred packet cache inserts/s"},
		{name: servername+".deferred-packetcache-lookup", legend: "Deferred packet cache lookups/s"},
	]};

	var config12 ={ items: [
	        { name: servername+".packetcache-size", legend: "Packet cache size", kind: "gauge"},
	        { name: servername+".query-cache-size", legend: "Query cache size", kind: "gauge"}
        ]};

	var config13 ={ items: [
	        { name: servername+".meta-cache-size", legend: "Meta cache size", kind: "gauge"},
	        { name: servername+".key-cache-size", legend: "Key cache size", kind: "gauge"},	        
	        { name: servername+".signature-cache-size", legend: "Signature cache size", kind: "gauge"},	        
        ]};

	var config14 = { items: [ 
		{name: servername+".signatures", legend: "DNSSEC signatures/s"},
	]};
	            

	var configs;
	var components = servername.split('.');
	if(components[2]=="recursor") { 
	    configs=[config1, config1a, config2, config2a, config2b, config3, config3threads, config3threads2, config3a, config3aa, config3ab, config3ac, config3b, config4, config4a, config5, config6, config6a, config6b, config7ab, config7ac, config7ad];

            configs.push({items:[ {name: servername+".dnssec-queries", legend: "DNSSEC queries/s"},
                                  {name: servername+".dnssec-validations", legend: "DNSSEC validations/s"}]});

            configs.push({ renderer: 'stack', items: [ 
            { name: servername+".dnssec-result-bogus", legend: "bogus/s"},
            { name: servername+".dnssec-result-insecure", legend: "insecure/s"},
                { name: servername+".dnssec-result-secure", legend: "secure/s"},
            { name: servername+".dnssec-result-indeterminate", legend: "indeterminate/s"},
            { name: servername+".dnssec-result-nta", legend: "neg trust anchor/s"}
            
	]});

            if ("filter" in m.hierarchy["pdns"][components[1]]["recursor"]) {
                var stackconfig1={renderer: 'stack', items: []};
                var stackconfig2={renderer: 'stack', items: []};
                var filters=m.listMetricsAt("pdns",components[1], "recursor", "filter");
                var stackcount=0;
                $.each(filters, function(a,b){
                    var thename="pdns."+components[1]+".recursor.filter."+b+".count";
                    if(stackcount++ < 5)
                        stackconfig1.items.push({name: thename, legend: b+" filtered/s"});
                    else
                        stackconfig2.items.push({name: thename, legend: b+" filtered/s"});
//                    config={items:[ {name: thename, legend: b+" filtered/s"}]};
                    config={items:[ { metrics: [thename, "pdns."+components[1]+".recursor.questions"],
                                    legend: b+" % of queries filtered",
                                    formula: m.percentalizer }]};
                    configs.push(config);
                });
                configs.push(stackconfig1);
                configs.push(stackconfig2);
/*
	        var glob = { items: [ 
                    { name: "pdns."+components[1]+".recursor.global-blocked.count", legend: "Global blocks/s"},
                    { name: "pdns."+components[1]+".recursor.parental-blocked.count", legend: "Category blocks/s"}
                ]};	

                configs.push(glob); */
            }
                
	}
	else if(components[2]=="auth") {
	    configs=[config3, config6a, config6aa, config6b, config7, config7a, config7aa, config7ab, config7b, config7c, config7d, config8, config9, config10, config10a, config11, config12, config13, config14];
	}
	else if(components[0]=="dnsdist") {
	    configs=[ { 
		items: [ 
		    { name: "dnsdist."+components[1]+".main.queries", legend: "Queries/s"},
                    { metrics: ["dnsdist."+components[1]+".main.cache-hits", "dnsdist."+components[1]+".main.responses"], legend: "Responses/s", 
                        formula: function(r,d) { return d[1]+d[2]; }}
		]
	    },
	    { renderer: 'stack', items: [ 
	    		  { name: "dnsdist."+components[1]+".main.no-policy", legend: "No-policy/s"},
			  { name: "dnsdist."+components[1]+".main.responses", legend: "Backend responses/s"},
			  { name: "dnsdist."+components[1]+".main.cache-hits", legend: "Cache hits/s"},
			  { name: "dnsdist."+components[1]+".main.downstream-timeouts", legend: "Downstream timeout/s"},
			  { name: "dnsdist."+components[1]+".main.self-answered", legend: "Self-answered/s"},
			  { name: "dnsdist."+components[1]+".main.rule-drop", legend: "Rule-drop/s"}
			   
                      ]},
		      { renderer: 'stack', items: [ 
			  { name: "dnsdist."+components[1]+".main.latency-slow", legend: "Slow answers/s"},
			  { name: "dnsdist."+components[1]+".main.latency0-1", legend: "<1 ms answers/s"},
			  { name: "dnsdist."+components[1]+".main.latency1-10", legend: "1-10ms answers/s"},
			  { name: "dnsdist."+components[1]+".main.latency10-50", legend: "10-50ms answers/s"},
			  { name: "dnsdist."+components[1]+".main.latency50-100", legend: "50-100ms answers/s"},
			  { name: "dnsdist."+components[1]+".main.latency100-1000", legend: "100-1000ms answers/s"} ]},
            { 
		items: [ 
		    {name: "dnsdist."+components[1]+".main.downstream-timeouts", legend: "Timeouts/s"},
		    {name: "dnsdist."+components[1]+".main.downstream-send-errors", legend: "Errors/s"},
		]
	    },
            { 
		items: [ 
		    {name: "dnsdist."+components[1]+".main.rule-drop", legend: "Rule drops/s"},
		    {name: "dnsdist."+components[1]+".main.acl-drops", legend: "ACL drops/s"},
		    {name: "dnsdist."+components[1]+".main.dyn-blocked", legend: "Dynamic drops/s"},
		    {name: "dnsdist."+components[1]+".main.block-filter", legend: "Blockfilter drops/s"}
		]
	    },
	    { items: [ 
              { 
		metrics: ["dnsdist."+components[1]+".main.cache-misses",
		          "dnsdist."+components[1]+".main.cache-hits"],
		legend: "% cache missrate", 
		formula: m.percentalizer
	     }]},
	    {
		items: [
		    {name: "dnsdist."+components[1]+".main.rdqueries", legend: "RD Queries/s"},
		    {name: "dnsdist."+components[1]+".main.rule-nxdomain", legend: "Rule NXDomain/s"},
		    {name: "dnsdist."+components[1]+".main.self-answered", legend: "Rule self-answered/s"},
		    {name: "dnsdist."+components[1]+".main.no-policy", legend: "No policy/s"},
		    {name: "dnsdist."+components[1]+".main.noncompliant-queries", legend: "Non-compliant queries/s"},
		    {name: "dnsdist."+components[1]+".main.noncompliant-responses", legend: "Non-compliant responses/s"},
		    {name: "dnsdist."+components[1]+".main.empty-queries", legend: "Empty queries/s"}
		]
	    },
	    { items: [
	        { name: "dnsdist."+components[1]+".main.latency-avg100", legend: "100 packet avg latency", kind: "gauge"},
	        { name: "dnsdist."+components[1]+".main.latency-avg10000", legend: "10000 packet avg latency", kind: "gauge"},	        
	        { name: "dnsdist."+components[1]+".main.latency-avg1000000", legend: "1000000 packet avg latency", kind: "gauge"},	        
            ]},
            { renderer: "stack", items: [
                { metrics: ["dnsdist."+components[1]+".main.cpu-sys-msec"], legend: "System CPU%", formula: function(r,d) { return d[0]/10.0; } },
                { metrics: ["dnsdist."+components[1]+".main.cpu-user-msec"], legend: "User CPU%", formula: function(r,d) { return d[1]/10.0; } },
            ]},
	    { items: [
	        { name: "dnsdist."+components[1]+".main.real-memory-usage", legend: "Memory usage (bytes)", kind: "gauge"}
            ]},
	    { items: [
	        { name: "dnsdist."+components[1]+".main.fd-usage", legend: "Used filedescriptors", kind: "gauge"}
            ]},            
	    { items: [
	        { name: "dnsdist."+components[1]+".main.uptime", legend: "Uptime (seconds)", kind: "gauge"}
            ]},
	    { items: [
	        { name: "dnsdist."+components[1]+".main.dyn-block-nmg-size", legend: "Dynamic blocks size", kind: "gauge"}
            ]}
		      ];

            if ("servers" in m.hierarchy["dnsdist"][components[1]]["main"]) {
                var servers=m.listMetricsAt("dnsdist", components[1], "main", "servers");
                var queries_values=[];
                var drops_values=[];
                var latency_values=[];
                var senderrors_values=[];
                var outstanding_values=[];
                var per_server_values=[];
                var servers_count=0;
                $.each(servers, function(key, val) {
                    servers_count++;
                    queries_values.push(
                        { name: "dnsdist."+components[1]+".main.servers."+val+".queries", legend: val+" Queries/s"}
                    );
                    drops_values.push(
                        { name: "dnsdist."+components[1]+".main.servers."+val+".drops", legend: val+" Drops/s"}
                    );
                    senderrors_values.push(
                        { name: "dnsdist."+components[1]+".main.servers."+val+".senderrors", legend: val+" Send Errors/s"}
                    );
                    latency_values.push(
                        { name: "dnsdist."+components[1]+".main.servers."+val+".latency", legend: val+" Latency", kind: "gauge"}
                    );
                    outstanding_values.push(
                        { name: "dnsdist."+components[1]+".main.servers."+val+".outstanding", legend: val+" Outstanding", kind: "gauge"}
                    );
                    per_server_values.push(
                        { items: [
                            { name: "dnsdist."+components[1]+".main.servers."+val+".queries", legend: val+" Queries/s"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.servers."+val+".drops", legend: val+" Drops/s"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.servers."+val+".senderrors", legend: val+" Send Errors/s"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.servers."+val+".outstanding", legend: val+" Outstanding", kind: "gauge"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.servers."+val+".latency", legend: val+" Latency", kind: "gauge"}
                        ]}
                    );
                });
                if (servers_count > 1) {
                    configs.push(
                        { renderer: 'stack', items: queries_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: drops_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: senderrors_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: outstanding_values}
                    );
                    configs.push(
                        { items: latency_values}
                    );
                }
                configs.push.apply(configs, per_server_values);
            }
            if ("frontends" in m.hierarchy["dnsdist"][components[1]]["main"]) {
                var frontends=m.listMetricsAt("dnsdist", components[1], "main", "frontends");
                var queries_values=[];
                var per_frontend_values=[];
                var frontends_count=0;
                $.each(frontends, function(key, val) {
                    frontends_count++;
                    queries_values.push(
                        { name: "dnsdist."+components[1]+".main.frontends."+val+".queries", legend: val+" Queries/s"}
                    );
                    per_frontend_values.push(
                        { items: [
                            { name: "dnsdist."+components[1]+".main.frontends."+val+".queries", legend: val+" Queries/s"}
                        ]}
                    );
                });
                if (frontends_count > 1) {
                    configs.push(
                        { renderer: 'stack', items: queries_values}
                    );
                }
                configs.push.apply(configs, per_frontend_values);
            }
            console.log("Dus...");
            if ("doh" in m.hierarchy["dnsdist"][components[1]]["main"]) {           
                console.log("hiero");
                var dohs=m.listMetricsAt("dnsdist", components[1], "main", "doh");             
                var https2_values=[];                
                var https1_values=[];
                var method_values=[];
                var tls_values=[];
                var connect_values=[];
                var dispo_values=[];                
                
                $.each(dohs, function(key, val) {
                    pools_count++;
                    https2_values.push(
                        { name: "dnsdist."+components[1]+".main.doh."+val+".http2-queries", legend: "http2 "+val}
                    );                
                    https1_values.push(
                        { name: "dnsdist."+components[1]+".main.doh."+val+".http1-queries", legend: "http1 "+val}
                    );

                    tls_values.push(
                        { name: "dnsdist."+components[1]+".main.doh."+val+".tls10-queries", legend: "TLS 1.0 "+val}
                    );                

                    tls_values.push(
                        { name: "dnsdist."+components[1]+".main.doh."+val+".tls11-queries", legend: "TLS 1.1 "+val}
                    );

                    tls_values.push(
                        { name: "dnsdist."+components[1]+".main.doh."+val+".tls12-queries", legend: "TLS 1.2 "+val}
                    );                

                    tls_values.push(
                        { name: "dnsdist."+components[1]+".main.doh."+val+".tls13-queries", legend: "TLS 1.3 "+val}
                    );                
/*
                    tls_values.push(
                        { name: "dnsdist."+components[1]+".main.doh."+val+".tls-unknown-queries", legend: "TLS ? "+val}
                    );                
*/
                    method_values.push(
                        { name:"dnsdist."+components[1]+".main.doh."+val+".get-queries", legend: "get "+val}
                    );                   
                    method_values.push(
                        { name:"dnsdist."+components[1]+".main.doh."+val+".post-queries", legend: "post "+val}
                    );
                    connect_values.push(
                        { name:"dnsdist."+components[1]+".main.doh."+val+".http-connects", legend: "connects "+val}
                    );
                    dispo_values.push(
                        { name:"dnsdist."+components[1]+".main.doh."+val+".bad-requests", legend: "bad "+val}
                    );
                    dispo_values.push(
                        { name:"dnsdist."+components[1]+".main.doh."+val+".valid-responses", legend: "valid resp "+val}
                    );
                    dispo_values.push(
                        { name:"dnsdist."+components[1]+".main.doh."+val+".error-responses", legend: "error resp "+val}
                    );
                })
                configs.push(
                        { renderer: 'stack', items: https2_values}
                    );
                configs.push(
                        { renderer: 'stack', items: https1_values}
                    );
                configs.push(
                        { renderer: 'stack', items: method_values}
                    );
                configs.push(
                        { renderer: 'stack', items: tls_values}
                    );

                configs.push(
                        { renderer: 'stack', items: connect_values}
                    );
                configs.push(
                        { renderer: 'stack', items: dispo_values}
                    );                    
            }
            if ("pools" in m.hierarchy["dnsdist"][components[1]]["main"]) {
                var pools=m.listMetricsAt("dnsdist", components[1], "main", "pools");
                var servers_values=[];
                var cache_size_values=[];
                var cache_entries_values=[];
                var cache_hits_values=[];
                var cache_misses_values=[];
                var cache_deferred_inserts_values=[];
                var cache_deferred_lookups_values=[];
                var cache_lookup_collisions_values=[];
                var cache_insert_collisions_values=[];
                var per_pool_values=[];
                var pools_count=0;
                $.each(pools, function(key, val) {
                    pools_count++;
                    servers_values.push(
                        { name: "dnsdist."+components[1]+".main.pools."+val+".servers", legend: val+" Servers", kind: "gauge"}
                    );
                    cache_size_values.push(
                        { name: "dnsdist."+components[1]+".main.pools."+val+".cache-size", legend: val+" Cache size", kind: "gauge"}
                    );
                    cache_entries_values.push(
                        { name: "dnsdist."+components[1]+".main.pools."+val+".cache-entries", legend: val+" Cache entries", kind: "gauge"}
                    );
                    cache_hits_values.push(
                        { name: "dnsdist."+components[1]+".main.pools."+val+".cache-hits", legend: val+" Cache hits"}
                    );
                    cache_misses_values.push(
                        { name: "dnsdist."+components[1]+".main.pools."+val+".cache-misses", legend: val+" Cache misses"}
                    );
                    cache_deferred_inserts_values.push(
                        { name: "dnsdist."+components[1]+".main.pools."+val+".cache-deferred-inserts", legend: val+" Cache deferred inserts"}
                    );
                    cache_deferred_lookups_values.push(
                        { name: "dnsdist."+components[1]+".main.pools."+val+".cache-deferred-lookups", legend: val+" Cache deferred lookups"}
                    );
                    cache_lookup_collisions_values.push(
                        { name: "dnsdist."+components[1]+".main.pools."+val+".cache-lookup-collisions", legend: val+" Cache lookup collisions"}
                    );
                    cache_insert_collisions_values.push(
                        { name: "dnsdist."+components[1]+".main.pools."+val+".cache-insert-collisions", legend: val+" Cache insert collisions"}
                    );
                    per_pool_values.push(
                        { items: [
                            { name: "dnsdist."+components[1]+".main.pools."+val+".servers", legend: val+" Servers", kind: "gauge"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.pools."+val+".cache-size", legend: val+" Cache size", kind: "gauge"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.pools."+val+".cache-entries", legend: val+" Cache entries", kind: "gauge"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.pools."+val+".cache-hits", legend: val+" Cache hits"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.pools."+val+".cache-misses", legend: val+" Cache misses"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.pools."+val+".cache-deferred-inserts", legend: val+" Cache deferred inserts"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.pools."+val+".cache-deferred-lookups", legend: val+" Cache deferred lookups"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.pools."+val+".cache-lookup-collisions", legend: val+" Cache lookup collisions"}
                        ]},
                        { items: [
                            { name: "dnsdist."+components[1]+".main.pools."+val+".cache-insert-collisions", legend: val+" Cache insert collisions"}
                        ]}
                    );
                });
                if (pools_count > 1) {
                    configs.push(
                        { renderer: 'stack', items: servers_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: cache_size_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: cache_entries_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: cache_hits_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: cache_misses_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: cache_deferred_inserts_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: cache_deferred_lookups_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: cache_lookup_collisions_values}
                    );
                    configs.push(
                        { renderer: 'stack', items: cache_insert_collisions_values}
                    );
                }
                configs.push.apply(configs, per_pool_values);
            }
        }
	else if(components[0]=="system" && components[2]=="network") { 
	    configs=[ { 
		items: [ 
		    {name: servername+".udp.in-errors", legend: "UDP Input error/s"},
		    {name: servername+".udp.sndbuf-errors", legend: "UDP SNDBuf error/s"},
		    {name: servername+".udp.rcvbuf-errors", legend: "UDP RCVBuf error/s"},
		    {name: servername+".udp.noport-errors", legend: "UDP Noport error/s"}
		]
	    }];

            var interfaces=m.listMetricsAt(components[0], components[1], components[2], "interfaces");
            $.each(interfaces, function(key, val) {
		configs.push({
		items: [ 
		    { name: servername+".interfaces."+val+".tx_bytes", legend: val+" TX bits/s", bytesToBits: true},
		    { name: servername+".interfaces."+val+".rx_bytes", legend: val+" RX bits/s", bytesToBits: true},
		    ]
		});
		configs.push({items: [ 
		    { name: servername+".interfaces."+val+".tx_packets", legend: val+" TX packets/s"},
		    { name: servername+".interfaces."+val+".rx_packets", legend: val+" RX packets/s"},
		    ]
			     });
	    });		      
	}
	else if(components[0]=="power") { 
	    configs=[ { 
		items: [ 
		    {
                        name: "power."+components[1]+".joules", legend: "Watt"
                    }
		]
	    },
	    { 
		items: [ 
		    {
                        metrics: ["power."+components[1]+".joules"], legend: "kWh", formula: function(r,d) { return r[0]/3600000.0; }
                    }
		]
	    }
	    ];

	}
	
	m.setupGraphs("#graphs", configs);	
    }

    var interval;     

    window.updateEverything = function() { 
	m.comconfig.beginTime = parseInt($("#duration").val());
	
	if(interval != undefined)
	    clearInterval(interval);
	interval = setInterval(updateEverything, m.getNaturalInterval());    

	m.updateGraphs();
    };    

    window.updateFromForm = function() {
	configAll();

	var stateObj = { server: $("#server").val(), beginTime: parseInt($("#duration").val()) };
	try {
	  history.pushState(stateObj, "Metronome", "?server="+stateObj.server+"&beginTime="+stateObj.beginTime);
	} catch(err) {}
	updateEverything();
    }


    m.getAllMetrics(function() { 
	var ret="";
	ret+="<option value='All'>All</option>";
	$.each(m.servers, function(a,b) {
	    ret+= "<option value='"+b+"'>"+b+"</option>";
	});
	$("#server").html(ret);

	if($.url().param('server') != undefined) {
	    $("#server").val($.url().param('server'));
	}
	if($.url().param('beginTime') != undefined)
	    $("#duration").val($.url().param('beginTime'));
	
	var stateObj = { server: $("#server").val(), beginTime: parseInt($("#duration").val()) };
	try {
	  history.replaceState(stateObj, "Metronome", "?server="+stateObj.server+"&beginTime="+stateObj.beginTime);
	} catch (err) {}
	
	configAll();
	updateEverything();
    });
});
