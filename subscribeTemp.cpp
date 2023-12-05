#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MQTTClient.h"
#include "analogIn.h"
#include "makeLEDs.h"
using namespace std;

#define ADDRESS     "tcp://io.adafruit.com:1883"
#define CLIENTID    "Beagle2"
#define TOPIC       "cjnarey/feeds/project.temperature"
#define AUTHMETHOD  "cjnarey"
#define AUTHTOKEN   "aio_bGdD54JVkSfEthuYITMBFZ9gjpf4"
#define QOS         1
#define TIMEOUT     10000L
#define LED_GPIO    "/sys/class/gpio/gpio60/"
#define THRESHOLD   30

// Use this function to control the LED
void writeGPIO(string filename, string value){
   fstream fs;
   string path(LED_GPIO);
   fs.open((path + filename).c_str(), fstream::out);
   fs << value;
   fs.close();
}

volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payloadptr = static_cast<char*>(message->payload);
    std::string messageStr(payloadptr, payloadptr + message->payloadlen);

    // Temperature functionality
    float temperature = std::atof(messageStr.c_str());
    std::cout << "Received temperature: " << temperature << std::endl;
    static AnalogIn analogSensor(0);
    int adcValue = analogSensor.readAdcSample();
    static LED usr0LED(0);
    if (temperature < THRESHOLD) {
        std::cout << "Turning on LED because temperature is below threshold." << std::endl;
        usr0LED.turnOn();
    } else {
        std::cout << "Temperature is above threshold. LED remains off." << std::endl;
    }

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}


void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;
    writeGPIO("direction", "out");

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);

    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

