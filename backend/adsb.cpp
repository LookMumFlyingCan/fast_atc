#include <iostream>
#include <cstring>
#include <string>
#include <chrono>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <backend/decoder.cpp>

#include <backend/plane.cpp>

#define BUFFER_SIZE 128

using namespace std::chrono;

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

		void loop(std::mutex &access, std::map< std::vector<unsigned char>, plane, container_comp<std::vector<unsigned char>> > &store) {
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

				buf.resize(14);

				bool surv = true;

				for(int i = 13; i >= 7; i--)
					if(buf[i] != 0)
						surv = false;

				if(surv)
					buf.resize(7);

				if(!Decoder::icao(buf))
					continue;

				surv = false;

				access.lock();
				if(store.size() == 0)
					surv = true;

				auto plane = store[*Decoder::icao(buf)];

				store.erase(*Decoder::icao(buf));
				access.unlock();

				plane.last_pkg = system_clock::now();

				if(Decoder::altitude(buf))
					plane.altitude = Decoder::altitude(buf);

				if(Decoder::identification(buf))
					plane.ident = Decoder::identification(buf);

				if(Decoder::callsign(buf))
					plane.callsign = Decoder::callsign(buf);

				if(Decoder::displacement(buf))
					plane.velocity = Decoder::displacement(buf);

				if(Decoder::squawk(buf))
					plane.squawk = Decoder::squawk(buf);

				if(Decoder::parity(buf) && (*Decoder::parity(buf))){
					plane.last_even = make_pair(buf, duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
				} else if(Decoder::parity(buf) && !(*Decoder::parity(buf))) {
					plane.last_odd = make_pair(buf, duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
				}

				if(plane.last_odd && plane.last_even)
					plane.position = Decoder::position((*plane.last_odd).first, (*plane.last_even).first, (*plane.last_odd).second,  (*plane.last_even).second);


					access.lock();
					store[*Decoder::icao(buf)] = plane;
					access.unlock();

			} while(true);
		}
};
