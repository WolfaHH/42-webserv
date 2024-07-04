- Main instanciate WebServer.
- WebServ is a children of the abstract class AServer.
- WebServ first calls AServer's constructor passing the config_file to it, and set the WebServer instance env variable to be the env passed to the main.
	- AServer split the server blocks of the config_file into a vector (maybe change the name to AServerBlocks)
	- Then creates the MasterSocket instances
		- MasterSocket instantiate the parent class Socket, responsible for parsing the blocks and binding the sockets.
		- Socket calls the construction of ServerConfig.

in default.conf "autoindex on;" means that if a directory is requested under the `/files` path, the server will not try to serve an index file and will instead generate a directory listing (because `autoindex` is `on`).