#ifndef MATHUTIL_H
#define MATHUTIL_H

#define FLOAT_TO_FIXED(f,b) ((int)((f) * (1 << b)))
#define INT_TO_FIXED(i,b) ((i) << b)
#define FIXED_TO_INT(x,b) ((x) >> b)
#define FIXED_TO_FLOAT(x,b) ((float)(x) / (1 << b))
#define FIXED_MUL(x,y,b) (((x) * (y)) >> b)
#define FIXED_DIV(x,y,b) (((x) << b) / (y))
#define FIXED_SQRT(x,b) (sqrt((x) << b))

#define PI 3.14159265359f

#define CLAMP_LEFT(value, min) { if (value < min) value = min; }
#define CLAMP_RIGHT(value, max) { if (value > max) value = max; }
#define CLAMP(value, min, max) { CLAMP_LEFT(value,min) CLAMP_RIGHT(value, max) }
#define CLAMP_POSITIVE(value) CLAMP_LEFT(value, 0)
#define CLAMP_POSITIVE_RIGHT(value, max) CLAMP(value, 0, max)
#define CLAMP01(value) CLAMP_POSITIVE_RIGHT(value, 1)

#define SINE_PING_PONG(t) (sin(range * PI))


struct vec2
{
	// I will use a union in a stupid way to debug something temporarilly
	// I use vec2 X and Y for near and far variables (scale and pixel height in the rotozoomfloor)
	// Easier to see what I mean by X and Y at those cases, I know it could possibly be written different but oh well..

	/*union {
		float x;
		float far;
	};

	union {
		float y;
		float near;
	};*/

	// Why the fuck Union suddenly doesn't work? Strange message from compiler. Previously it worked but debugger complained something about header file not being the same but still ran. What is that?

	float x, y;

	vec2(float _x, float _y) : x(_x), y(_y) {}
	vec2() : vec2(0, 0) {}

	inline vec2 operator+(const vec2 &v) {
		return vec2(x + v.x, y + v.y);
	}

	inline vec2 operator-(const vec2 &v) {
		return vec2(x - v.x, y - v.y);
	}

	inline vec2& operator+=(vec2& v) {
		x += v.x;
		y += v.y;
		return *this;
	}

	inline vec2& operator-=(vec2& v) {
		x -= v.x;
		y -= v.y;
		return *this;
	}

	inline vec2 operator*(float s) {
		return vec2(x*s, y*s);
	}

	inline vec2& operator*=(float s) {
		x *= s;
		y *= s;
		return *this;
	}

	vec2& rotate(float angle)
	{
		vec2 v = *this;
		x = (float)(v.x * cos(angle) - v.y * sin(angle));
		y = (float)(v.x * sin(angle) + v.y * cos(angle));
		return (*this);
	}
};

int getRand(int from, int to);
float getPingClamp01(float value, float elevate = 1.0f);
float getPingPongClamp01(float value, float elevate = 1.0f);
float getDelayPingClamp01(float value, float start = 0.25f, float end = 0.75f);
float ASDR(float value, float attack, float sustain, float decay, float release);

#endif
