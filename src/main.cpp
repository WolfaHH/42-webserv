#include "../Includes/WebServer.hpp"

int main(int ac, char **av, char **env) {
	if (ac != 2) {
		std::cerr << std::endl << RED << BOLD << "################################# ERROR #################################" << RESET << std::endl;
		std::cerr << std::endl << "Webserv requires a config file --> ./webserv [config_file]" << std::endl << std::endl;
		exit(EXIT_FAILURE);
	}

	try {
		WebServer serv(av[1], env);
		(void)env;
	}

	catch (const std::runtime_error& e) {
		// Catch a specific exception type
		std::cerr << std::endl << RED << BOLD << "################################# ERROR #################################" << RESET << std::endl;
		std::cerr << "Caught runtime_error: " << e.what() << std::endl;
	}

	catch (const std::exception& e) {
		// Catch all standard exceptions
		std::cerr << std::endl << RED << BOLD << "################################# ERROR #################################" << RESET << std::endl;
		std::cerr << "Caught exception: " << e.what() << std::endl;
	}

	catch (...) {
		// Catch any other exceptions not previously caught
		std::cerr << std::endl << RED << BOLD << "################################# ERROR #################################" << RESET << std::endl;
		std::cerr << "Caught an unknown exception!" << std::endl;
	}
}
