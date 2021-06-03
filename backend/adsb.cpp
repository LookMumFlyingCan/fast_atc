#include <iostream>
#include <cstring>
#include <string>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef unsigned char byte;

#include <unistd.h>
#include <backend/decoder.cpp>

#define BUFFER_SIZE 128

class Socket {
	private:
		int sock;

	public:
		Socket(int port) {
			addrinfo hints, *res;
			void *addr;
			char ipStr[INET6_ADDRSTRLEN];

		    memset(&hints, 0, sizeof(hints));

		    hints.ai_family   = AF_INET;
		    hints.ai_socktype = SOCK_STREAM;
		    hints.ai_flags    = AI_PASSIVE;

		    int g = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &res);
		    if(g != 0) {
		        std::cerr << gai_strerror(g) << "\n";
		        return;
		    }

		    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    		if(sock == -1) {
        		std::cerr << "error while creating socket\n";
        		freeaddrinfo(res);
        		return;
    		}

    		g = bind(sock, res->ai_addr, res->ai_addrlen);
    		if(g == -1) {
		        std::cerr << "error while binding socket\n";
		        close(sock);
		        freeaddrinfo(res);
		        return;
		    }

		    g = listen(sock, 8);
		    if(g == -1) {
		        std::cerr << "error while Listening on socket\n";
		        close(sock);
		        freeaddrinfo(res);
		        return;
		    }

		    std::cerr << "socket init done\n";
		}

		void loop() {
			bool ok = false;
			std::vector<byte> img;
			do {
				sockaddr_storage client_addr;
    			socklen_t client_addr_size = sizeof(client_addr);
				int conn = accept(sock, (sockaddr *) &client_addr, &client_addr_size);

				if(conn == -1){
					std::cerr << "failed to accept socket\n";
					continue;
				}

				std::vector<byte> buf(BUFFER_SIZE);
				int bytes = recv(conn, buf.data(), BUFFER_SIZE, 0);

				for(int i = BUFFER_SIZE - 1; i >= 0 && buf.back() == 0; i--)
					buf.pop_back();

				std::cerr << (*Decoder::squawk(buf)) << '\n';
			} while(true);
		}
};
