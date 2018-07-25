#pragma once

#include <algorithm>
#include <functional>
#include <cctype>
#include <cstring>
#include <locale>
#include <sstream>
#include <string>
#include <vector>

namespace strutils
{
  static bool replace(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
      return false;
    str.replace(start_pos, from.length(), to);
    return true;
  }

  static bool replaceLast(std::string& str, const std::string& from, const std::string& to) {
    size_t start_pos = str.rfind(from);
    if (start_pos == std::string::npos)
      return false;
    str.replace(start_pos, from.length(), to);
    return true;
  }

  static bool startsWith(const std::string &str, const std::string &prefix)
  {
    return str.size() >= prefix.size() &&
      str.compare(0, prefix.size(), prefix) == 0;
  }

  static bool endsWith(const std::string &str, const std::string &suffix)
  {
    return str.size() >= suffix.size() &&
      str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
  }

  template<typename T>
  static std::string toString(T value)
  {
    std::ostringstream ss;
    ss << value;
    return ss.str();
  }

  template<typename T>
  static T parse(const std::string &str)
  {
    T result;
    std::istringstream(str) >> result;

    return result;
  }

  template<typename T>
  static T parse(const std::string &str, bool adavanced)
  {
    // By default the advanced mode is disabled for all datatypes
    return parse<T>(str);
  }

  template<typename T> inline
    static std::vector<T> parseArray(const std::string &str)
  {
    std::vector<T> elems;
    std::istringstream f(str);
    std::string s;
    while (std::getline(f, s, ':'))
      elems.push_back(parse<T>(s));

    return elems;
  }

  static void toUpper(char* s)
  {
    for (size_t i = 0; s[i]; i++)
      s[i] = toupper(s[i]);
  }

  static void toUpper(std::string &str)
  {
    for (size_t i = 0; str[i]; i++)
      str[i] = toupper(str[i]);
  }

  static void toLower(char* s)
  {
    for (size_t i = 0; s[i]; i++)
      s[i] = tolower(s[i]);
  }

  static void toLower(std::string &str)
  {
    for (size_t i = 0; str[i]; i++)
      str[i] = tolower(str[i]);
  }

  static std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
  }

  static std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
  }

  static std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
  }

  template<typename T>
  static std::string join(const std::vector<T> &v, const std::string &token) {
    std::ostringstream result;
    for (typename std::vector<T>::const_iterator i = v.begin(); i != v.end(); i++) {
      if (i != v.begin())
        result << token;
      result << *i;
    }

    return result.str();
  }

  static std::vector<std::string> split(const std::string &str, char delim)
  {
    std::vector<std::string> elems;
    std::stringstream ss(str);

    std::string item;
    while (getline(ss, item, delim))
      elems.push_back(item);

    return elems;
  }

  template<> inline
    std::string parse(const std::string &str)
  {
    return str;
  }

  template<> inline
    const char* parse(const std::string &str)
  {
    return str.c_str();
  }

  template<> inline
    bool parse(const std::string &str, bool advanced)
  {
    std::string s = str;
    toLower(s);

    if (s == "on"
      || s == "yes")
      return true;

    return parse<bool>(str);
  }
}