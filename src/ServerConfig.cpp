#include "../Includes/ServerConfig.hpp"

/* ----- Construction ----- */
ServerConfig::ServerConfig(std::string serverBlock) {
	serverConfigDomain = AF_INET;				// "Address Family - IPv4" |the socket will use the IPv4 addressing scheme
	serverConfigService = SOCK_STREAM;			// Let the system decide the appropriate protocol based on the socket type (in this case, TCP for SOCK_STREAM)
	serverConfigProtocol = 0;					// The operating system chooses the appropriate protocol based on the previous settings. Given AF_INET and SOCK_STREAM, the OS will typically choose TCP.
	serverConfigInterface = INADDR_ANY; // Accept connections on any IP address that the server can communicate on.

	serverConfigPort = 80;
	serverConfigHost = "127.0.0.1";
	serverConfigServerName = "localhost";
	serverConfigMaxBodySize = 1 * 1024 * 1024; // 1MB

	serverConfigErrorPages[201] = "./error_pages/201.html";
	serverConfigErrorPages[204] = "./error_pages/204.html";
	serverConfigErrorPages[400] = "./error_pages/400.html";
	serverConfigErrorPages[403] = "./error_pages/403.html";
	serverConfigErrorPages[404] = "./error_pages/404.html";
	serverConfigErrorPages[405] = "./error_pages/405.html";
	serverConfigErrorPages[408] = "./error_pages/408.html";
	serverConfigErrorPages[413] = "./error_pages/413.html";
	serverConfigErrorPages[500] = "./error_pages/500.html";
	serverConfigErrorPages[501] = "./error_pages/501.html";

	// Default methods
	serverConfigLocation.allow_methods.push_back("GET");
	serverConfigLocation.allow_methods.push_back("POST");
	serverConfigLocation.allow_methods.push_back("DELETE");
	serverConfigLocation.autoindex = true;
	serverConfigLocation.root = "./www";
	serverConfigLocation.route = "/";
	serverConfigLocation.index = "index.html";

	//parse, check, and attribute directives from the config file
	parseServerBlock(serverBlock);
}

ServerConfig::~ServerConfig() {}

/* ----- Parsing Control Tower ----- */
// Receives the entire server{} block and parses all directives and location{} blocks
// then checks for remaining unkown directive
void ServerConfig::parseServerBlock(std::string serverBlock) {
	std::string temp;
	if (containsDirective(serverBlock, "\nlisten "))
		serverConfigPort = extractPort(removeDirective(serverBlock,  "\nlisten "));
			
	if (containsDirective(serverBlock, "\nserver_name "))
		serverConfigServerName = extractServerName(removeDirective(serverBlock,  "\nserver_name "));
			
	if (containsDirective(serverBlock, "\nhost "))
		serverConfigHost = extractHost(removeDirective(serverBlock,  "\nhost "));
			
	if (containsDirective(serverBlock, "\nerror_page "))
		extractErrorPage(removeDirective(serverBlock,  "\nerror_page "));

	if (containsDirective(serverBlock, "\nclient_max_body_size "))
		serverConfigMaxBodySize = extractClientMaxBodySize(removeDirective(serverBlock,  "\nclient_max_body_size "));

	parseServerConfigLocation(serverBlock);
	parseServerConfigLocationsVector(serverBlock);

	// Remove the first line containing '{'
	size_t startPos = serverBlock.find('{');
	if (startPos != std::string::npos)
		serverBlock.erase(startPos, 1);

	// Remove the last line containing '}'
	size_t endPos = serverBlock.rfind('}');
	if (endPos != std::string::npos)
		serverBlock.erase(endPos, 1);

	// remove the line containing "server"
	if (containsDirective(serverBlock, "server"))
		removeDirective(serverBlock,  "server");

	// check if there is something else remaining in the config file
	if (serverBlock.find_first_not_of(" \t\n\r\f\v{}") != std::string::npos) {
		std::cout << serverBlock << std::endl; // to test
		throw std::runtime_error("Unknown directive(s) --> " + serverBlock);
	}
}

