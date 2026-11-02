// Имитация Arduino.h для тестов Wi-Fi: String, Print, IPAddress, время вручную.
#pragma once
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <functional>
#include <string>
class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))
inline uint32_t g_ms = 100000;
inline void (*g_onDelay)() = nullptr;                         // вызывается на каждый delay(): тест подставляет "действия пользователя"
inline uint32_t millis() { return g_ms; }
inline void delay(uint32_t ms) { for (uint32_t i = 0; i < ms; i++) { g_ms++; if (g_onDelay) g_onDelay(); } }
#define PROGMEM
#define FPSTR(x) (reinterpret_cast<const __FlashStringHelper *>(x))
#include <vector>
class String {
 public:
  std::string s;
  String() {}
  String(const char *c) : s(c ? c : "") {}
  String(const std::string &x) : s(x) {}
  String(const __FlashStringHelper *f) : s((const char *)f) {}
  String(int v) : s(std::to_string(v)) {}
  void reserve(size_t) {}
  size_t length() const { return s.size(); }
  void toLowerCase() { for (auto &c : s) c = tolower(c); }
  bool endsWith(const char *x) const { size_t n = strlen(x); return s.size() >= n && s.compare(s.size() - n, n, x) == 0; }
  String(char c) : s(1, c) {}
  const char *c_str() const { return s.c_str(); }
  String &operator+=(const char *c) { s += c; return *this; }
  String &operator+=(const String &o) { s += o.s; return *this; }
  String &operator+=(const __FlashStringHelper *f) { s += (const char *)f; return *this; }
  String &operator+=(int v) { s += std::to_string(v); return *this; }
  String &operator+=(char c) { s += c; return *this; }
  long toInt() const { return atol(s.c_str()); }
  bool operator==(const char *c) const { return s == c; }
  bool operator==(const String &o) const { return s == o.s; }
  String operator+(const String &o) const { return String(s + o.s); }
  String operator+(const char *c) const { return String(s + c); }
};
inline String operator+(const char *a, const String &b) { return String(std::string(a) + b.s); }
struct IPAddress {
  uint8_t a[4] = {0, 0, 0, 0};
  IPAddress() {}
  IPAddress(uint8_t o1, uint8_t o2, uint8_t o3, uint8_t o4) { a[0] = o1; a[1] = o2; a[2] = o3; a[3] = o4; }
  String toString() const { char b[20]; snprintf(b, sizeof b, "%d.%d.%d.%d", a[0], a[1], a[2], a[3]); return String(b); }
};
class Print {
 public:
  std::string log; bool echo = false;
  void put(const std::string &x) { log += x; if (echo) fputs(x.c_str(), stdout); }
  size_t print(const char *x) { put(x); return 0; }
  size_t print(char c) { put(std::string(1, c)); return 0; }
  size_t print(int v) { put(std::to_string(v)); return 0; }
  size_t print(const String &x) { put(x.s); return 0; }
  size_t print(const IPAddress &ip) { put(ip.toString().s); return 0; }
  size_t print(const __FlashStringHelper *x) { put((const char *)x); return 0; }
  size_t print(unsigned v) { put(std::to_string(v)); return 0; }
  size_t println(unsigned v) { print(v); put("\n"); return 0; }
  size_t println() { put("\n"); return 0; }
  size_t println(const char *x) { put(x); put("\n"); return 0; }
  size_t println(int v) { print(v); put("\n"); return 0; }
  size_t println(const String &x) { put(x.s); put("\n"); return 0; }
  size_t println(const IPAddress &ip) { print(ip); put("\n"); return 0; }
  size_t println(const __FlashStringHelper *x) { put((const char *)x); put("\n"); return 0; }
};
