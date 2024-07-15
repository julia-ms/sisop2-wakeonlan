#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <fstream>

#include "./discovery.hpp"

#define BUFFER_SIZE 256

using namespace std;

// int Client::getHostname(char *buffer, size_t bufferSize, DiscoveredData &data)
// {
//     memset(buffer, 0, sizeof(buffer));

//     if ((gethostname(buffer, bufferSize)) == -1)
//     {
//         cout << "ERROR on getting the hostname." << endl;
//         return -1;
//     }

//     strncpy(data.hostname, buffer, MAX_HOSTNAME_SIZE - 1);
//     data.hostname[MAX_HOSTNAME_SIZE - 1] = '/0';

//     memset(buffer, 0, sizeof(buffer));

//     return 0;
// }

// int Client::getIpAddress(DiscoveredData &data)
// {
//     struct ifaddrs *netInterfaces, *tempInterface = NULL;

//     if (!getifaddrs(&netInterfaces))
//     {
//         tempInterface = netInterfaces;

//         while (tempInterface != NULL)
//         {
//             if (tempInterface->ifa_addr->sa_family == AF_INET)
//             {
//                 if (strcmp(tempInterface->ifa_name, "eth0") == 0)
//                 {
//                     strncpy(data.ipAddress, inet_ntoa(((struct sockaddr_in *)tempInterface->ifa_addr)->sin_addr), IP_ADDRESS_SIZE - 1);
//                     data.ipAddress[IP_ADDRESS_SIZE - 1] = '\0';
//                 }
//             }

//             tempInterface = tempInterface->ifa_next;
//         }

//         freeifaddrs(netInterfaces);
//     }
//     else
//     {
//         cout << "ERROR on getting IP Adress." << endl;
//         return -1;
//     }

//     return 0;
// }

// int Client::getMacAddress(int sockfd, char *macAddress, size_t size)
// {
//     struct ifreq ifr;

//     strcpy(ifr.ifr_name, "eth0");

//     if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) < 0)
//     {
//         cerr << "ERROR on getting Mac Address." << endl;
//         close(sockfd);
//         return -1;
//     }

//     unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
//     snprintf(macAddress, size, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

//     return 0;
// }

// int Client::getStatus(Status &status)
// {
//     FILE *fp = popen("systemctl is-active systemd-timesyncd.service", "r");
//     // FILE* fp = popen("service systemd-timesyncd status", "r");
//     if (!fp)
//     {
//         std::cerr << "Failed to open power status file." << std::endl;
//         return -1;
//     }

//     char result[10];
//     if (fgets(result, sizeof(result), fp))
//     {
//         pclose(fp);
//         // Check if the service is active
//         if (std::string(result).find("active") != std::string::npos)
//         {
//             status = Status::AWAKEN;
//         }
//         else
//         {
//             status = Status::ASLEEP;
//         }

//         return 0;
//     }
//     else
//     {
//         std::cerr << "Failed to read command output." << std::endl;
//         pclose(fp);
//         return -1;
//     }

//     // não fazendo nada, só ficando com o valor original
//     return 0;
// }

int Client::sendSocket(int argc)
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        cerr << "ERROR opening socket." << endl;

    const int optval{1};
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval)) < 0) {
        throw std::runtime_error("Failed to set socket options");
    }

    struct sockaddr_in participant_addr;
    participant_addr.sin_family = AF_INET;
    participant_addr.sin_port = htons(PORT_S);
    participant_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Bind ao endereço local
    bzero(&(participant_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *)&participant_addr, sizeof(struct sockaddr)) < 0) {
        cerr << "ERROR on binding socket." << endl;
        return 1;  // Certifique-se de sair ou tratar o erro apropriadamente
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT_S);
    serv_addr.sin_addr.s_addr = inet_addr(BROADCAST_ADDR);
    bzero(&(serv_addr.sin_zero), 8);

    char buffer[BUFFER_SIZE];

    DiscoveredData pcData;
    getHostname(buffer, BUFFER_SIZE, pcData);
    getIpAddress(pcData);
    getMacAddress(sockfd, pcData.macAddress, MAC_ADDRESS_SIZE);
    // getStatus(pcData.status);
    pcData.status = Status::AWAKEN;

    // cout << "Hostname: " << pcData.hostname << endl;
    // cout << "IP Address: " << pcData.ipAddress << endl;
    // cout << "Mac Address: " << pcData.macAddress << endl;
    // cout << "Status: " << ((pcData.status == 1) ? "AWAKEN" : "ASLEEP") << endl;

    /** Uncomment if necessary **/
    // printf("Enter the message: ");
    // bzero(buffer, 256);
    // fgets(buffer, 256, stdin);

    if (sendto(sockfd, &pcData, sizeof(pcData), 0, (const struct sockaddr *)&serv_addr, sizeof(struct sockaddr_in)) < 0)
        cerr << "ERROR on sendto." << endl;

    struct sockaddr_in from;
    unsigned int length = sizeof(struct sockaddr_in);
    memset(buffer, 0, sizeof(buffer));

    memset(&managerInfo, 0, sizeof(managerInfo));
    ssize_t k = recvfrom(sockfd, &managerInfo, sizeof(managerInfo), 0, (struct sockaddr *)&from, &length);

    if (recvfrom(sockfd, &managerInfo, sizeof(managerInfo), 0, (struct sockaddr *)&from, &length) < 0)
        cerr << "ERROR on recvfrom." << endl;

    cout << "Hostname: " << managerInfo.hostname << endl;
    cout << "IP Address: " << managerInfo.ipAddress << endl;
    cout << "Mac Address: " << managerInfo.macAddress << endl;

    // cout << "Got an ack: " << buffer << endl;

    close(sockfd);
    return 0;
}