/* ----- Directives ----- */
// check if the string contains the directive
// returns 
// - true if it finds the string once
// - false if it doesn't
// - throws an error if it finds the directive more than once
bool ServerConfig::containsDirective(const std::string &serverBlock, const std::string &directive) {
	int		occurrences = 0;
	size_t	pos = 0;

	while ((pos = serverBlock.find(directive, pos)) != std::string::npos) {
		++occurrences;
		pos += directive.length(); // Move past the directive that was just found.
	}
	if (occurrences == 1)
		return (true);
	else if (occurrences > 1) {
		std::string ret = directive;
		if (ret[0] == '\n')
			ret.erase(ret.begin());
		throw std::runtime_error("Directive declared more than once: " + ret);
	} else
		return (false);
}

// a function that removes the line containing the directive once it has been found
// it returns the line in order to extract, check, and save its content
std::string ServerConfig::removeDirective(std::string &serverBlock, const std::string &directive)
{
	size_t pos = serverBlock.find(directive);

	if (pos != std::string::npos)
	{
		size_t startOfLine = pos++; // Start after the previous newline character
		size_t endOfLine = serverBlock.find('\n', pos + directive.length()); // Find the next newline character

		std::string extractedLine = serverBlock.substr(startOfLine, endOfLine - startOfLine);
		serverBlock.erase(startOfLine, endOfLine - startOfLine);
		return (extractedLine);
	}

	return (""); // Return an empty string if the directive is not found
}


/* ----- Extractions ----- */
// extract & check the client_max_body_size from the line
int ServerConfig::extractClientMaxBodySize(std::string line)
{
	std::string bodySize = line.substr(21);  // skip "client_max_body_size "

	bodySize = trimWhiteSpaces(bodySize);

	// Check if ends with semicolon
	if (!endsWithSemicolon(bodySize))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	bodySize.pop_back();

	bodySize = trimWhiteSpaces(bodySize);  // trim any more potential whitespace

	if (!isNumber(bodySize))
		throw std::runtime_error("Invalid client_max_body_size specification: not a number");
	
	int res = std::stoi(bodySize);
	
	return (res);
}

// extract & check the error_pages from the line
void ServerConfig::extractErrorPage(std::string line)
{
	std::string errorStr = line.substr(11);  // skip "error_page "

	errorStr = trimWhiteSpaces(errorStr);

	// Check if ends with semicolon
	if (!endsWithSemicolon(errorStr))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	errorStr.pop_back();
	errorStr = trimWhiteSpaces(errorStr);  // trim any more potential whitespace

	// Split the string into a vector of strings 
	std::vector<std::string> tokens = splitToVector(errorStr);

	if (tokens.size() % 2 != 0)  // Ensure we have pairs
		throw std::runtime_error("Mismatched error codes and paths in error_page directive.");

	for (size_t i = 0; i < tokens.size(); i += 2) 
	{
		std::string errorCodeStr = tokens[i];
		std::string path = tokens[i + 1];

		if (!isNumber(errorCodeStr))
			throw std::runtime_error("Invalid error code in error_page directive: " + errorCodeStr);

		int errorCode = std::atoi(errorCodeStr.c_str());

		if (errorCode != 400 && errorCode != 403 && errorCode != 404 \
			&& errorCode != 405 && errorCode != 408 && errorCode != 410 && errorCode != 413 && errorCode != 500 && errorCode != 501) 
			throw std::runtime_error("Unsupported error code in error_page directive: " + errorCodeStr);

		// Validate the path if required.
		if (access(path.c_str(), F_OK) != 0)  // Check if file exists
			throw std::runtime_error("Specified error page does not exist: " + path);

		// Store the valid error code and path into a map<>
		serverConfigErrorPages[errorCode] = path;
	}

	// std::cout << errorStr << std::endl;
}

