#include "../Includes/WebServer.hpp"

WebServer::WebServer(char *config_file, char **env) : env(env) {
	split_server_blocks(config_file);

	if (serverBlocks.empty())
		throw std::runtime_error("Error: No server blocks found in config file");

	// Create a new MasetrSocket for each server{} block
	for (size_t i = 0; i < serverBlocks.size(); ++i)
		listeningSocketsVector.push_back(MasterSocket(serverBlocks[i]));

	launch();
}

WebServer::~WebServer() {}

/* ----- SplitServerBlocks*/
void WebServer::split_server_blocks(const char *config_file) {
	std::ifstream configFile(config_file);
	if (!configFile)
		throw std::runtime_error(std::string("Error opening file: ") + config_file);

	std::stringstream buffer;
	// When you do buffer << configFile.rdbuf();, you're reading the entire contents of the file into the std::stringstream object buffer. This is a quick and efficient way to read an entire file into memory.
	buffer << configFile.rdbuf();

	// The buffer.str() part is converting the std::stringstream object buffer to a std::string.
	std::string configContent = removeCommentLines(buffer);
	
	// it is guaranteed to be able to represent any possible string size, while other types might not be. This makes the code more portable and robust, as it will work correctly with very large strings on all platforms and compilers.
	std::string::size_type startPos = 0;

	// This loop continues as long as "server" is found in configContent. The find function is called with the second parameter startPos, which means it starts searching from startPos position. After each iteration, startPos is updated to the position of the next "server" occurrence, npos is returned if "server" is not found anymore
	while ((startPos = configContent.find("server", startPos)) != std::string::npos) {
		//This line calls the findEndOfServerBlock() method, which finds the position of the end of the server block that starts at startPos. It does this by counting the opening and closing brackets.
		std::string::size_type endPos = findEndOfServerBlock(configContent, startPos);
		
		// This line calls the validateAndAddServerBlock() method, which checks if the server block is valid (i.e., it has an equal number of opening and closing brackets) and, if it is, adds it to the serverBlocks vector.
		serverBlocks.push_back(configContent.substr(startPos, endPos - startPos));
		
		// This line erases the server block that was just processed from configContent. The erase function takes two parameters: the starting position and the number of characters to erase. Here, it erases the characters from startPos to endPos.
		configContent.erase(startPos, endPos - startPos);
	}

	// check if there is something else remaining in the config file
	if (configContent.find_first_not_of(" \t\n\r\f\v") != std::string::npos)
		throw std::runtime_error("Error: Only specify config inside server { } blocks!");
}

std::string WebServer::removeCommentLines(std::stringstream &inputStream) {
	std::stringstream output;
	std::string line;
	while (std::getline(inputStream, line)) {
		// This means it's looking for the first character in line that is not a space (' ') or a tab ('\t').
		size_t start = line.find_first_not_of(" \t");
		
		if (start != std::string::npos)
			line = line.substr(start);
		
		else
			line.clear();
		
		// Check if line starts with # after leading spaces are removed
		if (!line.empty() && line[0] != '#')
			output << line << "\n";
	}
	return output.str();
}

std::string::size_type WebServer::findEndOfServerBlock(const std::string& configContent, std::string::size_type startPos) {
	std::string::size_type endPos = startPos + 6;  // move past "server"

	while (std::isspace(configContent[endPos]) && endPos < configContent.size())
		endPos++;

	if (!configContent[endPos] || configContent[endPos] != '{')
		throw std::runtime_error("Error: wrong config file format");

	int bracketCount = 1;
	endPos++;

	while (bracketCount > 0 && endPos < configContent.size()) {
		if (configContent[endPos] == '{')
			bracketCount++;
		else if (configContent[endPos] == '}')
			bracketCount--;
		endPos++;
	}

	if (bracketCount != 0)
		throw std::runtime_error("Error: unmatched brackets in config file");

	return endPos;
}


/* ----- Launch ----- */
void	WebServer::launch() {
	// Initialize the file descriptor sets for reading and writing
	initializeSets();
	// Ignore SIGPIPE signals to prevent the program from crashing
	signal(SIGPIPE, SIG_IGN);

	while (true) {
		// Declare file descriptor sets for reading and writing
		fd_set	reading_set;
		fd_set	writing_set;

		// Wait for activity on any of the sockets. The function returns the number of sockets ready for activity.
		int select_activity = waitForActivity(reading_set, writing_set);
		
		// If there is activity on any of the sockets
		if (select_activity > 0) {
			// Handle write activity on the sockets
			// std::cout << "Entering handle Write\n";
			handleWriteActivity(reading_set, writing_set);

			// Handle read activity on the sockets
			// std::cout << "Entering handle Read\n";
			handleReadActivity(reading_set, writing_set);

			// Handle new incoming connections
			// std::cout << "Entering handle Accept\n";
			handleAcceptActivity(reading_set);
		} else {
			// Handle errors from the select function
			handleSelectError(select_activity);
		}
	}
}


