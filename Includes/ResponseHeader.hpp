#pragma once

#include "Colors.hpp"
#include "Utils.hpp"

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

#include <string>
#include <algorithm>
#include <cctype>
#include <sstream>
#include <vector>
#include <fstream>
#include <sys/stat.h>

class ResponseHeader {
	private:
		bool						_keepAlive;
		std::string					_allow;
		std::string					_contentLength;
		std::string					_contentLocation;
		std::string					_contentType;
		std::string					_date;
		std::string					_lastModified;
		std::string					_location;
		std::string					_retryAfter;
		std::string					_server;
		std::string					_transferEncoding;
		std::string					_wwwAuthenticate;
		std::map<int, std::string>	_errors;

		void						initErrorMap();

	public:
		ResponseHeader(void);
		ResponseHeader(const ResponseHeader & src);
		~ResponseHeader(void);

		ResponseHeader & operator=(const ResponseHeader & src);

		// Setter functions
		void			setAllow(std::vector<std::string> methods);
		void			setAllow(const std::string& allow = "");
		void			setContentLength(size_t size);
		void			setContentLocation(const std::string& path, int code);
		void			setContentType(std::string type, std::string path);
		void			setDate(void);
		void			setLastModified(const std::string& path);
		void			setLocation(int code, const std::string& redirect);
		void			setRetryAfter(int code, int sec);
		void			setServer(void);
		void			setTransferEncoding(void);
		void			setWwwAuthenticate(int code);

		// Member functions
		std::string		getHeader(size_t size, const std::string& path, int code, std::string type, const std::string& contentLocation);
		std::string		notAllowed(std::vector<std::string> methods, const std::string& path, int code, size_t size);
		std::string		writeHeader(void);
		void			setValues(size_t size, const std::string& path, int code, std::string type, const std::string& contentLocation);
		void			resetValues(void);
		void			setKeepAlive(bool keepAlive);
		std::string		getStatusMessage(int code);
};
