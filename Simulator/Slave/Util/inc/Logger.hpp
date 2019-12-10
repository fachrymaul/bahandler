#ifndef LOGGER_H
#define LOGGER_H

#include <cstdint>
#include <string>
#include <iostream>

class Logger
{
  public:
    Logger(uint8_t level);

    static void setDebug(bool enableDebug);
    Logger &operator<<(const char c);
    Logger &operator<<(const char *c);
    Logger &operator<<(const std::string s);
    Logger &operator<<(const int i);
    Logger &operator<<(const unsigned int i);
    Logger &operator<<(const long l);
    Logger &operator<<(const unsigned long lu);
    Logger &operator<<(const float f);
    Logger &operator<<(const double d);

  private:
    void startLine();
    void endLine();

    template <class T>
    void print(T t);

  private:
    uint8_t m_level;
    bool m_lineStart;
};

extern Logger debug;
extern Logger console;
extern Logger status;
extern Logger warning;
extern Logger error;
extern Logger severe;

#endif
