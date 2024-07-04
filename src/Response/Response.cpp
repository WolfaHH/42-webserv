#include "../../Includes/Response.hpp"

int		pathIsFile(const std::string& path) {
	struct stat s;
	if (stat(path.c_str(), &s) == 0 ) {
		if (s.st_mode & S_IFDIR)
			return 0;
		else if (s.st_mode & S_IFREG)
			return 1;
		else
			return 0;
	}
	else
		return 0;
}

// Static Assets

void	Response::makeFullLocalPath(MasterSocket *master_socket, const std::string &path, const std::string &method, Location &target_location) {
	std::vector<Location> locationVector = master_socket->getMasterSocketLocations();
	
	std::string basePath;

	// check if the path is the root path
	int secondBackslashPos = path.substr(1).find("/");
	if (secondBackslashPos != -1 || (secondBackslashPos == -1 && path.size() > 1)) {
		basePath = path.substr(1, secondBackslashPos);
	} else {
		basePath = "/";
	}
	bool found_loc = false;
	for (unsigned long i = 0;  i < locationVector.size(); i++) {
		if (!locationVector[i].route.compare("/" + basePath) && basePath != "/") {
			found_loc = true;
			target_location = locationVector[i];
			int meth = 0;
			for (size_t j = 0; j < locationVector[i].allow_methods.size(); j++) {
				if (locationVector[i].allow_methods[j] == method)
					meth = 1;
			}
			if (meth == 0)
				_code = 405;
			std::string first = locationVector[i].root;
			std::string sec = path.substr(basePath.length() + 1);
			
			// remove a "/" if both the first and the path have one
			if (!first.empty() && first[first.size() - 1] == '/' && !sec.empty() && sec[0] == '/')
				first.pop_back();

			// add a "/" if neither the first nor the sec have one
			else if (!first.empty() && first[first.size() - 1] != '/' && !sec.empty() && sec[0] != '/')
				first = first + '/';

			fullLocalPath = first + sec;
			base_index = locationVector[i].index;
		}
	}
	if (found_loc == false) {
		target_location = master_socket->getMasterSocketRootLocation();
		std::string first = master_socket->getMasterSocketRootLocation().root;

		int meth = 0;
		for (size_t j = 0; j < master_socket->getMasterSocketRootLocation().allow_methods.size(); j++) {
			if (master_socket->getMasterSocketRootLocation().allow_methods[j] == method)
				meth = 1;
		}
		if (meth == 0)
			_code = 405;

		// remove a "/" if both the first and the path have one
		if (!first.empty() && first[first.size() - 1] == '/' && !path.empty() && path[0] == '/')
			first.pop_back();

		// add a "/" if neither the first nor the path have one
		else if (!first.empty() && first[first.size() - 1] != '/' && !path.empty() && path[0] != '/')
			first = first + '/';

		fullLocalPath = first + path;
		base_index = master_socket->getMasterSocketRootLocation().index;
	}
};

// RESPONSE UTILS


// Function to extract the file extension
std::string Response::getFileExtension(const std::string& filename) {
	size_t dotPos = filename.find_last_of(".");
	if (dotPos == std::string::npos) {
		return "";  // No extension found
	}
	return filename.substr(dotPos);
}


std::map<std::string, std::string> Response::getMimeTypes() {
	std::map<std::string, std::string> mimeTypes;

	mimeTypes[".jpeg"] = "image/jpeg";
	mimeTypes[".jpg"] = "image/jpeg";
	mimeTypes[".png"] = "image/png";
	mimeTypes[".txt"] = "text/plain";
	mimeTypes[".html"] = "text/html";
	mimeTypes[".htm"] = "text/html";
	mimeTypes[".php"] = "text/html";
	mimeTypes[".py"] = "text/html";
	mimeTypes[".pdf"] = "application/pdf";
	mimeTypes[".css"] = "text/css";
	mimeTypes[".gif"] = "image/gif";
	mimeTypes[".mp4"] = "video/mp4";

	return mimeTypes;
}

std::string Response::getMimeType(const std::string& filePath) {
	static const std::map<std::string, std::string> mimeTypes = getMimeTypes();


	std::string ext = getFileExtension(filePath);

	std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);  // Convert to lowercase

	std::map<std::string, std::string>::const_iterator it = mimeTypes.find(ext);
	if (it != mimeTypes.end()) {
		return it->second;
	}
	return "what the fuck";  // default MIME type for unknown extensions
}


