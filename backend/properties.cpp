struct coordinates {
	long double lon, lat;
};

struct distance {
	int length; bool gnss;
};

enum source {
	inertial,
	airspeed_tas,
	airspeed_ias
};

struct movement {
	bool gnss;
	int vertical_rate, diff;
	long double velocity, heading;
	source info;
};