/* ----- InitializeSets ----- */
void	WebServer::initializeSets() {
	FD_ZERO(&fdSet);
	_max_fd = 0;
	
	// adds servers sockets to fdSet set
	for(std::vector<MasterSocket>::iterator it = listeningSocketsVector.begin(); it != listeningSocketsVector.end(); ++it) {
		// This line adds the socket's file descriptor to the set of file descriptors. This is necessary so that the select() function will monitor this socket for activity.
		FD_SET(it->getSock(), &fdSet);
		// This line inserts a pair into the listeningSocketsMap map, where the key is the socket's file descriptor and the value is the MasterSocket object itself. This map allows you to easily find the MasterSocket object associated with a given file descriptor.
		listeningSocketsMap.insert(std::make_pair(it->getSock(), *it));
		if (it->getSock() > _max_fd)
			_max_fd = it->getSock();
		std::cout << "Setting up " << it->getMasterSocketHost() << ":" << it->getMasterSocketPort() << "..." << std::endl;
	}
}


/* ----- WaitForActivity ----- */
int WebServer::waitForActivity(fd_set& reading_set, fd_set& writing_set) {
	struct timeval	timeout;
	int select_activity = 0;
	while (select_activity == 0) {
		timeout.tv_sec  = 3;
		timeout.tv_usec = 0;
		// std::cout << "Entering Prepare Sets\n";
		prepareSets(reading_set, writing_set);
		// std::cout << "Entering Cleanup\n";
		cleanupAndFindMaxFd();
		// std::cout << "Entering Select\n";
		select_activity = select(_max_fd + 1, &reading_set, &writing_set, NULL, NULL);
		// std::cout << select_activity << " out of " << _max_fd << " are added to select_activity" << std::endl;
	}
	return select_activity;
}

void WebServer::prepareSets(fd_set& reading_set, fd_set& writing_set) {
	memcpy(&reading_set, &fdSet, sizeof(fdSet));
	FD_ZERO(&writing_set);
	for (std::vector<int>::iterator it = readyToWriteSocketsVector.begin() ; it != readyToWriteSocketsVector.end() ; it++) {// add every new socket that is ready to the writing set
		FD_SET(*it, &writing_set);
		// std::cout << "Request fd " << *it << " added to readyToWriteSocketsVector set\n";
	}
	if (readyToWriteSocketsVector.size() == 0) {
		// std::cout << "No requests in readyToWriteSocketsVector set\n";
	}
}

void WebServer::cleanupAndFindMaxFd() {
	long highestActiveFd = 0;

	// Iterate over all possible file descriptors up to _max_fd
	for (int fd = 0; fd <= _max_fd; fd++) {
		// If the file descriptor is in the set of active descriptors
		if (FD_ISSET(fd, &fdSet)) {
			// Update highestActiveFd if necessary
			if (fd > highestActiveFd)
				highestActiveFd = fd;
		} 
		// If the file descriptor is not in the set and is greater than 10
		else if (fd > 10) {
			// Close the file descriptor and remove it from the readyToReadSocketsMap map
			close(fd);
			readyToReadSocketsMap.erase(fd);
		}
	}

	// Update _max_fd to the highest active file descriptor
	_max_fd = highestActiveFd;
}


/* ----- HandleWriteActivity ----- */
void WebServer::handleWriteActivity(fd_set& reading_set, fd_set& writing_set) {
	for (std::vector<int>::iterator it = readyToWriteSocketsVector.begin() ; it != readyToWriteSocketsVector.end() ; it++) {
		if (FD_ISSET(*it, &writing_set)) {
			long ret = writeRequest(*it, *readyToReadSocketsMap[*it]);
			handleWriteResult(ret, it, reading_set);
			break;
		}
	}
}

long WebServer::writeRequest(long socket, MasterSocket &serv) {
	static std::map<long, size_t>	sent;
	int ret = prepareWriteRequest(socket, serv, sent);

	if (ret == -1) {  // if an error occured with write() 
		handleWriteError(socket, serv, sent);
		return (-1);
	} else {
		sent[socket] += ret; // save how much of the response was already witten()
		if (sent[socket] >= serv._requests[socket].size()) { // if all of the response was written()
			serv._requests.erase(socket); // erase the response
			sent[socket] = 0; // reset the sent size

			if (serv._keepAlive.find(socket) != serv._keepAlive.end() && !serv._keepAlive[socket])
				close(socket);
			return (0);
		} else	// if part of the response was written
			return (1);
	}
}

