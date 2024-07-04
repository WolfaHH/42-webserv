#pragma once

#include "Colors.hpp"
#include "MasterSocket.hpp"

#include <iostream>
#include <cstring>
#include <vector>

#include <stdio.h>
#include <errno.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <fcntl.h>

class WebServer {
	private:
		char							**env;
		long							_max_fd;
		
		fd_set							fdSet;

		std::vector<std::string>		serverBlocks;
		std::vector<MasterSocket>		listeningSocketsVector;
		std::map<long, MasterSocket>	listeningSocketsMap;

		std::map<long, MasterSocket *>	readyToReadSocketsMap;
		
		std::vector<int>				readyToWriteSocketsVector;

		void split_server_blocks(const char *config_file);
		std::string	removeCommentLines(std::stringstream &inputStream);
    	std::string::size_type findEndOfServerBlock(const std::string& configContent, std::string::size_type startPos);

		void launch();
		void initializeSets();

		int waitForActivity(fd_set& reading_set, fd_set& writing_set);
		void prepareSets(fd_set& reading_set, fd_set& writing_set);
		void cleanupAndFindMaxFd();
	
		void handleWriteActivity(fd_set& reading_set, fd_set& writing_set);
		long writeRequest(long socket, MasterSocket &serv);
		long prepareWriteRequest(long socket, MasterSocket &serv, std::map<long, size_t>& sent);
		void handleWriteError(long socket, MasterSocket &serv, std::map<long, size_t>& sent);
		void handleWriteResult(long ret, std::vector<int>::iterator& it, fd_set& reading_set);

		void handleReadActivity(fd_set& reading_set, fd_set& writing_set);
		int readRequest(long socket, MasterSocket &serv);
		bool requestCompletelyReceived(std::string completeData, MasterSocket &serv, long socket);
		void handleConnectionHeader(std::string lowerCaseHeaders, MasterSocket &serv, long socket);
		bool handleContentLengthHeader(std::string lowerCaseHeaders, std::string body, MasterSocket &serv, long socket);
		
		void handleAcceptActivity(fd_set& reading_set);
		long acceptNewConnection(MasterSocket &activeSocket);

		void handleSelectError(int select_activity);
		
	public:
		WebServer(char *config_file, char **env);
		~WebServer();
};