bool Response::isDirectory(const std::string& path) {
	struct stat s;
	if (stat(path.c_str(), &s) == 0) {
		if (S_ISDIR(s.st_mode))
			return true;
	}
	return false;
}

bool Response::isFile(const std::string& path) {
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (S_ISREG(s.st_mode))
			return true;
	}
	return false;
}


// this function finds what to do with the request
// it sets the _responseType attribute to:
	// 0 = directory listing
	// 1 = normal html/jpeg body or CGI
	// 2 = Error
void Response::getPathResponse(MasterSocket &master_socket, Location target_location) {
	std::string pathToCheck;

	if (access(fullLocalPath.c_str(), F_OK) != 0) {
		if (errno == ENOENT)
			_code = 404;  // Not Found
		else if (errno == EACCES)
			_code = 403;  // Forbidden
		else
			_code = 500;  // Internal Server Error
		return;
	}
	
	// if the path is a directory, try to append the default file (index.html) otherwise check for autoindex
	if (!isFile(fullLocalPath))	{
		if (fullLocalPath[fullLocalPath.size() - 1] != '/')
			fullLocalPath += "/";
		std::string append_index = fullLocalPath + base_index;

		// if the default file doesnt exist and the auto index is off --> error
		if (access(append_index.c_str(), F_OK) != 0 && !target_location.autoindex) {
			
			if (errno == ENOENT)
				_code = 404;  // Not Found
			else if (errno == EACCES)
				_code = 403;  // Forbidden
			else
				_code = 500;  // Internal Server Error
			_responseType = 2;
			_mimeType = "text/html";
			_usePath = fullLocalPath;	// use the path + default/index file

			return;
		}

		// if the default file doesnt exist and the auto index is on --> show directory listing
		else if (access(append_index.c_str(), F_OK) != 0 && master_socket.getMasterSocketRootLocation().autoindex) {
			_responseType = 0;	// directory listing
			_mimeType = "text/html";
			_usePath = fullLocalPath;
			// FetchHtmlBody::dispatchResponse(*this, fullLocalPath, "text/html");
		}

		// if the default file exists --> show index file
		else {
			_responseType = 1;	// normal html/jpeg body or CGI
			_mimeType = getMimeType(append_index);	// check the mimeType??
			_usePath = append_index;	// use the path + default/index file
		}

	}
	// if the path is a file, check if it's a .py for CGI, and html/jpeg/png for normal body, or an error
	else {
		std::string mimeType = getMimeType(fullLocalPath);
		
		if (mimeType == "what the fuck") {
			_code = 501;
			_responseType = 2;
			_mimeType = "text/html";
			_usePath = fullLocalPath;	// use the path + default/index file
		}
		else {
			_responseType = 1; // CGI or html/jpeg body
			_mimeType = mimeType;
			_usePath = fullLocalPath;	// use the path + default/index file
		}
	}
}


// Member functions

void			Response::call(Request &request, MasterSocket &requestConf, bool keepAlive) {
	Location	target_location;
	this->_keepAlive = keepAlive;

	_code = request.getRet();
	// here, target_location is the right location block
	// private attribute "fullLocalPath" is the correct physical root in my computer
	// private attribute "base_index" is the correct index file
	if (_code == 200) {
		makeFullLocalPath(&requestConf, request.getPath(), request.getMethod(), target_location);
		requestConf.setPath(this->fullLocalPath);
	}
	_errorMap = requestConf.getMasterSocketErrorPages();
	_isAutoIndex = target_location.autoindex;
	_hostPort.host = requestConf.getMasterSocketHostToInt();
	_hostPort.port = requestConf.getMasterSocketPort();
	_path = requestConf.getMasterSocketRootLocation().root;
	if (static_cast<std::string::size_type>(requestConf.getMasterSocketMaxBodySize()) < request.getBody().size() && _code != 405)
		_code = 413; // Payload Too Large
	if (_code == 405 || _code == 413) {
		ResponseHeader	head;	// todo: add keepalive to every ResponseHeader	head declaration
		head.setKeepAlive(this->_keepAlive);
		_response = this->readHtml(_errorMap[_code]);
		head.setContentLength(_response.size());
		_response = head.notAllowed(target_location.allow_methods, request.getPath(), _code, _response.size()) + "\r\n" + _response;
		return ;
	}
	if ((request.getMethod() == "GET" || request.getMethod() == "POST") && _code != 400) {
		getPathResponse(requestConf, target_location);
		requestConf.setPath(_usePath);	// in case index was appended
	}
	if (_code == 501 || _code == 400) {
		ResponseHeader	head;
		head.setKeepAlive(this->_keepAlive);
		_mimeType = "text/html";
		_response = this->readHtml(_errorMap[_code]);
		head.setContentLength(_response.size());
		_response = head.getHeader(_response.size(), _path, _code, _mimeType, request.getPath()) + "\r\n" + _response;

		return ;
	}
	if (request.getMethod() == "GET")
		getHandler(request, requestConf);
	else if (request.getMethod() == "POST") 
		postHandler(request, requestConf);
	else if (request.getMethod() == "DELETE") 
		deleteHandler(request, requestConf);
	else
		throw std::runtime_error("Error with response: Method - " + request.getMethod());
}


