
An example of action code written in C (incomplete : work in progress).<br/>
--
Author : Roqyun KO <br/>

Node JS script provided is translated in C language.
A simple regex parser with limited functionality is written as well for this project.<br/>

--<br/>
This project uses the existing open source C library of mosquitto for MQTT Broker.<br/>
The current version of the library is 1.4.8<br/>

Eclipse Mosquitto<br/>
Git : https://github.com/eclipse/mosquitto<br/>
Site : https://mosquitto.org<br/>

--<br/>
This project uses the existing open source C library of json for parsing json.<br/>
The current version of the library is 1.1.0<br/>

json-parser<br/>
Git : https://github.com/udp/json-parser<br/>

Note : -lm flag has been manually added in the json-parser Makefile becasue of an ambiguous call towards 'pow' function<br/>
       Executed : sudo ldconfig -v (to fix the unrecognized shared library problem.)
