#include <vector>
#include <optional>
#include <cmath>
#include <algorithm>

#include <backend/properties.cpp>

#define BUFFER_SIZE 128

const byte generator[4] = {0b11111111, 0b11111010, 0b00000100, 0b10000000};
const std::string lookup = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ##### ###############0123456789######";

const int nz = 15;

class Decoder {
	private:
		static long long NL(long double lat){
			if(lat <= 1e-05 && lat >= -1e-5)
				return 0;
			if(abs(lat) <= 87 + 1e-05 && abs(lat) >= 87 - 1e-5)
				return 2;
			if(lat > 87 || lat < -87)
				return 1;

			return std::floor(
					2*M_PIl
						/						std::acos(
							1 - ((1 - std::cos(M_PIl/(2*nz)))/(std::pow(std::cos((M_PIl/180) * std::fabs(lat)), 2)))
						)
				);
		}

		static int cyclicCheck(std::vector<byte> &data, bool leave = false){
			std::vector<bool> crc;

			for(int i = 0; i < data.size(); i++)
				for(int j = 0; j < 8; j++)
					crc.push_back(data[i] & (1 << (7 - j)));

			if(leave)
				for(int i = 0; i < 24; i++)
					crc[crc.size() - 1 - i] = false;

			for(int i = 0; i < crc.size() - 24; i++)
				if(crc[i])
					for(int j = 0; j < 25; j++)
						crc[i + j] = crc[i + j] ^ ((generator[j/8] & (1 << (7 - (j % 8)))) > 0);

			int res = 0;
			for(int i = 0; i < 24; i++)
				res += crc[crc.size() - 24 + i] * (1 << (23 - i));

			return res;
		}

	public:
		static byte downlinkFormat(std::vector<byte> &data){
			return data.size() == 0 ? -1 : data[0] >> 3;
		}

		static std::optional<bool> verifyIntegrity(std::vector<byte> &data){
			auto df = Decoder::downlinkFormat(data);

			return (df == 11 || df == 17 || df == 18) ? std::optional<bool>(cyclicCheck(data) == 0) : std::optional<bool>(std::nullopt);
		}

		static std::optional<byte> typecode(std::vector<byte> &data){
			auto df = Decoder::downlinkFormat(data);

			return  (df == 17 || df == 18) ?
				std::optional<byte>(data[4] >> 3) : std::optional<byte>(std::nullopt);
		}

		static std::optional<std::vector<byte>> icao(std::vector<byte> &data){
			auto df = Decoder::downlinkFormat(data);

			if(df == 0 || df == 4 || df == 5 || df == 16 || df == 20 || df == 21) {
				auto remainder = cyclicCheck(data, true);
				auto tail = data[data.size() - 1] + data[data.size() - 2]*(1 << 8) + data[data.size() - 3]*(1 << 16);

				std::vector<byte> res;
				res.push_back(((remainder ^ tail) >> 16) % 256);
				res.push_back(((remainder ^ tail) >> 8) % 256);
				res.push_back((remainder ^ tail) % 256);

				return res;
			} else if(df == 11 || df == 17 || df == 18)
				return std::optional<std::vector<byte>>(std::vector<byte>(data.begin() + 1, data.begin() + 4));


			return std::optional<std::vector<byte>>(std::nullopt);
		}

		static std::optional<std::pair<byte, byte>> identification(std::vector<byte> &data){
			auto tc = Decoder::typecode(data);

			return  (tc && *tc >= 1 && *tc <= 4) ?
				std::optional<std::pair<byte, byte>>(std::make_pair(data[4] >> 3, data[4] % (1 << 4))) : std::optional<std::pair<byte, byte>>(std::nullopt);
		}

		static std::optional<bool> parity(std::vector<byte> &data){
			auto df = Decoder::downlinkFormat(data);
			auto tc = Decoder::typecode(data);

			if(!(df == 17 && ((*tc >= 9 && *tc <= 18) || (*tc >= 20 && *tc <= 22))))
				return std::nullopt;

			return data[7] & (1 << 2);
		}

		static std::optional<std::string> callsign(std::vector<byte> &data){
			auto df = Decoder::downlinkFormat(data);
			auto tc = Decoder::typecode(data);

			if(!(df == 17 && *tc >= 1 && *tc <= 4))
				return std::nullopt;

			std::string res = "";

			for(int i = 40; i < 88; i += 6){
				byte c = 0;
				for(int j = 0; j < 6; j++)
					c += ((data[(i+j)/8] & (1 << (7 - ((i+j) % 8)))) > 0) * (1 << (5 - j));


				res.push_back(lookup[c]);
			}

			return res;
		}

