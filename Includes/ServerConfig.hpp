#pragma once

#include "Colors.hpp"
#include "Utils.hpp"

#include <stdio.h>
#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <cstdlib>
#include <unistd.h>
#include <cctype>
#include <string>
#include <arpa/inet.h>

struct Location {
	std::string					route;			// URL path (or name)
	std::string					root;			// physical path
	std::string					index;			// default file
	std::vector<std::string>	allow_methods;	// GET, POST, DELETE
	bool						autoindex;		// auto look for path
};

class ServerConfig {
	private:
		int							serverConfigDomain;					// Default to AF_INET ??
		int							serverConfigService;				// hardcoded
		int							serverConfigProtocol;				// hardcoded
		u_long						serverConfigInterface;				// hardcoded
		
		int							serverConfigPort;		// Default to 80
		std::string					serverConfigHost;					// Default to 127.0.0.1
		std::string					serverConfigServerName;			// Default to localhost
		int							serverConfigMaxBodySize;	// Default to 1MB
		
		std::map<int, std::string>	serverConfigErrorPages;

		Location					serverConfigLocation;			// Root or default location
		std::vector<Location>		serverConfigLocationsVector;

	public:
		// constructors
		ServerConfig(std::string serverBlock);
		~ServerConfig();

		// PARSING
		void						parseServerBlock(std::string serverBlock);
		bool						containsDirective(const std::string &serverBlock, const std::string &directive);
		std::string					removeDirective(std::string &serverBlock, const std::string &directive);

		// extract directives
		int							extractPort(std::string line);
		std::string					extractHost(std::string line);
		std::string					extractServerName(std::string line);
		void						extractErrorPage(std::string line);
		int							extractClientMaxBodySize(std::string line);

		// extract locations
		void						parseServerConfigLocationsVector(std::string &serverBlock);
		void						parseServerConfigLocation(std::string &serverBlock);
		struct Location				createLocation(std::string locStr);
		std::string					extractRoot(std::string line);
		bool						extractAutoindex(std::string line);
		std::vector<std::string>	extractAllowMethods(std::string line);
		std::string					extractIndex(std::string line, std::string path);
		struct Location				extractRedirect(std::string line);

		// getters
		int							getServerConfigPort();
		int							getServerConfigDomain();
		std::string					getServerConfigHost();
		std::map<int, std::string>	getServerConfigErrorPages();
		int							getServerConfigMaxBodySize();
		Location					getServerConfigLocation();
		std::vector<Location>		getServerConfigLocationsVector();
		std::string					getEndpoint();
		int							getServerConfigService();
		u_long						getServerConfigInterface();
		int							getServerConfigProtocol();
};
