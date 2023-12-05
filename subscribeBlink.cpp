#include <iostream>
#include <stdio.h>
#include <string.h>
#include "MQTTClient.h"
#include "makeLEDs.h"

using namespace std;

#define ADDRESS      "tcp://io.adafruit.com:1883"
#define CLIENTID     "Beagle2_Blink"
#define TOPIC_BLINK  "cjnarey/feeds/Blink"
#define AUTHMETHOD   "cjnarey"
#define AUTHTOKEN    "aio_bGdD54JVkSfEthuYITMBFZ9gjpf4"
#define QOS          1
#define TIMEOUT      10000L

// Function to handle incoming messages for the Blink feed
int msgarrvdBlink(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payloadptr = (char*) message->payload;
    int numberOfBlinks = atoi(payloadptr);
    printf("Number of Blinks: %d\n", numberOfBlinks);

    static LED usr0LED(0);
    usr0LED.blink(numberOfBlinks, "500");  // Blink with a delay of 500 milliseconds

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;
    MQTTClient_setCallbacks(client, NULL, NULL, msgarrvdBlink, NULL);  // Use the Blink-specific msgarrvd function

    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC_BLINK, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC_BLINK, QOS);

    int ch;
    do {
        ch = getchar();
    } while(ch != 'Q' && ch != 'q');

    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