long WebServer::prepareWriteRequest(long socket, MasterSocket &serv, std::map<long, size_t>& sent) {
	if (sent.find(socket) == sent.end())
		sent[socket] = 0;

	std::string str = serv._requests[socket].substr(sent[socket], 30000);
	return write(socket, str.c_str(), str.size());
}

void WebServer::handleWriteError(long socket, MasterSocket &serv, std::map<long, size_t>& sent) {
	if (socket > 0)
		close(socket);

	serv._requests.erase(socket);
	sent[socket] = 0;
}

void WebServer::handleWriteResult(long ret, std::vector<int>::iterator& it, fd_set& reading_set) {
	if (ret == 0) { // if the response was entirely written()
		std::cout << GREEN << "\r============= HTTP RESPONSE SENT =============" << RESET << std::endl;
		readyToWriteSocketsVector.erase(it); // erase the socket from the ready set

		// Check if we should close the connection
		if (readyToReadSocketsMap[*it]->_keepAlive.find(*it) != readyToReadSocketsMap[*it]->_keepAlive.end() && !readyToReadSocketsMap[*it]->_keepAlive[*it]) {
			FD_CLR(*it, &fdSet);
			FD_CLR(*it, &reading_set);
			close(*it);
			readyToReadSocketsMap.erase(*it);
		}
	} else if (ret == -1) {
		FD_CLR(*it, &fdSet);
		FD_CLR(*it, &reading_set);
		close(*it);
		readyToReadSocketsMap.erase(*it);
		readyToWriteSocketsVector.erase(it);
	}
}


/* ----- HandleReadActivity ----- */
void WebServer::handleReadActivity(fd_set& reading_set, fd_set& writing_set) {
	for (std::map<long, MasterSocket *>::iterator it = readyToReadSocketsMap.begin() ; it != readyToReadSocketsMap.end() ; it++) {
		long	socket = it->first;
		if (FD_ISSET(socket, &reading_set)) {
			long	readRet = readRequest(socket, *it->second);
			if (readRet == 0) {
				std::cout << GREEN << "\r============= HTTP REQUEST RECEIVED/READ =============" << RESET << std::endl;
				it->second->handle(socket, env); // parsing
				readyToWriteSocketsVector.push_back(socket); // add to ready set to write()
			} else if (readRet == -1) {
				FD_CLR(socket, &fdSet);
				FD_CLR(socket, &reading_set);
				FD_CLR(socket, &writing_set);
				
				close(socket);	// closing twice
				readyToReadSocketsMap.erase(socket);
				it = readyToReadSocketsMap.begin();
			}
			break;
		}
	}
}

int	WebServer::readRequest(long socket, MasterSocket &serv) {
	int		bytes_read = 0;
	char	buffer[40000];

	memset(buffer, 0, sizeof(buffer));

	bytes_read = read(socket, buffer, sizeof(buffer) - 1);

	if (bytes_read == 0 || bytes_read == -1) {
		if (socket > 0)
			close(socket);
		serv._requests.erase(socket);

		if (!bytes_read)
			std::cout << YELLOW << BOLD << "Client " << socket << " closed connection" << RESET << std::endl;
		else
			std::cerr << RED << BOLD << "ERROR: Client " << socket << " read returned -1." << RESET << std::endl;
		return (-1);
	} else if (bytes_read != 0)
		serv._requests[socket].append(buffer, bytes_read);
	
	if (requestCompletelyReceived(serv._requests[socket], serv, socket) == true)
		return 0;
	else
		return 1;
}

bool WebServer::requestCompletelyReceived(std::string completeData, MasterSocket &serv, long socket) {
	size_t headersEndPos = completeData.find("\r\n\r\n");
	if (headersEndPos == std::string::npos)
		return (false);

	std::string headers = completeData.substr(0, headersEndPos);
	std::string body = completeData.substr(headersEndPos + 4);  // +4 to skip "\r\n\r\n"

	// lower case the headers to find "content-length" and "Content-Length"
	std::string lowerCaseHeaders = toLowerCase(headers);
	handleConnectionHeader(lowerCaseHeaders, serv, socket);
	lowerCaseHeaders = lowerCaseHeaders + "\r\n";
	size_t contentLengthPos = lowerCaseHeaders.find("\r\ncontent-length:");

	if (handleContentLengthHeader(lowerCaseHeaders, body, serv, socket))
		return (true);

	if (lowerCaseHeaders.find("transfer-encoding: chunked") != std::string::npos) {
		if (body.find("0\r\n\r\n") != std::string::npos)
			return (true);
	}

	// No body to receive, just headers
	if (contentLengthPos == std::string::npos && lowerCaseHeaders.find("transfer-encoding:") == std::string::npos)
		return (true);

	std::cout << "Body not completely received\n";
	// Body not completely received
	return (false);
}

