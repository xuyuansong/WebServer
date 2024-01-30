#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

class Logger {
public:
    Logger(const std::string& filename) : m_filename(filename) {
        m_file.open(filename.c_str(), std::ios::out | std::ios::app);
    }

    ~Logger() {
        if (m_file.is_open()) {
            m_file.close();
        }
    }

    void log(const std::string& message) {
        if (m_file.is_open()) {
            time_t now = time(0);
            tm* localTime = localtime(&now);
            m_file << "[" << localTime->tm_year + 1900 << "-"
                   << localTime->tm_mon + 1 << "-"
                   << localTime->tm_mday << " "
                   << localTime->tm_hour << ":"
                   << localTime->tm_min << ":"
                   << localTime->tm_sec << "] "
                   << message << std::endl;
        }
    }

private:
    std::string m_filename;
    std::ofstream m_file;
};

#endif 