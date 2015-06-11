#ifndef PERLINNOISE_H
#define PERLINNOISE_H

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define B 0x100
#define BM 0xff

#define N 0x1000
#define NP 12   /* 2^N */
#define NM 0xfff

#define s_curve(t) ( t * t * (3. - 2. * t) )
#define lerp(t, a, b) ( a + t * (b - a) )

#define setup(i,b0,b1,r0,r1)\
	t = vec[i] + N;\
	b0 = ((int)t) & BM;\
	b1 = (b0+1) & BM;\
	r0 = t - (int)t;\
	r1 = r0 - 1.;

class PerlinNoise
{
public:
	PerlinNoise();
	~PerlinNoise();

	int start;
	int p[B + B + 2];

	float g3[B + B + 2][3];
	float g2[B + B + 2][2];
	float g1[B + B + 2];

	void init(void);

	double noise1(double arg);

	float noise2(float vec[2]);
	float noise3(float vec[3]);

	void normalize2(float v[2]);
	void normalize3(float v[3]);
};

#endif