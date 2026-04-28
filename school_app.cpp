#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <cctype>
#include "include/json.hpp"
#include <sstream>


class Device {
public:
    std::string ip;
    std::string imei;
    std::string imsi;
    std::string config;
    std::string nodes;
    std::string prot;

    double x;
    double y;
    double z;

    int port;

    bool active;
    bool should_exit;

    Device()
        : port(0),
          x(0),
          y(0),
          z(0),
          active(false),
          prot("json"),
          should_exit(false) {}

    void print_menu() {
        std::cout << "\n====================================\n";
        std::cout << "IMSI: " << imsi << std::endl;
        std::cout << "ACTIVE: " << (active ? "TRUE" : "FALSE") << std::endl;
        std::cout << "CURRENT LOCATION: (" << x << ", " << y << ", " << z << ")" << std::endl;
        std::cout << "CURRENT PROTOCOL: " << prot << std::endl;
        std::cout << "\nCOMMANDS:\n";
        std::cout << "EXIT\n";
        std::cout << "    terminates application\n\n";
        std::cout << "ACTIVE <0/1 | true/false>\n";
        std::cout << "    changes active state\n\n";
        std::cout << "MOVE [x] [y] [z]\n";
        std::cout << "    changes position vector\n\n";
        std::cout << "PROTOCOL <json/binary>\n";
        std::cout << "    changes data representation method\n";
        std::cout << "====================================\n";
    }

    std::string get_command_to_lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        return s;
    }
    bool is_command(std::string s, std::stringstream& ss) {
        bool flag = true;
        if (s == "exit") {
            this->should_exit = true;
        }
        else if (s == "active") {
            std::string value;
            ss >> value;
            value = get_command_to_lower(value);
            if (value == "1" or value == "true") {
                this->active = true;
            }
            else if (value == "0" or value == "false") {
                this->active = false;
            }
            else {
                std::cout << "ACTIVE expects: 1, 0, true or false\n";
            }
        }
        else if (s == "protocol") {
            std::string variation;
            ss >> variation;
            variation = get_command_to_lower(variation);
            if (variation == "json" or variation == "binary") {
                this->prot = variation;
            } else {
                std::cout << "INCORRECT PROTOCOL\n";
            }
        }
        else if (s == "move") {
            double val;
            int count = 0;
            double coords[3] = {this->x, this->y, this->z};
            while (count < 3 and ss >> val) {
                coords[count] = val;
                count++;
            }
            if (count > 0) {
                this->x = coords[0];
                this->y = coords[1];
                this->z = coords[2];
                std::cout << "Moved using " << count << " argument(s)\n";
            } else {
                std::cout << "MOVE needs at least one coordinate\n";
            }
        }
        else {
            flag = false;
            std::cout << "INCORRECT INPUT\n";
        }
        return flag;
    }
};

int main(int argc, char* argv[]) {
    using json = nlohmann::json;
    if (argc < 2) {
        std::cerr << "Usage: app <json_file>\n";
        return 1;
    }
    std::ifstream f(argv[1]);
    if (!f.is_open()) {
        std::cerr << "Could not open json file: " << argv[1] << std::endl;
        return 1;
    }
    json data;
    try {
        data = json::parse(f);
    }
    catch (json::parse_error& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return 1;
    }
    Device device;
    if (data.contains("ip")) device.ip = data["ip"];
    if (data.contains("port")) device.port = data["port"];
    if (data.contains("imei")) device.imei = data["imei"];
    if (data.contains("imsi")) device.imsi = data["imsi"];
    if (data.contains("location") && data["location"].is_array()) {
        device.x = data["location"][0];
        device.y = data["location"][1];
        device.z = data["location"][2];
    }
    for (int i = 2; i < argc; i++) {
        std::string arg = argv[i];
        if ((arg == "-a" or arg == "--ip") and i + 1 < argc) {
            device.ip = argv[++i];
        }
        else if ((arg == "-p" or arg == "--port") and i + 1 < argc) {
            device.port = std::stoi(argv[++i]);
        }
        else if ((arg == "-e" or arg == "--imei") and i + 1 < argc) {
            device.imei = argv[++i];
        }
        else if ((arg == "-i" or arg == "--imsi") and i + 1 < argc) {
            device.imsi = argv[++i];
        }
        else if ((arg == "-k" or arg == "--config") and i + 1 < argc) {
            device.config = argv[++i];
        }
        else if ((arg == "-n" or arg == "--nodes") and i + 1 < argc) {
            device.nodes = argv[++i];
        }
        else if ((arg == "-l" or arg == "--loc") and i + 3 < argc) {
            device.x = std::stod(argv[++i]);
            device.y = std::stod(argv[++i]);
            device.z = std::stod(argv[++i]);
        }
        else {
            std::cout << "Unknown argument: " << arg << std::endl;
        }
    }
    std::string line;
    while (!device.should_exit) {
        device.print_menu();
        std::cout << "> ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (line.empty()) {
            continue;
        }
        std::stringstream ss(line);
        std::string command;
        ss >> command;
        command = device.get_command_to_lower(command);
        device.is_command(command, ss);
    }

    return 0;
}