void	Response::htmlResponder() {
	int op = 0;
	ssize_t readbytes = 1;

	char buffer[10000]; // consider using a dynamic approach or a larger buffer

	op = open(_usePath.c_str(), O_RDONLY);
	if (op == -1) {
		if (errno == ENOENT)
			_code = (404);  // Not Found
		else if (errno == EACCES)
			_code = (403);  // Forbidden
		else
			_code = (500);  // Internal Server Error
		_responseType = 2;	// useful?
		_mimeType = "text/html"; // useful?
		return;
	}
	while (readbytes > 0) {
		memset(buffer, 0, sizeof(buffer));
		readbytes = read(op, buffer, sizeof(buffer));
		_response.append(buffer, readbytes);
	}
	if (readbytes == -1) {
		_code = (500);  // Internal Server Error
		_responseType = 2;	// useful?
		_mimeType = "text/html"; // useful?
		return;
	}

	_code = (200);

};



std::string	Response::findExecutablePath(const std::string& command, char** env) {
    std::string pathString;

    // Extract PATH from env
    while (*env) {
        std::string tmp(*env);
        if (tmp.substr(0, 5) == "PATH=") {
            pathString = tmp.substr(5); // Exclude "PATH=" part
            break;
        }
        env++;
    }

    if (pathString.empty())	{
        // Couldn't find PATH variable
        return "";
    }

    // Search for the executable in each directory listed in PATH
    int start = 0;
    int end = 0;
    while (end != -1) {
        end = pathString.find(":", start);
        std::string testPath = pathString.substr(start, (end == -1 ? pathString.length() : end) - start) + "/" + command;
        start = end + 1;

        struct stat sb;
        if ((stat(testPath.c_str(), &sb) == 0 && (sb.st_mode & S_IXUSR))) {
            // Found the executable
            return testPath;
        }
    }
    // Couldn't find the executable
    return "";
}


// Methods
void			Response::getHandler(Request &request, MasterSocket &requestConf) {
	ResponseHeader	head;
	head.setKeepAlive(this->_keepAlive);

	std::string phpPath = findExecutablePath("python", _env);

	if (phpPath.empty()) {
		std::cerr << RED << "Couldn't locate PHP." << RESET << std::endl;	// throw an error?
		_code = 500;
	}

	if ((_usePath.size() > 3 && !_usePath.substr(_usePath.size() - 3).compare(".py")))	{
		CgiHandler	cgi(request, requestConf);
		size_t		i = 0;
		size_t		j = _response.size() - 2;

		_response = cgi.executeCgi(phpPath);
		while (_response.find("\r\n\r\n", i) != std::string::npos || _response.find("\r\n", i) == i) {
			std::string	str = _response.substr(i, _response.find("\r\n", i) - i);
			if (str.find("Status: ") == 0)
				_code = std::atoi(str.substr(8, 3).c_str());
			else if (str.find("Content-type: ") == 0)
				_mimeType = str.substr(14, str.size());
			i += str.size() + 2;
		}
		while (_response.find("\r\n", j) == j)
			j -= 2;

		_response = _response.substr(i, j - i);
	}
	else if (isFile(_usePath))
		htmlResponder();
	else if  (_code == 200)	{
		std::stringstream	buffer;
		buffer << getPage(_usePath.c_str(), to_string(_hostPort.host), _hostPort.port, request.getPath());
		_response = buffer.str();
		_mimeType = "text/html";
	}
	else {
		_mimeType = "text/html";
		_response = this->readHtml(_errorMap[_code]);
	}
	if (_response.size() > 5000000)
		_code = 403;
	if (_code == 500 || _code == 403) {
		std::cout << BLUE << "_response.size() 2" << _response.size() << RESET << std::endl;
		_mimeType = "text/html";
		_response = this->readHtml(_errorMap[_code]);
	}
	_response = head.getHeader(_response.size(), _path, _code, _mimeType, request.getPath()) + "\r\n" + _response;
}

