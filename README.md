# dvb_tdt
TDT / TOT Table-Generator for TSDUCK

Start when stream is running. Add the following line to your tsduck script

-P datainject -r -s localhost:32000 -b 50000 -p 0x14

Usage: "tdt.x86xx port" add the same port as in the line above. Don't use the same port as for emmg. 
