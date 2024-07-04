#include "../Includes/Request.hpp"

// constructor
// receives the entire fetched http request string
Request::Request(const std::string& str) : _method (""), _version(""), _ret(200), _body(""), _port(80), _path(""), _query(""), _raw(str) {
	methods.push_back("GET");
	methods.push_back("POST");
	methods.push_back("DELETE");
	this->resetHeaders();
	// this->_env_for_cgi.clear();
	this->parse(str);
	if (this->_ret != 200 && str != "400")
		std::cerr << RED << "Parse error: " << this->_ret << RESET << std::endl;
}

Request::~Request() {}

/*** GETTERS ***/
const std::map<std::string, std::string>&	Request::getHeaders() const {
	return this->_headers;
}

const std::map<std::string, std::string>&	Request::getEnv() const {
	return this->_env_for_cgi;
}

const std::string&	Request::getMethod() const {
	return this->_method;
}

const std::string&	Request::getVersion() const {
	return this->_version;
}

int	Request::getRet() const {
	return this->_ret;
}

const std::string&	Request::getBody() const {
	return this->_body;
}

int	Request::getRequestPort() const {
	return this->_port;
}

const std::string& Request::getPath() const {
	return this->_path;
}

const std::string& Request::getQuery() const {
	return this->_query;
}

const std::string& Request::getRaw() const {
	return this->_raw;
}


/*** SETTERS ***/

void	Request::setBody(const std::string& str) {
	char	strip[] = {'\n', '\r'};

	this->_body.assign(str);
	for (int i = 0; i < 4; i++)
		if (this->_body.size() > 0 && this->_body[this->_body.size() - 1] == strip[i % 2])
			pop(this->_body);
		else
			break ;
}

void Request::setRet(int ret) {
	this->_ret = ret;
}

void Request::setMethod(const std::string &method) {
	this->_method = method;
}

void Request::resetHeaders() {
	this->_headers.clear();

	this->_headers["Accept-Charsets"] = "";
	this->_headers["Allow"] = "";
	this->_headers["Auth-Scheme"] = "";
	this->_headers["Authorization"] = "";
	this->_headers["Content-Length"] = "";
	this->_headers["Content-Location"] = "";
	this->_headers["Content-Type"] = "";
	this->_headers["Date"] = "";
	this->_headers["Host"] = "";
	this->_headers["Last-Modified"] = "";
	this->_headers["Location"] = "";
	this->_headers["Referer"] = "";
	this->_headers["Retry-After"] = "";
	this->_headers["Server"] = "";
	this->_headers["Transfer-Encoding"] = "";
	this->_headers["User-Agent"] = "";
	this->_headers["Www-Authenticate"] = "";
	this->_headers["Connection"] = "Keep-Alive";
}

int					Request::readFirstLine(const std::string& str) {
	size_t	i;
	std::string	line;

	i = str.find_first_of('\n');
	line = str.substr(0, i);
	i = line.find_first_of(' ');

	if (i == std::string::npos) {
		this->_ret = 400;
		std::cerr << RED << "ERROR: Wrong header line in HTTP Request" << RESET << std::endl;
		return 400;
	}
	this->_method.assign(line, 0, i);
	to_upper(this->_method);
	return this->readPath(line, i);
}

std::string decodeSpaces(const std::string& input) {
    std::string result;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (i < input.size() - 2 && input.substr(i, 3) == "%20") {
            result += ' ';
            i += 2;  // skip the next two characters
        } else
            result += input[i];
    }
    return result;
}

int Request::readPath(const std::string& line, size_t i) {
	size_t	j;

	if ((j = line.find_first_not_of(' ', i)) == std::string::npos) {
		this->_ret = 400;
		std::cerr << RED << "No PATH / HTTP version" << RESET << std::endl;
		return 400;
	}
	if ((i = line.find_first_of(' ', j)) == std::string::npos) {
		this->_ret = 400;
		std::cerr << RED << "No HTTP version" << RESET << std::endl;
		return 400;
	}
	this->_path.assign(line, j, i - j);
	_path = decodeSpaces(_path);
	return this->readVersion(line, i);
}