std::string handlePostResponse(std::string phpResponse) {
	std::string httpResponse;
	std::string imagePath;
	std::string nameLastname;

	size_t pos = phpResponse.find("SUCCESS:");
	if (pos != std::string::npos) {
		imagePath = phpResponse.substr(pos + 9);  // +9 to skip "SUCCESS:"
		imagePath = imagePath.substr(0, imagePath.find('\n'));  // Get only the path part
		nameLastname = phpResponse.substr(imagePath.size() + pos + 9); // +9 to account for "SUCCESS:" and newline
		imagePath = imagePath.substr(imagePath.find("post/"), imagePath.size());
		//imagePath = imagePath.substr(59); 
	}
	//std::cout << phpResponse << std::endl;
	std::cout << imagePath << std::endl;

	httpResponse += "<!DOCTYPE html>\n";
	httpResponse += "<html lang=\"en\">\n";
	httpResponse += "<head>\n";
	httpResponse += "<meta charset=\"UTF-8\">\n";
	httpResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	httpResponse += "<title>Upload Successful</title>\n";
	httpResponse += "</head>\n";
	httpResponse += "<body>\n";
	httpResponse += "<h1>" + nameLastname + "</h1>\n";  // Display the name and lastname
	httpResponse += "<p>Your post was successful!</p>\n";
	httpResponse += "<img src=\"" + imagePath + "\" alt=\"\">\n";  // Display the image
	//  <img src="files/image.jpeg" alt="">
	httpResponse += "</body>\n";
	httpResponse += "</html>\n";
	return httpResponse;
}

void			Response::postHandler(Request & request, MasterSocket & requestConf)
{
	ResponseHeader	head;
	head.setKeepAlive(this->_keepAlive);

	if (request.getBody().size() > 0) {
		CgiHandler	cgi(request, requestConf);
		size_t		i = 0;
		size_t		j = _response.size() - 2;

		// Use php-cgi as executable, not php!
		std::string phpCgiPath = findExecutablePath("python", _env);

		// std::cout << "test : "<< phpCgiPath << "fin de test" << std::endl;

		if (phpCgiPath.empty())
			std::cerr << RED << "Couldn't locate PHP." << RESET << std::endl;	// throw an error?
		_mimeType = "text/html";

		_response = cgi.executeCgi(phpCgiPath);

		while (_response.find("\r\n\r\n", i) != std::string::npos || _response.find("\r\n", i) == i)
		{
			std::string	str = _response.substr(i, _response.find("\r\n", i) - i);
			if (str.find("Status: ") == 0)
				_code = std::atoi(str.substr(8, 3).c_str());
			else if (str.find("Content-Type: ") == 0)
				_mimeType = str.substr(14, str.size());
				i += str.size() + 2;
		}
		while (_response.find("\r\n", j) == j)
			j -= 2;

		_response = _response.substr(i, j - i);
	}
	else {
		_mimeType = "text/html";
		_code = 204;	// empty response
		_response = "";
	}
	if (_code == 500)
		_response = this->readHtml(_errorMap[_code]);
	_response = handlePostResponse(_response);

	_response = head.getHeader(_response.size(), _path, _code, _mimeType, request.getPath()) + "\r\n" + _response;
}

std::string handleDeleteResponse(std::string path) {
	std::string httpResponse;

	httpResponse += "<!DOCTYPE html>\n";
	httpResponse += "<html lang=\"en\">\n";
	httpResponse += "<head>\n";
	httpResponse += "<meta charset=\"UTF-8\">\n";
	httpResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
	httpResponse += "<title>Delete Successful</title>\n";
	httpResponse += "<link rel=\"stylesheet\" href=\"/css/styles.css\">\n";
	httpResponse += "</head>\n";
	httpResponse += "<body>\n";
	httpResponse += "<div class=\"container\">\n";
	httpResponse += "<h1>" + path + "</h1>\n";  // Display the name and lastname
	httpResponse += "<p>was successfully deleted!</p>\n";
	httpResponse += "<a href=\"/\">Go back to the home page</a>\n";
	httpResponse += "</div\n";
	httpResponse += "</body>\n";
	httpResponse += "</html>\n";
	return httpResponse;
}

