var mqtt = require('mqtt');

var hostname = "mqtt://localhost";
var client  = mqtt.connect(hostname);

client.on('connect', function () {
    console.log("[Snips Log] Connected to MQTT broker " + hostname);
    client.subscribe('hermes/#');
});

client.on('message', function (topic, message) {
    if (topic === "hermes/asr/startListening") {
        onListeningStateChanged(true);
    } else if (topic === "hermes/asr/stopListening") {
        onListeningStateChanged(false);
    } else if (topic.match(/hermes\/hotword\/.+\/detected/g) !== null) {
        onHotwordDetected()
    } else if (topic.match(/hermes\/intent\/.+/g) !== null) {

	let intent  = JSON.parse(message);
    	onIntentDetected(intent);
	client.publish('hermes/dialogueManager/endSession', '{\"sessionId\":\"' + intent.sessionId + '\",\"text\":\"la gauche\"}');
 
    }
});

function onIntentDetected(intent) {
    console.log("[Snips Log] Intent detected: " + JSON.stringify(intent));
    console.log("\"text\":\"la gauche\",\"lang\":\"fr\",\"siteId\":\"default\"");
    console.log("{\"sessionId\":" + intent.sessionId + ",\"text\":\"la gauche\"}");
   //cmd.run('mosquitto_pub -h localhost -p 1883 -t hermes/tts/say -m "{\"text\":\"speak to me\",\"lang\":\"fr\",\"siteId\":\"default\"}"');
	
}

function onHotwordDetected() {
    console.log("[Snips Log] Hotword detected");
}

function onListeningStateChanged(listening) {
    console.log("[Snips Log] " + (listening ? "Start" : "Stop") + " listening");
}
