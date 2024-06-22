#define NOMINMAX
#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <limits>
#include <sstream>
#pragma comment(lib, "ws2_32.lib")

void sendRequest(SOCKET clientSocket, const std::string& request);

void showMenu(SOCKET clientSocket, const std::string& username) {
    while (true) {
        std::string menuCommand;
        std::cout << "1. Add new location\n";
        std::cout << "2. Show my location\n";
        std::cout << "3. Update my location\n";
        std::cout << "4. Delete my location\n";
        std::cout << "5. Show my suggested locations\n";
        std::cout << "6. Share location to my friend\n";
        std::cout << "7. My notification\n";
        std::cout << "8. Logout\n";
        std::cout << "9. Exit\n";
        std::cout << "Enter your choice: ";
        std::getline(std::cin, menuCommand);

        if (menuCommand == "1") {
            std::string locationName, description;
            std::cout << "Enter location name: ";
            std::getline(std::cin, locationName);
            std::cout << "Enter description: ";
            std::getline(std::cin, description);
            std::string request = "ADD_LOCATION \"" + locationName + "\" \"" + description + "\"";
            sendRequest(clientSocket, request);
        }
        else if (menuCommand == "2") {
            std::string request = "LIST_LOCATIONS";
            sendRequest(clientSocket, request);
        }
        else if (menuCommand == "3") {
            int id;
            std::string locationName, description;
            std::cout << "Enter location ID to update: ";
            std::cin >> id;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Enter new location name: ";
            std::getline(std::cin, locationName);
            std::cout << "Enter new description: ";
            std::getline(std::cin, description);
            std::string request = "UPDATE_LOCATION " + std::to_string(id) + " \"" + locationName + "\" \"" + description + "\"";
            sendRequest(clientSocket, request);
        }
        else if (menuCommand == "4") {
            int id;
            std::cout << "Enter location ID to delete: ";
            std::cin >> id;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::string request = "DELETE_LOCATION " + std::to_string(id);
            sendRequest(clientSocket, request);
        }
        else if (menuCommand == "5") {
            std::string request = "LIST_ADMIN_LOCATIONS";
            sendRequest(clientSocket, request);
        }
        else if (menuCommand == "6") {
            int locationId;
            std::string sharedWith;
            std::cout << "Enter location ID to share: ";
            std::cin >> locationId;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Enter username to share with: ";
            std::getline(std::cin, sharedWith);
            std::string request = "SHARE_LOCATION " + std::to_string(locationId) + " " + sharedWith;
            sendRequest(clientSocket, request);
        }
        else if (menuCommand == "7") {
            std::string request = "GET_NOTIFICATIONS";
            sendRequest(clientSocket, request);
        }
        else if (menuCommand == "8") {
            break;
        }
        else if (menuCommand == "9") {
            closesocket(clientSocket);
            WSACleanup();
            exit(0);
        }
        else {
            std::cout << "Invalid choice. Please enter again.\n";
        }
    }
}

void sendRequest(SOCKET clientSocket, const std::string& request) {
    send(clientSocket, request.c_str(), static_cast<int>(request.length()), 0);

    char buffer[512];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        std::string response(buffer);
        std::cout << "Server response:\n" << response << "\n";
        if (response.find("Hello") != std::string::npos) {
            size_t pos = response.find("Hello") + 6;
            size_t end_pos = response.find(", here are your features:");
            std::string username = response.substr(pos, end_pos - pos);
            showMenu(clientSocket, username);
        }
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed.\n";
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);

    if (inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr) <= 0) {
        std::cerr << "Invalid address / Address not supported.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed.\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    while (true) {
        std::string command;
        std::cout << "Welcome to the user management system\n";
        std::cout << "Select function (REGISTER or LOGIN or EXIT): ";
        std::getline(std::cin, command);

        if (command == "EXIT") {
            break;
        }

        if (command == "REGISTER") {
            std::string username, password, name;
            int age;
            std::cout << "Enter username: ";
            std::getline(std::cin, username);
            std::cout << "Enter password: ";
            std::getline(std::cin, password);
            std::cout << "Enter name: ";
            std::getline(std::cin, name);
            std::cout << "Enter age: ";
            std::cin >> age;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            std::string request = command + " " + username + " " + password + " " + name + " " + std::to_string(age);
            sendRequest(clientSocket, request);
        }
        else if (command == "LOGIN") {
            std::string username, password;
            std::cout << "Enter username: ";
            std::getline(std::cin, username);
            std::cout << "Enter password: ";
            std::getline(std::cin, password);

            std::string request = command + " " + username + " " + password;
            sendRequest(clientSocket, request);
        }
        else {
            std::cout << "Invalid command. Please enter again.\n";
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}