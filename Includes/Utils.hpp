#pragma once

#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>
#include <fstream>
#include <sstream>

bool						isNumber(const std::string &s);
bool						isValidPort(int port);
bool						endsWithSemicolon(const std::string &str);
bool						isValidHost(const std::string& host);
std::string					trimWhiteSpaces(const std::string &str);
std::vector<std::string>	splitToVector(const std::string &str);

std::string					toLowerCase(const std::string& input);
std::string					&pop(std::string& str);
std::string					&to_upper(std::string& str);
void						ft_skip_spacenl(const char *str, int *i);
std::string&				strip(std::string& str, char c);
std::vector<std::string>	split(const std::string& str, char c);
std::string&				capitalize(std::string& str);
std::string&				to_lower(std::string& str);

std::string					to_string(size_t n);
