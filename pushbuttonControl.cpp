#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include "MQTTClient.h"
#include "GPIO.h"

using namespace std;

#define ADDRESS     "tcp://io.adafruit.com:1883"
#define CLIENTID    "Beagle2"
#define TOPIC       "cjnarey/feeds/project.status"
#define AUTHMETHOD  "cjnarey"
#define AUTHTOKEN   "aio_bGdD54JVkSfEthuYITMBFZ9gjpf4"
#define QOS         1
#define TIMEOUT     10000L
#define LED_GPIO    "/sys/class/gpio/gpio60/"
#define THRESHOLD   30

// Function to check if the button is pressed
bool isButtonPressed(exploringBB::GPIO &button) {
    return button.getValue() == exploringBB::LOW;
}

// Function to publish status to MQTT
void publishStatus(int status) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    cout << "Publishing status: " << (status ? "true" : "false") << endl;

    // Initialize MQTT client
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    // Connect to MQTT broker
    int rc;
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        cout << "Failed to connect to MQTT, return code " << rc << endl;
        return;
    }


    // Convert numeric status to string
    string statusStr = to_string(status);

    // Construct MQTT message
    pubmsg.payload = const_cast<char*>(statusStr.c_str());
    pubmsg.payloadlen = statusStr.length();
    pubmsg.qos = QOS;
    pubmsg.retained = 0;

    // Publish message to the specified topic
    MQTTClient_publishMessage(client, TOPIC, &pubmsg, &token);
    cout << "Waiting for up to " << (int)(TIMEOUT/1000) << " seconds for publication on topic " << TOPIC << endl;

    // Wait for the publication to complete
    rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
    if (rc == MQTTCLIENT_SUCCESS) {
        cout << "Message with token " << (int)token << " delivered." << endl;
    } else {
        cout << "Publication failed with error code: " << rc << endl;
    }

    // Disconnect from MQTT broker
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
}

int main(int argc, char *argv[]) {
    // Initialize GPIO for LED
    exploringBB::GPIO ledGPIO(60);
    ledGPIO.setDirection(exploringBB::OUTPUT);

    // Initialize GPIO for button
    exploringBB::GPIO buttonGPIO(46);
    buttonGPIO.setDirection(exploringBB::INPUT);
    buttonGPIO.setEdgeType(exploringBB::FALLING); // Detect falling edge (button press)

    while (true) {
        // Check if the button is pressed
        if (isButtonPressed(buttonGPIO)) {
            // Button is pressed, turn on the LED
            ledGPIO.setValue(exploringBB::HIGH);

            // Publish status "true" to MQTT
            publishStatus(1);
	    
	    cout << "Button pressed - publishing status 'true'" << endl;

            // Wait for some time to debounce the button press
            sleep(1);
        } else {
            // Button is not pressed, turn off the LED
            ledGPIO.setValue(exploringBB::LOW);

            // Publish status "false" to MQTT
            publishStatus(0);

	    cout << "Button not pressed - publishing status 'false'" << endl;

            // Wait for some time to debounce the button release
            sleep(1);
        }

    }

    return 0;
}

