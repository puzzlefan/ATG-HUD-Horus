#ifndef SERVER_H
#define SERVER_H

class Server
{
private:
	//Vars for the main thread
	int sockfd;
	uint16_t portno = 42000;
	struct sockaddr_in serv_addr;
	//Client Vector to handle multiple at the same time
	std::vector<socklen_t> SocketLengths;
	std::vector<struct sockaddr_in> ClientAddresses;
	std::vector<int> ClientFd;

	//threads
	std::thread *ServerMain;
	std::vector<std::thread> clientThreads;
public:
	Server();
	~Server();

	void ServerMainThread();//{return 1;}
	void ServerPrivateThread();

	void dataExchange();
};

#endif
