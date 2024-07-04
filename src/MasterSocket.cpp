#include "../Includes/MasterSocket.hpp"

MasterSocket::~MasterSocket() {}

/* ----- Construction & Start Listening ----- */
MasterSocket::MasterSocket(std::string serverBlock) {
	// for socket
	config = new ServerConfig(serverBlock);

	// define address structure
	address.sin_family = config->getServerConfigDomain();
	address.sin_port = htons(config->getServerConfigPort());
	address.sin_addr.s_addr = config->getServerConfigInterface();

	//establish socket
	sock_fd = socket(config->getServerConfigDomain(), config->getServerConfigService(), config->getServerConfigProtocol());
	
	int option_value = 1;
	setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(int));

	connection = bind(sock_fd, (struct sockaddr *)&address, sizeof(address));
	if (connection < 0) {
		// std::cerr << "ERROR: Failed to bind: " << strerror(errno) << std::endl;
		std::cerr << "ERROR: Failed to bind to " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << ": " << strerror(errno) << std::endl;
		throw std::runtime_error("ERROR: Failed to connect...");
	}
	
	// for mastersocket
	backlog = 10000; // find an other way
	startListening();
	if (listening < 0)
		throw std::runtime_error("ERROR: Failed to connect...");

	// ensure that the master_socket socket operates in non-blocking mode
	// possible to add FD_CLOEXEC arg too, which closes automatically the fd when execve is called
	if (fcntl(sock_fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("ERROR: Failed to connect...");
}

void MasterSocket::startListening() {
	// starts listening for incoming connections on the socket with file descriptor sock_fd. The backlog parameter specifies the maximum number of pending connections that can be queued up waiting to be accepted.
	listening = listen(sock_fd, backlog);
	struct timeval timeout;
	timeout.tv_sec = 5;  // 5 seconds timeout
	timeout.tv_usec = 0;

	// Set receive timeout
	if (setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		throw std::runtime_error("Error: setsockopt() timeout");

	// Set send timeout
	if (setsockopt(sock_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout)) < 0)
		throw std::runtime_error("Error: setsockopt() timeout");

	std::cout << std::endl << GREEN << BOLD << "################################# NEW MASTER SOCKET #################################" << RESET << std::endl;
	std::cout << "socket fd = " << sock_fd << std::endl;
	std::cout << "Endpoint / IP:port comination = " << config->getEndpoint() << std::endl << std::endl;
}


/* ----- Handling ----- */
// at this point, the entire request was fetched, and is not parsed
// request[socket] is the correct request
void MasterSocket::handle(long socket, char **env) {
	Response		response(env);

	if (_requests[socket].find("Transfer-Encoding: chunked") != std::string::npos &&
		_requests[socket].find("Transfer-Encoding: chunked") < _requests[socket].find("\r\n\r\n"))
		parseChunks(socket); // manage chunked data and remove hexadecimals from body

	if (_requests[socket] != "") {
		Request	request(_requests[socket]);
		if (request.getRet() != 200)
			request.setMethod("GET");
		if (_requests[socket] == "400")
			request.setRet(400);
		response.call(request, *this, _keepAlive[socket]);
		// remove the fetched http request string
		_requests.erase(socket);
		// Replace it with the complete http response string
		_requests.insert(std::make_pair(socket, response.getResponse()));
	}
}

void MasterSocket::parseChunks(long socket) {
	size_t headersEndPos = _requests[socket].find("\r\n\r\n");

	std::string headers = toLowerCase(_requests[socket].substr(0, headersEndPos));
	std::string body = _requests[socket].substr(headersEndPos + 4);  // +4 to skip "\r\n\r\n"

	if (headers.find("transfer-encoding: chunked") != std::string::npos) 	{
		size_t		i = 0;
		std::string	_bodyString;

		while (i < body.size()) {
			// Parse the chunk size
			size_t chunkSize = 0;

			while (i < body.size() && body[i] != '\r') {
				chunkSize = chunkSize * 16 + (isxdigit(body[i]) ? (isdigit(body[i]) ? body[i] - '0' : tolower(body[i]) - 'a' + 10) : 0);
				i++;
			}
			i += 2; // Skip the CRLF after the chunk size

			// Append the chunk data to _bodyString
			_bodyString.append(body, i, chunkSize);

			i += chunkSize + 2; // Skip the chunk data and the following CRLF
		}
		_requests[socket] = headers + "\r\n\r\n" + _bodyString + "\r\n\r\n";
	}
}


/* ----- Setters ----- */
void MasterSocket::setPath(std::string path) {
	this->fullLocalPath = path;
}


/* ----- Getters ----- */
int MasterSocket::getSock() {
	return (sock_fd);
}

const std::string &MasterSocket::getPath() const {
	return (this->fullLocalPath);
}

int MasterSocket::getMasterSocketPort() {
	return (config->getServerConfigPort());
}

std::string MasterSocket::getMasterSocketHost() {
	return (config->getServerConfigHost());
}

int MasterSocket::getMasterSocketHostToInt() {
	try {
		return std::stoi(config->getServerConfigHost());
	} catch (std::invalid_argument& e) {
		// handle error: the host string isn't a valid integer
		return 0;
	} catch (std::out_of_range& e) {
		// handle error: the integer is out of the range of int
		return 0;
	}
}

std::map<int, std::string> MasterSocket::getMasterSocketErrorPages() {
	return (config->getServerConfigErrorPages());
}

int MasterSocket::getMasterSocketMaxBodySize() {
	return (config->getServerConfigMaxBodySize());
}

Location MasterSocket::getMasterSocketRootLocation() {
	return (config->getServerConfigLocation());
}

std::vector<Location> MasterSocket::getMasterSocketLocations() {
	return (config->getServerConfigLocationsVector());
}