int Request::readVersion(const std::string& line, size_t i) {
	if ((i = line.find_first_not_of(' ', i)) == std::string::npos) {
		this->_ret = 400;
		std::cerr << RED << "No HTTP version" << RESET << std::endl;
		return 400;
	}
	if (line[i] == 'H' && line[i + 1] == 'T' && line[i + 2] == 'T' &&
			line[i + 3] == 'P' && line[i + 4] == '/')
		this->_version.assign(line, i + 5, 3);
	if (this->_version != "1.0" && this->_version != "1.1") {
		this->_ret = 400;
		std::cerr << RED << "BAD HTTP VERSION (" << this->_version << ")" << RESET << std::endl;
		return (this->_ret);
	}
	return (this->checkMethod());
}

int Request::checkMethod() {
	for (size_t i = 0; i < this->methods.size(); i++)
		if (this->methods[i] == this->_method)
			return this->_ret;
	std::cerr << RED << "Invalid method requested" << RESET << std::endl;
	this->_ret = 400;
	return this->_ret;
}

int Request::checkPort() {
	size_t i = this->_headers["Host"].find_first_of(':');

	if (i == std::string::npos)
		this->_port = 80;
	else {
		std::string tmp(this->_headers["Host"], i + 1);
		this->_port = std::atoi(tmp.c_str());
	}
	return (this->_port);
}

std::string Request::nextLine(const std::string &str, size_t& i) {
	std::string		ret;
	size_t			j;

	if (i == std::string::npos)
		return "";
	j = str.find_first_of('\n', i);
	ret = str.substr(i, j - i);
	if (ret[ret.size() - 1] == '\r')
		pop(ret);
	i = (j == std::string::npos ? j : j + 1);
	return ret;
}

int Request::parse(const std::string& str) {
	std::string		key;
	std::string		value;
	std::string		line;
	size_t			i(0);

	this->readFirstLine(nextLine(str, i));

	while ((line = nextLine(str, i)) != "\r" && line != "" && this->_ret != 400) { // parse headers line by line
		// handle each header line which are structured as follows: "Key: Value"
		key = readKey(line); // get capitalized 'KEY'
		value = readValue(line); // get 'Value' as received
		if (this->_headers.count(key)) // check in the headers map if there is such data a key and set it
				this->_headers[key] = value;
		if (key.find("secret") != std::string::npos)
			this->_env_for_cgi[formatHeaderForCGI(key)] = value; // create a new map element, with <HTTP_ + KEY, value>
	}
	if (this->_headers["www-authenticate"] != "")
		this->_env_for_cgi["www-authenticate"] = this->_headers["www-authenticate"];
	// set the _body attribute and trim the tailing \r\n chars

	if (str != "400") // check for body not an int
		this->setBody(str.substr(i, std::string::npos));

	this->findQuery();
	return this->_ret;
}

std::string Request::readKey(const std::string& line) {
	std::string	ret;

	size_t	i = line.find_first_of(':');
	ret.append(line, 0 , i);
	capitalize(ret);
	return (strip(ret, ' '));
}

std::string	Request::readValue(const std::string& line) {
	size_t i;
	std::string	ret;

	i = line.find_first_of(':');
	i = line.find_first_not_of(' ', i + 1);
	if (i != std::string::npos)
		ret.append(line, i, std::string::npos);
	return (strip(ret, ' ')); // trim leading and tailing spaces
}

void Request::stripAll() {
	strip(this->_method, '\n');
	strip(this->_method, '\r');
	strip(this->_method, ' ');
	strip(this->_version, '\n');
	strip(this->_version, '\r');
	strip(this->_version, ' ');
	strip(this->_path, '\n');
	strip(this->_path, '\r');
	strip(this->_path, ' ');
}

void Request::findQuery() {
	size_t		i;

	// at this point, _path includes the ? variables
	i = this->_path.find_first_of('?');

	// if there are some ? query variables, remove them from _path and save them to _query
	if (i != std::string::npos) {
		this->_query.assign(this->_path, i + 1, std::string::npos);
		this->_path = this->_path.substr(0, i);
	}
}

std::string Request::formatHeaderForCGI(std::string& key) {
	to_upper(key);
	for (size_t i = 0 ; i < key.size() ; i++) {
		if (key[i] == '-')
			key[i] = '_';
	}
	return "HTTP_" + key;
}