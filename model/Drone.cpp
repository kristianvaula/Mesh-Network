//
// Created by HansMagne Asheim on 16/05/2023.
//
#include "enums/Priority.cpp"

class Drone {
private:
    int droneId;
    int port;
    int ipAddress;
    int xPosition;
    int yPosition;
    Priority priority;

public:
    Drone(int droneId, int port, int ipAddress)
        : droneId(droneId), port(port), ipAddress(ipAddress), xPosition(0), yPosition(0), priority(Priority::NONE) {}

    Drone(int droneId, int port, int ipAddress, int xPosition, int yPosition, Priority priority)
        : droneId(droneId), port(port), ipAddress(ipAddress), xPosition(xPosition), yPosition(yPosition),
        priority(priority) {}

    Drone(const Drone& other)
        : droneId(other.droneId), port(other.port), ipAddress(other.ipAddress), xPosition(other.xPosition),
        yPosition(other.yPosition), priority(other.priority) {}

    Drone(const DroneData& droneData) : droneId(droneData.droneId), port(droneData.port) {}

    int getDroneId() const {
        return droneId;
    }

    int getPort() const {
        return port;
    }

    int getIpAddress() const {
        return ipAddress;
    }

    int getXPosition() const {
        return xPosition;
    }

    void setXPosition(int x) {
        xPosition = x;
    }

    int getYPosition() const {
        return yPosition;
    }

    void setYPosition(int y) {
        yPosition = y;
    }

    Priority getPriority() const {
        return priority;
    }

    void setPriority(Priority p) {
        priority = p;
    }
};