void WebServer::handleConnectionHeader(std::string lowerCaseHeaders, MasterSocket &serv, long socket) {
	size_t conn_pos = lowerCaseHeaders.find("\r\nconnection:");
	if (conn_pos != std::string::npos) {
		// Get the value after the "connection:" string.
		size_t start = conn_pos + std::strlen("\r\nconnection:");
		size_t end = lowerCaseHeaders.find("\r\n", start);
		if(end == std::string::npos) // if there's no other header after "connection:"
			end = lowerCaseHeaders.length(); // consider till the end of the string
		
		std::string conn_value = lowerCaseHeaders.substr(start, end - start);

		// Trim whitespace (both leading and trailing)
		conn_value.erase(0, conn_value.find_first_not_of(" \t\n\r"));
		conn_value.erase(conn_value.find_last_not_of(" \t\n\r") + 1);

		if (conn_value == "keep-alive")
			serv._keepAlive[socket] = true;
		else if (conn_value == "close")
			serv._keepAlive[socket] = false;
		// If not specified, and if you're dealing with HTTP/1.1:
		else
			serv._keepAlive[socket] = true; // Default for HTTP/1.1
	} else {
		// If the header isn't present and you're assuming HTTP/1.1:
		serv._keepAlive[socket] = true; // Default for HTTP/1.1
	}
}

bool WebServer::handleContentLengthHeader(std::string lowerCaseHeaders, std::string body, MasterSocket &serv, long socket) {
	size_t contentLengthPos = lowerCaseHeaders.find("\r\ncontent-length:");
	if (contentLengthPos != std::string::npos) {
		size_t start = contentLengthPos + std::strlen("\r\ncontent-length:");
		size_t end = lowerCaseHeaders.find("\r\n", start);
		if (end != std::string::npos) {
			std::string lengthStr = trimWhiteSpaces(lowerCaseHeaders.substr(start, end - start));
			try {
				int contentLength = std::stoi(lengthStr);
				if (contentLength == 0)
					return (true);
				if (body.size() == static_cast<size_t>(contentLength))
					return (true);
				else if (body.size() > static_cast<size_t>(contentLength)) {
					serv._requests[socket] = "400";
					std::cerr << RED << "Error with content-length: body > content-length!" << RESET << std::endl;
					return (true);
				}
			} catch (const std::exception &e) {
				std::cerr << RED << "Error with content-length: not an int!" << RESET << std::endl;
				serv._requests[socket] = "400";
				return (true);
			}
		}
	}
	return (false);
}


/* ----- HandleAcceptActivity ----- */
void WebServer::handleAcceptActivity(fd_set& reading_set) {
	for (std::map<long, MasterSocket>::iterator it = listeningSocketsMap.begin() ; it != listeningSocketsMap.end() ; it++) {
		if (FD_ISSET(it->first, &reading_set)) {
			// std::cout << "accept" << std::endl;
			long	socket = acceptNewConnection(it->second); // get new socket
			if (socket != -1) {
				FD_SET(socket, &fdSet);
				readyToReadSocketsMap.insert(std::make_pair(socket, &(it->second)));
				if (socket > _max_fd)
					_max_fd = socket;
			}
			break;
		}
	}
}

long	WebServer::acceptNewConnection(MasterSocket &activeSocket) {
	struct sockaddr_in	address;
	long				addrlen = sizeof(address);
	long				acceptedSocket;

	acceptedSocket = accept(activeSocket.getSock(), (struct sockaddr *)&address, (socklen_t*)&addrlen);
	if (acceptedSocket == -1)
		std::cerr << RED << "Could not accept socket: " << strerror(errno) << RESET << std::endl;
	else {
		fcntl(acceptedSocket, F_SETFL, O_NONBLOCK);
		activeSocket._requests.insert(std::make_pair(acceptedSocket, ""));
		std::cout << BLUE << BOLD << "New Connection, Assigned Socket " << acceptedSocket << RESET << std::endl;
	}
	return (acceptedSocket);
}


/* ----- HandleSelectError ----- */
void WebServer::handleSelectError(int select_activity) {
	std::cerr << RED << "ERROR: select returned = " << select_activity << RESET << std::endl;
	for (std::map<long, MasterSocket *>::iterator it = readyToReadSocketsMap.begin() ; it != readyToReadSocketsMap.end() ; it++) {
		if (it->first > 0)
			close(it->first);
		it->second->_requests.erase(it->first);
	}
	readyToReadSocketsMap.clear();
	readyToWriteSocketsVector.clear();
	FD_ZERO(&fdSet);

	_max_fd = 0;
	for (std::map<long, MasterSocket>::iterator it = listeningSocketsMap.begin() ; it != listeningSocketsMap.end() ; it++) {
		FD_SET(it->first, &fdSet);
		if (it->first > _max_fd)
			_max_fd = it->first;
	}
}


