#pragma once

# define PRINT 0 		// set to 1 to print request and response

#include "Colors.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Utils.hpp"
#include "ServerConfig.hpp"

#include <fcntl.h>

class Response; //initialize class to avoid circular dependency, used in handle.

// MasterSocket represents each server
// it is first created to parse the config file, create, bind, and listen to each master socket
// it holds eversy attributes as well, making it easier to access the data later
// once a request is received, it is parsed here to create the response
class MasterSocket {
	private:
		int							backlog;					// hardcoded
		int							listening;
		int							sock_fd;
		int							connection;
		std::string					fullLocalPath;
		struct sockaddr_in			address;

		ServerConfig 				*config;

	public:
		std::map<long, std::string>			_requests;					// at first, this map is the request, after the parsing, it's the response
		std::map<long, bool>				_keepAlive;

		// Construction & Start Listening
		MasterSocket(std::string serverBlock);
		~MasterSocket();

		void								startListening();
		
		// Handling
		void								handle(long socket, char **env);	// main parsing function. Entry point from WebServer
		void								parseChunks(long socket);

		// Setter
		void								setPath(std::string path);
		
		// Getters
		int									getSock();
		const std::string					&getPath() const;
		int									getMasterSocketPort();
		std::string							getMasterSocketHost();
		int									getMasterSocketHostToInt();
		std::map<int, std::string>			getMasterSocketErrorPages();
		int									getMasterSocketMaxBodySize();
		Location							getMasterSocketRootLocation();
		std::vector<Location>				getMasterSocketLocations();
};
