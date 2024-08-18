#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include "../interface/interface.hpp"
#include "../monitoring/monitoring.hpp"
#include <thread>
#include <chrono>
#include <cstring>
#include <iostream>

class Management {
public:
    static void displayServer(Server &server);
    static void displayClient(Client &client);
    static void checkAndElectClient(Client &client);
    static void tempo();
};

#endif // MANAGEMENT_H