		static std::optional<coordinates> position(std::vector<byte> first, std::vector<byte> second, unsigned long long timeF, unsigned long long timeS){
			if(!parity(first) || !parity(second) || (*parity(first) == *parity(second)))
				return std::nullopt;

			if(!*parity(first)){
				std::swap(first, second);
				std::swap(timeF, timeS);
			}

			long double even_lat = (((first[6] % 4) * (1 << 15)) + (first[7] * (1 << 7)) + (first[8] >> 1)) / (long double)(1L << 17);
			long double even_lon = (((first[8] % 2) * (1 << 16)) + (first[9] * (1 << 8)) + first[10]) / (long double)(1L << 17);

			long double odd_lat = (((second[6] % 4) * (1 << 15)) + (second[7] * (1 << 7)) + (second[8] >> 1)) / (long double)(1L << 17);
			long double odd_lon = (((second[8] % 2) * (1 << 16)) + (second[9] * (1 << 8)) + second[10]) / (long double)(1L << 17);

			long double j = std::floor(59 * even_lat - 60 * odd_lat + 0.5);

			even_lat = (360 / (long double)60) * (std::fmod(j, 60) + even_lat);
			odd_lat = (360 / (long double)59) * (std::fmod(j, 59) + odd_lat);

			if(even_lat >= 270)
				even_lat -= 360;
			if(odd_lat >= 270)
				odd_lat -= 360;

			if(NL(even_lat) != NL(odd_lat))
				return std::nullopt;

			long double lat = 0, lon = 0;

			if(timeF > timeS){
				lat = even_lat;
				auto nl = NL(lat);
				lon = (360.0L / std::max(nl, 1LL)) * (
					(std::fmod(std::floor((even_lon * (nl - 1)) - (odd_lon * nl) + .5), std::max(nl, 1LL))) + even_lon);
			} else {
				lat = odd_lat;
				auto nl = NL(lat);
				lon = (360.0L / std::max(nl - 1, 1LL)) * (
					(std::fmod(std::floor((even_lon * (nl - 1)) - (odd_lon * nl) + .5), std::max(nl - 1, 1LL))) + odd_lon);
			}

			return coordinates{ .lon = lon, .lat = lat };
		}

		static std::optional<distance> altitude(std::vector<byte> &data) {
			auto df = Decoder::downlinkFormat(data);
			auto tc = Decoder::typecode(data);

			if(!(df == 17 && ((*tc >= 9 && *tc <= 18) || (*tc >= 20 && *tc <= 22))))
				return std::nullopt;

			if(*tc >= 9 && *tc <= 18)
				return distance{
					.length = (((data[5] & 1) ? 25 : 100) * (((data[5] >> 1) * (1 << 4)) + (data[6] >> 4))) - 1000,
					.gnss = false
				};

			return distance{
				.length = (data[5] * (1 << 5)) + (data[6] >> 4),
				.gnss = false
			};
		}

		static std::optional<movement> displacement(std::vector<byte> &data) {
			auto df = Decoder::downlinkFormat(data);
			auto tc = Decoder::typecode(data);

			if(!(df == 17 && *tc == 19))
				return std::nullopt;

			int subtype = data[4] % 8;
			long double velocity = 0, angle = 0;
			source info;

			if(subtype == 1 || subtype == 2){
				info = inertial;
				int ewvelocity = (((data[5] % 8) >> 2) ? -1 : 1) * (subtype == 2 ? 4 : 1) * (( ((data[5] % 4) * (1 << 8)) + data[6] - 1));
				int nsvelocity = ((data[7] >> 7) ? -1 : 1) * (subtype == 2 ? 4 : 1) * ( ((data[7] % 128) * (1 << 3)) + (data[8] >> 5) - 1);

				velocity = std::sqrt((ewvelocity*ewvelocity) + (nsvelocity*nsvelocity));
				angle = std::fmod(((std::atan2(ewvelocity, nsvelocity) * 360.0L/(2*M_PIl)) + 360*10), 360);
			} else {
				info = (data[7] >> 7) ? airspeed_tas : airspeed_ias;
				angle = (( ((data[5] % 4) * (1 << 8)) + data[6])) * 360.0L/1024.0L;
				velocity = (subtype == 3 ? 1 : 4) * ( ((data[7] % 128) * (1 << 3)) + (data[8] >> 5) - 1);
			}

			return movement{
				.vertical_rate = ((((data[8] % 16) >> 3) ? -1 : 1) * (64 * ((((data[8] % 8) * (1 << 6)) + (data[9] >> 2)) - 1))),
				.diff = ((data[10] >> 7) ? 1 : -1) * (data[10] % 128),
				.velocity = velocity,
				.heading = angle,
				.info = info
			};
		}

		static std::optional<int> squawk(std::vector<byte> &data) {
			auto df = Decoder::downlinkFormat(data);

			if(df != 5)
				return std::nullopt;

			return ((((bool)(data[3] & 0b10000000) * 4) + ((bool)(data[2] & 0b00000010) * 2) + (bool)(data[2] & 0b00001000)) * 1000) +
						 ((((bool)(data[3] & 0b00000010) * 4) + ((bool)(data[3] & 0b00001000) * 2) + (bool)(data[3] & 0b00100000)) * 100) +
						 ((((bool)(data[2] & 0b00010000) * 4) + ((bool)(data[2] & 0b00000100) * 2) + (bool)(data[2] & 0b00000001)) * 10) +
						  (((bool)(data[3] & 0b00000001) * 4) + ((bool)(data[3] & 0b00000100) * 2) + (bool)(data[3] & 0b00010000));
		}
};