// MONITORAMENTO
void Client::waitForRequests()
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        cerr << "ERROR opening socket." << endl;
        return;
    }

    //printf("esperando por requests\n");

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(client_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) < 0)
    {
        cerr << "ERROR on binding socket." << endl;
        close(sockfd);
        return;
    }

    while (true)
    {
        RequestData request;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t bytesReceived = recvfrom(sockfd, &request, sizeof(request), 0, (struct sockaddr *)&from, &fromlen);
        if (bytesReceived < 0)
        {
            cerr << "ERROR on recvfrom." << endl;
            continue;
        }

        // printf("recebi algo\n");

        if (request.request == Request::SLEEP_STATUS)
        {
            // Status status;
            // getStatus(status);
            // retornar o status
            Status status = Status::AWAKEN;
            sendto(sockfd, &status, sizeof(status), 0, (struct sockaddr *)&from, fromlen);
        }
    }

    close(sockfd);
}

int Client::sendExitRequest(const char *ipAddress)
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        cerr << "ERROR opening socket." << endl;
        return -1;
    }

    // Definir tempo limite de 5 segundos para recebimento
    struct timeval tv;
    tv.tv_sec = 3;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv)) < 0) {
        cerr << "ERROR setting socket timeout." << endl;
        close(sockfd);
        return -1;
    }

    struct sockaddr_in recipient_addr;
    memset(&recipient_addr, 0, sizeof(recipient_addr));
    recipient_addr.sin_family = AF_INET;
    recipient_addr.sin_port = htons(PORT_E);
    if (inet_pton(AF_INET, ipAddress, &recipient_addr.sin_addr) <= 0) {
        cerr << "ERROR invalid address/ Address not supported." << endl;
        close(sockfd);
        return -1;
    }

    RequestData req;
    req.request = Request::EXIT;

    if (sendto(sockfd, &req, sizeof(req), 0, (struct sockaddr *)&recipient_addr, sizeof(recipient_addr)) < 0) {
        cerr << "ERROR sending request." << endl;
        close(sockfd);
        return -1;
    }
    
    close(sockfd);
    return 0;
}

void Client::waitForParticipantDataRequests()
{
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        cerr << "ERROR opening socket." << endl;
        return;
    }

    //printf("esperando por requests\n");

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT_PD);
    client_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(client_addr.sin_zero), 8);

    if (bind(sockfd, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)) < 0)
    {
        cerr << "ERROR on binding socket." << endl;
        close(sockfd);
        return;
    }

    while (true)
    {
        RequestData request;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);
        ssize_t bytesReceived = recvfrom(sockfd, &request, sizeof(request), 0, (struct sockaddr *)&from, &fromlen);
        if (bytesReceived < 0)
        {
            cerr << "ERROR on recvfrom." << endl;
            continue;
        }

        // printf("recebi algo\n");

        if (request.request == Request::PARTICIPANT_DATA)
        {
            // Status status;
            // getStatus(status);
            // retornar o status
            char buffer[BUFFER_SIZE];

            DiscoveredData pcData;
            getHostname(buffer, BUFFER_SIZE, pcData);
            getIpAddress(pcData);
            getMacAddress(sockfd, pcData.macAddress, MAC_ADDRESS_SIZE);
            // getStatus(pcData.status);
            pcData.status = Status::AWAKEN;

            sendto(sockfd, &pcData, sizeof(pcData), 0, (struct sockaddr *)&from, fromlen);
        }
    }

    close(sockfd);
}