// extract & check the port from the line
int ServerConfig::extractPort(std::string line)
{
	std::string portString = line.substr(7);  // skip "listen "

	portString = trimWhiteSpaces(portString);

	// Check if ends with semicolon
	if (!endsWithSemicolon(portString))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	portString.pop_back();
	portString = trimWhiteSpaces(portString);  // trim any more potential whitespace

	if (!isNumber(portString))
		throw std::runtime_error("Invalid port specification: not a number");
	
	int port = std::stoi(portString);
	
	if (!isValidPort(port))
		throw std::runtime_error("Invalid port number: out of range");

	// std::cout << port << std::endl;

	return (port);
}

// extract & check the host from the line
std::string ServerConfig::extractHost(std::string line)
{
	std::string host = line.substr(6);  // skip "host "

	host = trimWhiteSpaces(host);

	// Check if ends with semicolon
	if (!endsWithSemicolon(host))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	host.pop_back();
	host = trimWhiteSpaces(host);  // trim any more potential whitespace
	
	if (!isValidHost(host))
		throw std::runtime_error("Invalid host number.");


	return (host);
}

// extract the server_name from the line
std::string ServerConfig::extractServerName(std::string line) {
	std::string server_name = line.substr(12);  // skip "server_name "

	server_name = trimWhiteSpaces(server_name);

	// Check if ends with semicolon
	if (!endsWithSemicolon(server_name))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	server_name.pop_back();
	server_name = trimWhiteSpaces(server_name);  // trim any more potential whitespace

	return (server_name);
}


/* ----- Locations Parsing ----- */
// Look for a root location {} block, check that there's only one, and modify the rootLocation struct if needed
void ServerConfig::parseServerConfigLocation(std::string &serverBlock)
{
	std::string::size_type startPos = 0, endPos = 0;
	int root_count = 0; // count the number of root location in the block

	while ((startPos = serverBlock.find("\nlocation ", startPos)) != std::string::npos)
	{
		endPos = startPos + std::strlen("location ");;  // move past "location "

		// skip white spaces, until the "/"
		while (endPos < serverBlock.size() && isspace(serverBlock[endPos]))
   			++endPos;

		// check that the first char is a "/"
		if (endPos >= serverBlock.size() || serverBlock[endPos] != '/')
			throw std::runtime_error("Error: wrong location path");

		++endPos;

		// skip white spaces again, until the "{"
		while (endPos < serverBlock.size() && isspace(serverBlock[endPos]))
			++endPos;

		// If any other char than "{" is found --> not the root location
		if (endPos >= serverBlock.size() || serverBlock[endPos] != '{')
		{
			startPos = endPos;
			continue;
		}

		// throw an error if a root location was already found
		if (root_count > 0)
			throw std::runtime_error("Error: too many root locations!");
	
		root_count++;
		int bracketCount = 1;
		endPos++;
		
		// look for ending matching "}"
		while (bracketCount > 0 && endPos < serverBlock.size())
		{
			if (serverBlock[endPos] == '{')
				bracketCount++;
			else if (serverBlock[endPos] == '}')
				bracketCount--;
			endPos++;
		}
		
		// add the location {} block to the vector if bracketCount == 0
		// otherwise send an error
		if (bracketCount == 0)
			serverConfigLocation = createLocation(serverBlock.substr(startPos, endPos - startPos));
		else
			throw std::runtime_error("Error: Unmatched brackets in config file!");

		// std::string locationBlock = serverBlock.substr(startPos, endPos - startPos + 1);

		// erase the location {} block
		serverBlock.erase(startPos, endPos - startPos);
	}
}

