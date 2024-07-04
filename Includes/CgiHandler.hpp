#pragma once

# define CGI_BUFSIZE 100536

// # include "Config.hpp"
#include "Request.hpp"
#include "MasterSocket.hpp"
#include "Utils.hpp"

#include <signal.h>

class MasterSocket;

class CgiHandler {
	private:
		CgiHandler(void);
		void								_initEnv(Request &request, MasterSocket &config);
		char								**_getEnvAsCstrArray() const;
		int									_getSocket(unsigned int port);
		int									_connectSocket(unsigned int port);
		std::map<std::string, std::string>	_env;
		std::string							_body;

	public:
		CgiHandler(Request &request, MasterSocket &config); // sets up env according to the request
		CgiHandler(CgiHandler const &src);
		virtual ~CgiHandler(void);

		CgiHandler   	&operator=(CgiHandler const &src);
		std::string		executeCgi(const std::string &scriptName);	// executes cgi and returns body
};
