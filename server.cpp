#include <iostream>
#include <winsock2.h>
#include <vector>
#include <string>
#include <thread>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

struct User {
    std::string username;
    std::string password;
    std::string name;
    int age;
};

struct Location {
    int id;
    std::string username;
    std::string locationName;
    std::string description;
};

struct AdminLocation {
    int id;
    std::string locationName;
    std::string description;
};

struct SharedLocation {
    int locationId;
    std::string sharedBy;
    std::string sharedWith;
};

struct Notification {
    std::string username;
    int locationId;
    std::string sharedBy;
};

std::vector<User> users;
std::vector<Location> locations;
std::vector<AdminLocation> adminLocations;
std::vector<SharedLocation> sharedLocations;
std::vector<Notification> notifications;

int nextLocationId = 1;

void initializeAdminLocations() {
    adminLocations.push_back({ 1, "Cafe", "A place to enjoy coffee and meet friends." });
    adminLocations.push_back({ 2, "Restaurant", "A place to eat and enjoy delicious food." });
    adminLocations.push_back({ 3, "Cinema", "A place to watch the latest movies." });
    adminLocations.push_back({ 4, "School", "A place for education and learning." });
    adminLocations.push_back({ 5, "Clothing Store", "A place to shop for clothes and fashion accessories." });
}

std::string registerUser(const std::string& username, const std::string& password, const std::string& name, int age) {
    for (const auto& user : users) {
        if (user.username == username) {
            return "Username already exists.\n";
        }
    }
    users.push_back({ username, password, name, age });
    return "User registered successfully.\n";
}

std::string loginUser(const std::string& username, const std::string& password) {
    for (const auto& user : users) {
        if (user.username == username && user.password == password) {
            return "Login successfully.\nSHOW_MENU";
        }
    }
    return "Invalid username or password.\n";
}

std::string addLocation(const std::string& username, const std::string& locationName, const std::string& description) {
    locations.push_back({ nextLocationId++, username, locationName, description });
    return "Location added successfully.\n";
}

std::string updateLocation(int id, const std::string& username, const std::string& locationName, const std::string& description) {
    for (auto& location : locations) {
        if (location.id == id && location.username == username) {
            location.locationName = locationName;
            location.description = description;
            return "Location updated successfully.\n";
        }
    }
    return "Location not found or you do not have permission to update it.\n";
}

std::string deleteLocation(int id, const std::string& username) {
    for (auto it = locations.begin(); it != locations.end(); ++it) {
        if (it->id == id && it->username == username) {
            locations.erase(it);
            return "Location deleted successfully.\n";
        }
    }
    return "Location not found or you do not have permission to delete it.\n";
}

std::string listUserLocations(const std::string& username) {
    std::string result;
    for (const auto& location : locations) {
        if (location.username == username) {
            result += "ID: " + std::to_string(location.id) + "\n";
            result += "Location: " + location.locationName + "\n";
            result += "Description: " + location.description + "\n";
            result += "Added by: " + location.username + "\n\n";
        }
    }
    if (result.empty()) {
        return "No locations available.\n";
    }
    return result;
}

std::string listAdminLocations() {
    if (adminLocations.empty()) {
        return "No admin locations available.\n";
    }

    std::string result;
    for (const auto& location : adminLocations) {
        result += "ID: " + std::to_string(location.id) + "\n";
        result += "Location: " + location.locationName + "\n";
        result += "Description: " + location.description + "\n\n";
    }
    return result;
}

std::string shareLocation(int locationId, const std::string& sharedBy, const std::string& sharedWith) {
    for (const auto& location : locations) {
        if (location.id == locationId && location.username == sharedBy) {
            sharedLocations.push_back({ locationId, sharedBy, sharedWith });
            notifications.push_back({ sharedWith, locationId, sharedBy });
            return "Location shared successfully.\n";
        }
    }
    return "Location not found or you do not have permission to share it.\n";
}

std::string getNotifications(const std::string& username) {
    std::string result;
    for (const auto& notification : notifications) {
        if (notification.username == username) {
            for (const auto& location : locations) {
                if (location.id == notification.locationId) {
                    result += "ID: " + std::to_string(location.id) + "\n";
                    result += "Location: " + location.locationName + "\n";
                    result += "Description: " + location.description + "\n";
                    result += "Shared by: " + notification.sharedBy + "\n\n";
                }
            }
        }
    }
    if (result.empty()) {
        return "No notifications.\n";
    }
    return result;
}

void processClientRequest(SOCKET clientSocket) {
    char buffer[512];
    std::string currentUser;

    while (true) {
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0) {
            std::cerr << "Connection closed or error receiving data." << std::endl;
            break;
        }

        buffer[bytesReceived] = '\0';
        std::string request(buffer);
        std::string response;

        std::cout << "Received request: " << request << std::endl;

        std::istringstream ss(request);
        std::string command;
        ss >> command;

        if (command == "REGISTER") {
            std::string username, password, name;
            int age;
            ss >> username >> password >> name >> age;
            response = registerUser(username, password, name, age);
        }
        else if (command == "LOGIN") {
            std::string username, password;
            ss >> username >> password;
            response = loginUser(username, password);
            if (response.find("Login successfully") != std::string::npos) {
                currentUser = username;
            }
        }
        else if (command == "ADD_LOCATION") {
            if (currentUser.empty()) {
                response = "You must be logged in to add a location.\n";
            }
            else {
                std::string locationName, description;
                ss >> locationName;
                std::getline(ss, description);
                response = addLocation(currentUser, locationName, description);
            }
        }
        else if (command == "LIST_LOCATIONS") {
            if (currentUser.empty()) {
                response = "You must be logged in to list locations.\n";
            }
            else {
                response = listUserLocations(currentUser);
            }
        }
        else if (command == "LIST_ADMIN_LOCATIONS") {
            response = listAdminLocations();
        }
        else if (command == "UPDATE_LOCATION") {
            if (currentUser.empty()) {
                response = "You must be logged in to update a location.\n";
            }
            else {
                int id;
                std::string locationName, description;
                ss >> id >> locationName;
                std::getline(ss, description);
                response = updateLocation(id, currentUser, locationName, description);
            }
        }
        else if (command == "DELETE_LOCATION") {
            if (currentUser.empty()) {
                response = "You must be logged in to delete a location.\n";
            }
            else {
                int id;
                ss >> id;
                response = deleteLocation(id, currentUser);
            }
        }
        else if (command == "SHARE_LOCATION") {
            if (currentUser.empty()) {
                response = "You must be logged in to share a location.\n";
            }
            else {
                int locationId;
                std::string sharedWith;
                ss >> locationId >> sharedWith;
                response = shareLocation(locationId, currentUser, sharedWith);
            }
        }
        else if (command == "GET_NOTIFICATIONS") {
            if (currentUser.empty()) {
                response = "You must be logged in to get notifications.\n";
            }
            else {
                response = getNotifications(currentUser);
            }
        }
        else {
            response = "Invalid command.";
        }

        send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
    }

    closesocket(clientSocket);
}

void clientHandler(SOCKET clientSocket) {
    std::thread(processClientRequest, clientSocket).detach();
}

int main() {
    initializeAdminLocations();

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed.\n";
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Error creating socket.\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(54000);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 54000...\n";

    while (true) {
        SOCKET clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Accept failed.\n";
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        clientHandler(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}