// find all location {} blocks which are not root location and create a location struct from them
void ServerConfig::parseServerConfigLocationsVector(std::string &serverBlock)
{
	std::string::size_type startPos = 0, endPos = 0;

	while ((startPos = serverBlock.find("\nlocation ", startPos)) != std::string::npos)
	{
		endPos = startPos + 9;  // move past "location "

		// skip all white space until "/"
		while ((serverBlock[endPos] == ' ' || serverBlock[endPos] == '\n' || serverBlock[endPos] == '\t') && endPos < serverBlock.size())
			endPos++;
		
		// Check if the first char is a "/"
		if (!serverBlock[endPos] || serverBlock[endPos] != '/')
			throw std::runtime_error("Error: problem with location in conf file!");

		// from the "/" to the "{", we have the URL route
		while ((serverBlock[endPos] != '{') && endPos < serverBlock.size())
			endPos++;

		// throw an error if no "{" was found
		if (endPos >= serverBlock.size())
			throw std::runtime_error("Error: problem with location in conf file!");

		int bracketCount = 1;
		endPos++;
		
		// look for ending matching "}"
		while (bracketCount > 0 && endPos < serverBlock.size())
		{
			if (serverBlock[endPos] == '{')
				bracketCount++;
			else if (serverBlock[endPos] == '}')
				bracketCount--;
			endPos++;
		}
		
		// add the location{} block to the vector if bracketCount == 0
		// otherwise send an error
		if (bracketCount == 0)
			serverConfigLocationsVector.push_back(createLocation(serverBlock.substr(startPos, endPos - startPos)));
		else
			throw std::runtime_error("Error: Unmatched brackets in config file!");
		
		//std::string locationBlock = serverBlock.substr(startPos, endPos - startPos + 1);
		
		// erase the location {} block
		serverBlock.erase(startPos, endPos - startPos);

	}
}

// create a Location struct from the location {} block in the config file
// checks for index, root, path, autoindex and allow_methods
// if one of them is missing, the location attribute is set to default
struct Location ServerConfig::createLocation(std::string locStr)
{
	Location loc;

	std::string path = locStr.substr(9);  // skip "location "

	size_t startPos = locStr.find('/'); // the path (route) MUST start with a "/"
	size_t endPos = locStr.find('{', startPos);

	std::string extractedPath = locStr.substr(startPos, endPos - startPos);

	// std::cout << "str = ." << extractedPath << ".\n";
	while (isspace(extractedPath.back()))
		extractedPath.pop_back();
	// std::cout << "str = ." << extractedPath << ".\n";

	// Check if there is any white space in the route
	if (extractedPath.find_first_of(" \t\n\r\f\v") != std::string::npos)
		throw std::runtime_error("CONFIG ERROR: Wrong location path:" + extractedPath);
	
	loc.route = extractedPath;
	if (containsDirective(locStr, "\nredirect "))
	{
		Location redirect = extractRedirect(removeDirective(locStr,  "\nredirect "));

		loc.root = redirect.root;
		loc.autoindex = redirect.autoindex;
		loc.allow_methods = redirect.allow_methods;
		loc.index = redirect.index;
	}
	else
	{
		// check for the root directive, otherwise use the default root
		if (containsDirective(locStr, "\nroot "))
		{
			std::string tempRoot = extractRoot(removeDirective(locStr,  "\nroot "));

			// remove a "/" if both the tempRoot and the extractedPath already have one
			if (!tempRoot.empty() && tempRoot[tempRoot.size() - 1] == '/' && !extractedPath.empty() && extractedPath[0] == '/')
				tempRoot.pop_back();

			// add a "/" if neither the tempRoot nor the extractedPath have one
			else if (!tempRoot.empty() && tempRoot[tempRoot.size() - 1] != '/' && !extractedPath.empty() && extractedPath[0] != '/')
				tempRoot = tempRoot + '/';

			loc.root = tempRoot + extractedPath;
		}
		else
		{	
			// remove a "/" if both the rootLocation.root and the extractedPath already have one
			if (!serverConfigLocation.root.empty() && serverConfigLocation.root[serverConfigLocation.root.size() - 1] == '/' && !extractedPath.empty() && extractedPath[0] == '/')
				serverConfigLocation.root.pop_back();

			// add a "/" if neither the rootLocation.root nor the extractedPath have one
			else if (!serverConfigLocation.root.empty() && serverConfigLocation.root[serverConfigLocation.root.size() - 1] != '/' && !extractedPath.empty() && extractedPath[0] != '/')
			serverConfigLocation.root = serverConfigLocation.root + '/';

			loc.root = serverConfigLocation.root + extractedPath;
		}

		// check for the autoindex directive, otherwise use the default autoindex
		if (containsDirective(locStr, "\nautoindex "))
			loc.autoindex = extractAutoindex(removeDirective(locStr,  "\nautoindex "));
		else
			loc.autoindex = serverConfigLocation.autoindex;

		// check for the allow_methods directive, otherwise use the default allow_methods
		if (containsDirective(locStr, "\nallow_methods "))
			loc.allow_methods = extractAllowMethods(removeDirective(locStr,  "\nallow_methods "));
		else
			loc.allow_methods = serverConfigLocation.allow_methods;

		// check for the index directive, otherwise use the default index
		if (containsDirective(locStr, "\nindex "))
			loc.index = extractIndex(removeDirective(locStr,  "\nindex "), loc.root);
		else
			loc.index = serverConfigLocation.index;	}
	
