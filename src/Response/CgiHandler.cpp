#include "../../Includes/CgiHandler.hpp"

CgiHandler::CgiHandler(Request &request, MasterSocket &config) : _body(request.getBody()) {
	this->_initEnv(request, config);
}

CgiHandler::CgiHandler(CgiHandler const &src) {
	if (this != &src) {
		this->_body = src._body;
		this->_env = src._env;
	}
	return ;
}

CgiHandler::~CgiHandler(void) {}

CgiHandler	&CgiHandler::operator=(CgiHandler const &src) {
	if (this != &src) {
		this->_body = src._body;
		this->_env = src._env;
	}
	return *this;
}


// create the env for execve
void		CgiHandler::_initEnv(Request &request, MasterSocket &config) {
	std::map<std::string, std::string>	headers = request.getHeaders();
	if (headers.find("Auth-Scheme") != headers.end() && headers["Auth-Scheme"] != "")
		this->_env["AUTH_TYPE"] = headers["Authorization"];

	this->_env["REDIRECT_STATUS"] = "200"; //Security needed to execute php-cgi
	this->_env["GATEWAY_INTERFACE"] = "CGI/1.1";
	this->_env["SCRIPT_NAME"] = config.getPath();
	this->_env["SCRIPT_FILENAME"] = config.getPath();
	this->_env["REQUEST_METHOD"] = request.getMethod();
	this->_env["CONTENT_LENGTH"] = to_string(this->_body.length());
	this->_env["CONTENT_TYPE"] = headers["Content-Type"];
	this->_env["PATH_INFO"] = request.getPath(); //might need some change, using config path/contentLocation
	this->_env["PATH_TRANSLATED"] = request.getPath(); //might need some change, using config path/contentLocation
	this->_env["QUERY_STRING"] = request.getQuery();
	this->_env["REMOTEaddr"] = to_string(config.getMasterSocketHostToInt());
	this->_env["REMOTE_IDENT"] = headers["Authorization"];
	this->_env["REMOTE_USER"] = headers["Authorization"];
	this->_env["REQUEST_URI"] = request.getPath() + request.getQuery();
	if (headers.find("Hostname") != headers.end())
		this->_env["SERVER_NAME"] = headers["Hostname"];
	else
		this->_env["SERVER_NAME"] = this->_env["REMOTEaddr"];
	this->_env["SERVER_PORT"] = to_string(config.getMasterSocketPort());
	this->_env["SERVER_PROTOCOL"] = "HTTP/1.1";
	this->_env["SERVER_SOFTWARE"] = "WebServ/1.0";

	if (request.getMethod() == "POST") {
		this->_env["SCRIPT_NAME"] = "./src/Cgi/post.py";
		this->_env["SCRIPT_FILENAME"] = "./src/Cgi/post.py";
	}
}

char	**CgiHandler::_getEnvAsCstrArray() const {
	char	**env = new char*[this->_env.size() + 1];
	int	j = 0;
	for (std::map<std::string, std::string>::const_iterator i = this->_env.begin(); i != this->_env.end(); i++) {
		std::string	element = i->first + "=" + i->second;
		env[j] = new char[element.size() + 1];
		env[j] = strcpy(env[j], (const char*)element.c_str());
		j++;
	}
	env[j] = NULL;
	return env;
}


bool    timer(const std::clock_t start) {
	static int i = 1;
	std::clock_t end = std::clock();
	if ((end - start) >= static_cast<unsigned long> (i * CLOCKS_PER_SEC))
		std::cout << i++ << "sc passed\n" << std::endl;
	if (end - start >= 5 * CLOCKS_PER_SEC)
		return true;
	return false;
}

std::string		CgiHandler::executeCgi(const std::string& scriptName) {
	pid_t		pid;
	int			saveStdin;
	int			saveStdout;
	char		**env;
	std::string	newBody;

	std::cout << scriptName << std::endl;

	try	{
		env = this->_getEnvAsCstrArray();
	}
	catch (std::bad_alloc &e) {
		std::cout << "cerr " << std::endl;
		std::cerr << RED << e.what() << RESET << std::endl;
	}

	saveStdin = dup(STDIN_FILENO);
	saveStdout = dup(STDOUT_FILENO);

	int pipeOut[2];  // pipe for output

    if ( pipe(pipeOut) == -1) {
        std::cerr << "Failed to create pipes" << std::endl;
        return ("Status: 500\r\n\r\n");
    }

	const char* tmpFilePath = "/tmp/request_body.txt";
    int tmpFileFd = open(tmpFilePath, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
	if (tmpFileFd == -1)
		std::cout << RED << "OPEN ERROR" << RESET << std::endl;

	if (write(tmpFileFd, _body.c_str(), _body.size()) < 0) {
		std::cerr << RED << "Write crashed." << RESET << std::endl;
		return ("Status: 500\r\n\r\n");
	}
	
	close(tmpFileFd);
	tmpFileFd = open(tmpFilePath, O_RDWR | O_CREAT );

	pid = fork();
	
	if (pid == -1) {
		std::cerr << RED << "Fork crashed." << RESET << std::endl;
		return ("Status: 500\r\n\r\n");
	}
	else if (pid == 0) {
		char *argv[] =
		{
			const_cast<char*>(scriptName.c_str()),
			const_cast<char*>(this->_env["SCRIPT_NAME"].c_str()),
			NULL
		};

		dup2(tmpFileFd, STDIN_FILENO);
		close(tmpFileFd);
		close(pipeOut[0]); // Close read end of output pipe

        dup2(pipeOut[1], STDOUT_FILENO);
		close(pipeOut[1]);

		execve(scriptName.c_str(), argv, env);

		std::cerr << RED << "Execve crashed." << RESET << std::endl;
		if (write(STDOUT_FILENO, "Status: 500\r\n\r\n", 15) < 0)
			std::cerr << RED << "Write crashed." << RESET << std::endl;
		exit(0);
	}
	else {
		int status;
		const std::clock_t c_start = std::clock();
		while (waitpid(pid, &status, WNOHANG) != pid) {
			if (timer(c_start)) {
				kill(pid, SIGKILL);
				std::cerr << RED << "ERROR: Terminating script due to timeout." << RESET << std::endl;
				return ("Status: 500\r\n\r\n");
			}
		}

		char	buffer[CGI_BUFSIZE] = {0};

		waitpid(-1, NULL, 0);
  		close(pipeOut[1]);
		while (true) {
			memset(buffer, 0, sizeof(buffer));
			int readbytes = read(pipeOut[0], buffer, sizeof(buffer) - 1);

			if (readbytes <= 0)
				break;
			else if (readbytes > 20000)
				return ("Status: 500\r\n\r\n");
			newBody += buffer;
		}
	}
	dup2(saveStdin, STDIN_FILENO);
	dup2(saveStdout, STDOUT_FILENO);
	close(pipeOut[0]);
  
	close(tmpFileFd);
	close(saveStdin);
	close(saveStdout);

	for (size_t i = 0; env[i]; i++)
		delete[] env[i];
	delete[] env;

	if (!pid)
		exit(0);

	// std::cout << "test newBody: " << newBody << " : fin test newBody" <<std::endl;

	return (newBody);
}