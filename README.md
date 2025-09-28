# WMATA-MAP
IN PROGRESS

Project to display live WMATA Metro Positions on physical map.
Rust backend calls WMATA API and maps each metro car to "percent complete" on line. On an individually-programmable LED strip, Led # [(percent complete) * (total LEDS)] is lit to show the position of a metro car. More dense LED strip = cooler looking probably.   

"wmata_map" contains Rust project. Getting LEDs working is a TODO, among other clean-up tasks.  

"old" contains old code that is bad and I won't ever touch again (TODO delete).
