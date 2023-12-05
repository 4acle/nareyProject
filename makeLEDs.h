#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

class LED {
private:
    std::string path;
    int number;

    void writeLED(std::string filename, std::string value);
    void removeTrigger();

public:
    LED(int number);
    void turnOn();
    void turnOff();
    void flash(std::string delayms);
    void outputState();
    void blink(int count, std::string delayms);
    ~LED();
};

