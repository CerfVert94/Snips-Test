#include <stdio.h>
#include <string.h>
#include <mosquitto.h>
#include <json.h>
#include "client_shared.h"
#include "topic-regex.h"
//static char *topic = NULL;
//static char *message = NULL;
#define STATUS_CONNECTING 0
#define STATUS_CONNACK_RECVD 1
#define STATUS_WAITING 2
//static char *message = NULL;
//static long msglen = 0;
//static int qos = 0;
//static int retain = 0;
static int mode = MSGMODE_NONE;
//static int status = STATUS_CONNECTING;
//static int mid_sent = 0;
static int last_mid = -1;
static int last_mid_sent = -1;
//static bool connected = true;
//static char *username = NULL;
//static char *password = NULL;
static bool disconnect_sent = false;
//static bool quiet = false;
#define _LOG(str) "[Snips log] "str"\n"
#define _TOPIC_ASR(str) "hermes/asr"str
#define _TOPIC_HOTWORD(str) "hermes/hotword"str
#define _TOPIC_INTENT(str) "hermes/intent"

char payload[128] =  "{\"sessionId\":\"default\",\"text\":\"la gauche\"}";
char text[128];
char sessionId[128];
json_char *json;
json_value *value;
int mid = 100;
void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
	if(message->payloadlen) {
		if (!strcmp(message->topic, "hermes/asr/startListening")) {
			fprintf(stderr, _LOG("Start listening"));
		}
		else if (!strcmp(message->topic, "hermes/asr/stopListening")) {
			fprintf(stderr, _LOG("Stop listening"));	
		}
		else if (match(message->topic, "/hermes\\/hotword/.+/detected")) {
			fprintf(stderr, _LOG("Hotword detected"));
		}
		if(match(message->topic, "/hermes\\/intent/.+")) {
			fprintf(stderr, _LOG("Intent detected"));
			fprintf(stderr, "%s\n",(char*) message->payload);
			json = (json_char*) message->payload;	
			value = json_parse(json, message->payloadlen);
			if( !value) {
				fprintf(stderr, "Failure : parsing json\n");	

			}
			sprintf(payload, "{\"sessionId\":\"%s\",\"text\":\"%s\"}", \
				(char*)value->u.object.values[0].value->u.string.ptr, \
				"j'allume la gauche");	
//			mosquitto_publish(mosq, &mid, "hermes/dialogueManager/endSession", message->payloadlen, (char*)payload,2, false);
			mosquitto_publish(mosq, &mid, "hermes/dialogueManager/endSession", strlen(payload), (char*)payload,2, false);
			last_mid = mid;
				
			printf("%d, %s\n", mid, payload);	
			json_value_free(value);
		}
		
	}else{
	}
}

void my_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
//	int i;
	if(!result){
		fprintf(stdout, _LOG("Connected to MQTT broker"));
		/* Subscribe to broker information topics on successful connect. */
		mosquitto_subscribe(mosq, NULL, "hermes/#", 2);
	}
	else{
		fprintf(stderr, _LOG("Connect to MQTT broker failed"));
	}
}

void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
{
	int i;

	//printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
	for(i=1; i<qos_count; i++){
		//printf(", %d", granted_qos[i]);
	}
//	printf("\n");
}

void my_publish_callback(struct mosquitto *mosq, void *obj, int mid)
{
	last_mid_sent = mid;
	fprintf(stderr, _LOG("Published : %d"), mid);
	if(mode == MSGMODE_STDIN_LINE){
		if(mid == last_mid){
			mosquitto_disconnect(mosq);
			disconnect_sent = true;
		}
	}else if(disconnect_sent == false){
		mosquitto_disconnect(mosq);
		disconnect_sent = true;
	}
}

void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str)
{
	/* Pring all log messages regardless of level. */
	//printf("%s\n", str);
}

int main(int argc, char *argv[])
{
	//int i;
	char *host = "localhost";
	int port = 1883;
	int keepalive = 60;
	bool clean_session = true;
	struct mosquitto *mosq = NULL;

	mosquitto_lib_init();
	mosq = mosquitto_new(NULL, clean_session, NULL);
	if(!mosq){
		fprintf(stderr, "Error: Out of memory.\n");
		return 1;
	}
	mosquitto_log_callback_set(mosq, my_log_callback);
	mosquitto_connect_callback_set(mosq, my_connect_callback);
	mosquitto_message_callback_set(mosq, my_message_callback);
	mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
	mosquitto_publish_callback_set(mosq, my_publish_callback);


	if(mosquitto_connect(mosq, host, port, keepalive)){
		fprintf(stderr, "Unable to connect.\n");
		return 1;
	}

	mosquitto_loop_forever(mosq, -1, 1);

	mosquitto_destroy(mosq);
	mosquitto_lib_cleanup();
	return 0;
}