void Response::deleteHandler(Request &request, MasterSocket &requestConf) {
	ResponseHeader	head;
	head.setKeepAlive(this->_keepAlive);
	(void)request;
	(void)requestConf;

	_response = "";


	if (pathIsFile(fullLocalPath)) {
		if (remove(fullLocalPath.c_str()) == 0) {
			_response = handleDeleteResponse(fullLocalPath);
			_code = 200;
		}
		else
			_code = 403;
	}
	else if (isDirectory(fullLocalPath)) {
		_code = 403;
	}
	else {
		std::cout << "fullLocalPath = " << fullLocalPath << std::endl;
		_code = 404;
	}
	if (_code == 403 || _code == 404)
		_response = this->readHtml(_errorMap[_code]);
	_response = head.getHeader(_response.size(), fullLocalPath, _code, _mimeType, request.getPath()) + "\r\n" + _response;
}

std::string		Response::readHtml(const std::string& path) {
	std::ofstream		file;
	std::stringstream	buffer;

	if (pathIsFile(path)) {
		file.open(path.c_str(), std::ifstream::in);
		if (file.is_open() == false)
			return ("<!DOCTYPE html>\n<html><title>404</title><body>There was an error finding your error page</body></html>\n");

		buffer << file.rdbuf();
		file.close();
		_mimeType = "text/html";

		return (buffer.str());
	}
	else
		return ("<!DOCTYPE html>\n<html><title>404</title><body>There was an error finding your error page</body></html>\n");
}

// Getter functions

std::string		Response::getResponse(void) {
	return (_response);
}

// Overloaders

Response & Response::operator=(const Response & src) {
	_response = src._response;
	_path = src._path;
	_code = src._code;
	return (*this);
}

// Constructors and destructors

Response::Response(char **env) : _env(env) {}

Response::Response(const Response & src) {
	*this = src;
}

Response::~Response(void) {}


// for autoindex
std::string Response::getPage(const char *path, std::string const &host, int port, std::string basicUri) {
	std::string dirName(path);
	DIR *dir = opendir(path);
	std::string page =\
	"<!DOCTYPE html>\n\
	<html lang=\"en\">\n\
	<head>\n\
	<meta charset=\"UTF-8\">\n\
	<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\
	<title>" + dirName + "</title>\n\
	<link rel=\"stylesheet\" href=\"/css/styles.css\">\n\
	</head>\n\
	<body>\n\
	<header class=\"header\">\n\
	<div class=\"home-link\">\n\
	<a href=\"/\">HOME</a>\n\
	</div>\n\
	<nav>\n\
	<a href=\"../get/\">GET</a>\n\
	<a href=\"../post/\">POST</a>\n\
	<a href=\"../delete/\">DELETE</a>\n\
	<a href=\"../files/\">DIRECTORY LISTING</a>\n\
	</nav>\n\
	<div class=\"logo-link\">\n\
	<a href=\"../about/\">\n\
	<img src=\"../files/owllogo.jpg\" alt=\"Logo\">\n\
	</a>\n\
	</div>\n\
			</header>\n\
	<section class=\"container\">\n\
	<h1>INDEX</h1>\n\
	<h2>Check this Directory Listing, bro:</h2>\n\
	<ul>\n";

	if (dir == NULL) {
		std::cerr << RED << "Error: could not open [" << path << "]" << RESET << std::endl;
		return "";
	}
	if (dirName[0] != '/')
		dirName = "/" + dirName;
	for (struct dirent *dirEntry = readdir(dir); dirEntry; dirEntry = readdir(dir)) {
		if (std::string(dirEntry->d_name) != "." && std::string(dirEntry->d_name) != "..")
			page += getLink(std::string(dirEntry->d_name), dirName, host, port, basicUri);
	}
	page +="\
	</ul>\n\
	</section>\n\
	</body>\n\
	</html>\n";
	closedir(dir);
	return page;
}

std::string	Response::getLink(std::string const &dirEntry, std::string const &dirName, std::string const &host, int port, std::string basicUri) {
	std::stringstream   ss;

	(void)dirName;
	// Ensure the basicUri ends with a '/' before appending dirEntry
	if (basicUri[basicUri.length() - 1] != '/')	{
		basicUri += "/";
	}
	std::string betterHost = "127.0.0.1";
	(void)host;
	ss << "\t\t\t\t\t<li><a href=\"http://" + betterHost + ":" <<\
		port << basicUri + dirEntry + "\">" + dirEntry + "</a></li>\n";

	return ss.str();
}
