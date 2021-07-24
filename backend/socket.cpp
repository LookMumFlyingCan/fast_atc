#include <iostream>
#include <cstring>
#include <string>
#include <chrono>
#include <iomanip>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <backend/decoder.cpp>
#include <cmath>
#include <SFML/Network.hpp>
#include "backend/telemetry.h"

#define BUFFER_SIZE 128

using namespace std::chrono;

class Socket {
	private:
		sf::TcpListener lis;

	public:
		Socket(int port) {
			if(lis.listen(port) != sf::Socket::Done){
				std::cerr << "failed to listen\n";
			}

		    std::cerr << "socket init done\n";
		}		

		void loop(std::mutex &access, std::map< std::vector<unsigned char>, plane, container_comp<std::vector<unsigned char>> > &store, std::mutex &sat_access, sat_status &status) {
			bool ok = false;
			std::vector<byte> img;
			sf::TcpSocket client;
			do {
				if(lis.accept(client) != sf::Socket::Done){
					std::cerr << "failed to accept connection\n";
				}

				byte bufraw[BUFFER_SIZE];
				size_t bytes; unsigned short portd;
				sf::IpAddress sender;
				if(client.receive(bufraw, BUFFER_SIZE, bytes) != sf::Socket::Done)
					std::cerr << "failed ro recv data\n";


				std::vector<byte> buf(bufraw, bufraw + BUFFER_SIZE);

				std::cout << std::hex << std::setw(2) << std::setfill('0');

				if(buf[0] == '*') {
					buf.erase(buf.begin());

					buf.resize(14);

					for(int i = 0; i < 14; i++){
						std::cout << (int)buf[i];
					}
					std::cout << '\n';

					bool surv = true;

					for(int i = 13; i >= 7; i--)
						if(buf[i] != 0)
							surv = false;

					if(surv)
						buf.resize(7);

					if(!Decoder::icao(buf))
						continue;

					surv = false;

					if(!Decoder::verifyIntegrity(buf) || !(*Decoder::verifyIntegrity(buf)))
						continue;
					

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
						plane.last_even[*Decoder::bsgs(buf)] = make_pair(buf, duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
					} else if(Decoder::parity(buf) && !(*Decoder::parity(buf))) {
						plane.last_odd[*Decoder::bsgs(buf)] = make_pair(buf, duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
					}

					for(auto bsgs : {0,1}){
						if(plane.last_odd[bsgs] && plane.last_even[bsgs]) {
							plane.position = Decoder::position((*plane.last_odd[bsgs]).first, (*plane.last_even[bsgs]).first, (*plane.last_odd[bsgs]).second,  (*plane.last_even[bsgs]).second);
						}
					}

					access.lock();
					store[*Decoder::icao(buf)] = plane;
					access.unlock();
				} else if (buf[0] == 'B'){
					auto newt = reinterpret_cast<telemetry*>(&buf[0]);
					sat_access.lock();
					status.tel = *newt;
					sat_access.unlock();
				} else if (buf[0] == 'G') {
					auto newg = reinterpret_cast<gps*>(&buf[0]);
					newg->longitude = std::floor(newg->longitude / 100) + (std::fmod(newg->longitude, 100) / 60);
					newg->latitude = std::floor(newg->latitude / 100) + (std::fmod(newg->latitude, 100) / 60);

					std::cout << newg->longitude << ' ' << newg->latitude << '\n';
					sat_access.lock();
					status.pos = *newg;
					sat_access.unlock();
				} else {
					std::cerr << "invalid packet begg" << buf[0] << "\n";
				}

			} while(true);
		}
};