	// remove the line containing "location"
	if (containsDirective(locStr, "location"))
		removeDirective(locStr,  "location");

	// check if there is something else remaining in the config file
	if (locStr.find_first_not_of(" \t\n\r\f\v{}") != std::string::npos)
	{
		std::cout << locStr << std::endl; // to test
		throw std::runtime_error("Wrong location directive(s) --> " + locStr);
	}
	return (loc);
}

// extract redirection
struct Location ServerConfig::extractRedirect(std::string line)
{
	std::string redirect = line.substr(9);  // skip "redirect "

	redirect = trimWhiteSpaces(redirect);

	// Check if ends with semicolon
	if (!endsWithSemicolon(redirect))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	redirect.pop_back();
	redirect = trimWhiteSpaces(redirect);  // trim any more potential whitespace

	// if there is any white character within the remaining string, the path can't be valid
	if (redirect.find_first_of(" \t\n\r\f\v") != std::string::npos)
		throw std::runtime_error("CONFIG ERROR: Wrong redirection path: " + redirect);
	
	for (size_t i = 0; i < serverConfigLocationsVector.size(); i++)
	{
		if (serverConfigLocationsVector[i].route == redirect)
			return serverConfigLocationsVector[i];
	}

	// std::cout << redirect << std::endl;

	throw std::runtime_error("CONFIG ERROR: Unset redirection path: " + redirect);
}

// extract & check the root from the line
std::string ServerConfig::extractRoot(std::string line)
{
	std::string root = line.substr(5);  // skip "root "

	root = trimWhiteSpaces(root);

	// Check if ends with semicolon
	if (!endsWithSemicolon(root))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	root.pop_back();
	root = trimWhiteSpaces(root);  // trim any more potential whitespace

	// if there is any white character within the remaining string, the path can't be valid
	if (root.find_first_of(" \t\n\r\f\v") != std::string::npos)
		throw std::runtime_error("CONFIG ERROR: Wrong location path: " + root);
	
	if (access(root.c_str(), F_OK) != 0)  // Check if file exists
		throw std::runtime_error("Specified root does not exist: " + root);

	// std::cout << root << std::endl;

	return (root);
}

// extract & check autoindex from the line
bool ServerConfig::extractAutoindex(std::string line)
{
	std::string autoindex = line.substr(10);  // skip "autoindex "

	autoindex = trimWhiteSpaces(autoindex);

	// Check if ends with semicolon
	if (!endsWithSemicolon(autoindex))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	autoindex.pop_back();
	autoindex = trimWhiteSpaces(autoindex);  // trim any more potential whitespace

	// std::cout << autoindex << std::endl;

	// autoindex can either be on or off
	if (autoindex == "on")
		return true;
	else if (autoindex == "off")
		return false;
	else
		throw std::runtime_error("CONFIG ERROR: Wrong autoindex: " + autoindex);
}

