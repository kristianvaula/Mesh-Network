//
// Created by HansMagne Asheim on 16/05/2023.
//
#include <unordered_map>

#include "Drone.cpp"

class DroneList {
private:
    std::unordered_map<int, Drone> drones;

public:
    DroneList() {}

    void addDrone(const Drone& drone) {
        drones.insert(std::make_pair(drone.getDroneId(), drone));
    }

    void addDrone(const DroneData& droneData) {
        Drone drone(droneData);
        drones.insert(std::make_pair(drone.getDroneId(), drone));
    }

    void editDrone(const Drone& drone) {
        if (drones.count(drone.getDroneId()) > 0) {
            drones.insert(std::make_pair(drone.getDroneId(), drone));
        }
    }

    int getSize() {
        return drones.size();
    }

    std::unordered_map<int, Drone> getDronesWithPriority(Priority priority) const {
        std::unordered_map<int, Drone> priorityDrones;

        for (const auto& entry : drones) {
            int droneId = entry.first;
            const Drone& drone = entry.second;

            if (drone.getPriority() == priority) {
                priorityDrones.insert(std::make_pair(drone.getDroneId(), drone));
            }
        }

        return priorityDrones;
    }

    bool isDroneInMesh() const {
        return drones.size() > getDronesWithPriority(Priority::NONE).size();
    }
};
