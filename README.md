metronome
=========

Mini-graphite so we can ship pdnscontrol w/o depending on graphite.

'metronome' implements the carbon protocol, so anything that can feed Graphite can feed metronome. 
We also accept submissions via POST:

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

Installing
==========
Either install libeigen3-dev (Debian, Ubuntu), or:

    $ wget http://bitbucket.org/eigen/eigen/get/3.2.1.tar.bz2
    $ tar xf 3.2.1.tar.bz2
    $ ln -s eigen-eigen-*/Eigen .

Then:

    $ make
    $ ./metronome --help
    $ mkdir stats
    $ ./metronome --stats-directory=./stats

Next, host the 'html/' directory somewhere on a webserver, and edit 'html/local.js' so
it knows the Webserver IP address of metronome (port 8000 on :: by default).

Thanks to
=========
Aki Tuomi for the excellent [yahttp](https://github.com/cmouse/yahttp), 
[Rickshaw](http://code.shutterstock.com/rickshaw/) for the graphs. [Eigen](http://eigen.tuxfamily.org) for the math. And a cast of thousands for C++2011.

status
======
"Toy" for now - stores all data in flat files, which we read in their entirety to 
replay data.

But, starting to look nice.