// extract & check allow_methods from the line
std::vector<std::string> ServerConfig::extractAllowMethods(std::string line)
{
	std::string allow_methods = line.substr(14);  // skip "allow_methods "
	std::vector<std::string> result;

	allow_methods = trimWhiteSpaces(allow_methods);

	// Check if ends with semicolon
	if (!endsWithSemicolon(allow_methods))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	allow_methods.pop_back();
	allow_methods = trimWhiteSpaces(allow_methods);  // trim any more potential whitespace
	
	// Split the string into a vector of strings
	std::vector<std::string> methods = splitToVector(allow_methods);

	// check each substring for an accepted method
	for(size_t i = 0; i < methods.size(); i++)
	{
		if (methods[i] == "GET")
			result.push_back("GET");
		else if (methods[i] == "POST")
			result.push_back("POST");
		else if (methods[i] == "DELETE")
			result.push_back("DELETE");
		else
			throw std::runtime_error("Unknown method: " + methods[i]);
	}

	// std::cout << allow_methods << std::endl;

	return (result);
}

// extract & check the root from the line
std::string ServerConfig::extractIndex(std::string line, std::string path)
{
	std::string index = line.substr(6);  // skip "index "

	index = trimWhiteSpaces(index);

	// Check if ends with semicolon
	if (!endsWithSemicolon(index))
		throw std::runtime_error("Directive does not end with a semicolon");

	// Remove the semicolon for parsing
	index.pop_back();
	index = trimWhiteSpaces(index);  // trim any more potential whitespace

	// if there is any white character within the remaining string, the path can't be valid
	if (index.find_first_of(" \t\n\r\f\v") != std::string::npos)
		throw std::runtime_error("CONFIG ERROR: Wrong index file name: " + index);
	
	// remove a "/" if both the path and the index already have one
	if (!path.empty() && path[path.size() - 1] == '/' && !index.empty() && index[0] == '/')
		path.pop_back();

	// add a "/" if neither the path nor the index have one
	else if (!path.empty() && path[path.size() - 1] != '/' && !index.empty() && index[0] != '/')
		path = path + '/';

	std::string checkAccess = path + index;
	
	// std::cout << "access = " << checkAccess << "PATH = " << path << " index = " << index << std::endl;
	if (access(checkAccess.c_str(), F_OK) != 0)  // Check if file exists --> 404 error
		throw std::runtime_error("Specified index file does not exist: " + checkAccess);
	return (index);
}





// --------------- GETTERS & SETTERS ---------------
int ServerConfig::getServerConfigPort() {
	return (this->serverConfigPort);
}

int ServerConfig::getServerConfigDomain() {
	return (this->serverConfigDomain);
}

std::string ServerConfig::getServerConfigHost() {
	return (this->serverConfigHost);
}

std::map<int, std::string> ServerConfig::getServerConfigErrorPages() {
	return (this->serverConfigErrorPages);
}

int ServerConfig::getServerConfigMaxBodySize() {
	return (this->serverConfigMaxBodySize);
}

Location ServerConfig::getServerConfigLocation() {
	return (this->serverConfigLocation);
}

std::vector<Location> ServerConfig::getServerConfigLocationsVector() {
	return (this->serverConfigLocationsVector);
}

int ServerConfig::getServerConfigService() {
	return (this->serverConfigService);
}

int ServerConfig::getServerConfigProtocol() {
	return (this->serverConfigProtocol);
}

u_long ServerConfig::getServerConfigInterface() {
	return (this->serverConfigInterface);
}

std::string ServerConfig::getEndpoint() {
	std::string res = this->serverConfigHost + ':' + std::to_string(this->serverConfigPort);
	return (res);
}