#include <Logger.hpp>

static bool s_debug = true;

Logger debug(0);
Logger console(1);
Logger status(2);
Logger warning(3);
Logger error(4);
Logger severe(5);

Logger::Logger(uint8_t level) : m_level(level), m_lineStart(true)
{
}

void Logger::setDebug(bool enableDebug)
{
    s_debug = enableDebug;
}

Logger &Logger::operator<<(const char c)
{
    print(c);
    if (s_debug)
    {
        if (c == '\n' || c == '\r')
            endLine();
    }
    return *this;
}

Logger &Logger::operator<<(const char *c)
{
    print(c);
    if (s_debug)
    {
        if (c[0] == '\r' && c[1] == '\n')
            endLine();
        else if (c[0] == '\r' || c[0] == '\n')
            endLine();
    }
    return *this;
}

Logger &Logger::operator<<(const std::string s)
{
    print(s);
    return *this;
}

Logger &Logger::operator<<(const int i)
{
    print(i);
    return *this;
}

Logger &Logger::operator<<(const unsigned int i)
{
    print((long)i);
    return *this;
}

Logger &Logger::operator<<(const long l)
{
    print(l);
    return *this;
}

Logger &Logger::operator<<(const unsigned long lu)
{
    print(lu);
    return *this;
}

Logger &Logger::operator<<(const float f)
{
    print(f);
    return *this;
}

template <class T>
void Logger::print(T t)
{
    if (s_debug)
    {
        startLine();
        std::cout << t << std::flush;
    }
}

void Logger::startLine()
{
    if (m_lineStart)
    {
        if (m_level > 1)
        {
            if (m_level == 3)
                std::cout << "[WARNING] ";
            else if (m_level > 3)
                std::cout << "[ERROR] ";
        }
    }

    m_lineStart = false;
}

void Logger::endLine()
{
    m_lineStart = true;
}