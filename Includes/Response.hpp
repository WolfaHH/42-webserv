#pragma once

# include "ResponseHeader.hpp"
# include "Request.hpp"
# include "MasterSocket.hpp"
# include "CgiHandler.hpp"
# include "Utils.hpp"
# include "Colors.hpp"
# include "ServerConfig.hpp"

#include <stdio.h>
#include <iostream>
#include <cstring>

#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <errno.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <sys/time.h>
#include <fcntl.h>
#include <map>
#include <vector>
#include <list>
#include <dirent.h>

#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <fstream>
#include <sys/stat.h>

class MasterSocket;

typedef struct	s_listen {
	unsigned int	host;
	int			port;
}				t_listen;

class Response {
	private:
		std::string					_response;
		std::string					_path;
		int							_code;
		bool						_isAutoIndex;
		t_listen					_hostPort;
		std::map<int, std::string>	_errorMap;
		bool						_keepAlive;
		// 0 = directory listing
		// 1 = normal html/jpeg body or CGI
		// 2 = Error
		int							_responseType;
		std::string					_usePath;
		std::string					_mimeType;

		std::string					_responseBody;
		char						**_env;

		// mine
		std::string					fullLocalPath;
		std::string					base_index;

		// for autoindex
		static std::string  getLink(std::string const &dirEntry, std::string const &dirName, std::string const &host, int port, std::string basicUri);
		static std::string getPage(const char *path, std::string const &host, int port, std::string basicUri);


	public:
		Response(char **env);
		Response(const Response & src);
		~Response(void);

		Response &operator=(const Response &src);

		// Getter functions
		std::string	getResponse(void);

		// Member functions
		void		call(Request & request, MasterSocket &requestConf, bool keepAlive);

		void		getHandler(Request & request, MasterSocket &requestConf);
		void		postHandler(Request & request, MasterSocket &requestConf);
		void		deleteHandler(Request & request, MasterSocket &requestConf);

		int			writeContent(std::string content);
		int			fileExists(std::string path);
		std::string	readHtml(const std::string& path);

		void		makeFullLocalPath(MasterSocket *master_socket, const std::string &path, const std::string &method, Location &target_location);
		bool		isFile(const std::string& path);
		bool		isDirectory(const std::string& path);
		std::string	getMimeType(const std::string& filePath);
		std::string	getFileExtension(const std::string& filename);
		void		getPathResponse(MasterSocket &master_socket, Location target_location);

		void		htmlResponder();
		std::string	findExecutablePath(const std::string& command, char** env);

		std::map<std::string, std::string>	getMimeTypes();
};
