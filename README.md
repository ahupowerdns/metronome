metronome
=========

Mini-graphite so we can ship pdnscontrol w/o depending on graphite.

'metronome' implements the carbon protocol, plus accepts submissions via POST:

For example, to submit RX bytes for eth0:

while true; 
do
	VAL=$(ip -s link ls eth0 | grep RX -A 1 | tail -1 | awk '{print $1}')
	wget -q --post-data="" "http://127.0.0.1:8000/?do=store&name=rxbytes&timestamp=$(date +%s)&value=$VAL" -O /dev/null
	sleep 1
done

To retrieve data:

$ wget http://127.0.0.1:8000/?do=retrieve&name=rxbytes&begin=0&end=$(date +%s)&callback=jsonp

This delivers a JSONP callback with your values in there. 

To view, try html/index.html

status
======
"Toy" for now - stores all data in flat files, which we read in their entirety to 
replay data.

Also, in lieu of interpollating, we just drop 90% of all entries in the storage now.
