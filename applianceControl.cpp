#include <iostream>
#include <unistd.h>
#include <cstring>
#include "MQTTClient.h"
#include "GPIO.h"

using namespace std;

#define ADDRESS     "tcp://io.adafruit.com:1883"
#define CLIENTID    "Beagle2"
#define TOPIC       "cjnarey/feeds/project.appliance-control"
#define AUTHMETHOD  "cjnarey"
#define AUTHTOKEN   "aio_bGdD54JVkSfEthuYITMBFZ9gjpf4"
#define QOS         1
#define TIMEOUT     10000L
#define LED_GPIO_NUMBER 60

// Initialize LED GPIO
exploringBB::GPIO ledGPIO(LED_GPIO_NUMBER);

// Function to control the LED GPIO
void controlLED(exploringBB::GPIO& ledGPIO, bool state) {
    if (state) {
        ledGPIO.setValue(exploringBB::HIGH);
    } else {
        ledGPIO.setValue(exploringBB::LOW);
    }
}

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("   message: ");
    payloadptr = (char*) message->payload;

    bool applianceState = (strcmp(payloadptr, "1") == 0);

    // Control the appliance based on the MQTT payload
    controlLED(ledGPIO, applianceState);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main(int argc, char* argv[]) {
    ledGPIO.setDirection(exploringBB::OUTPUT);

    // MQTT configurations
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;

    // Initialize MQTT client
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    // Connect to MQTT broker
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }

    // Subscribe to the MQTT topic
    MQTTClient_subscribe(client, TOPIC, QOS);

    // Main loop
    while (1) {
        // Add a delay to avoid excessive polling
        usleep(500000); // 500ms
    }

    // Disconnect and clean up
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);

    return rc;
}

