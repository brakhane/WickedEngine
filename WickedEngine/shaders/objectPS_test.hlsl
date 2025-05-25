#define OBJECTSHADER_LAYOUT_COMMON
#include "objectHF.hlsli"


float4 VGA(int i)
{
	float3 col;
	if (i == 0) col = float3(0, 0, 0);
	else if (i == 1) col = float3(0, 0, 42);
	else if (i == 2) col = float3(0, 42, 0);
	else if (i == 3) col = float3(0, 42, 42);
	else if (i == 4) col = float3(42, 0, 0);
	else if (i == 5) col = float3(42, 0, 42);
	else if (i == 6) col = float3(42, 21, 0);
	else if (i == 7) col = float3(42, 42, 42);
	else if (i == 8) col = float3(21, 21, 21);
	else if (i == 9) col = float3(21, 21, 63);
	else if (i == 10) col = float3(21, 63, 21);
	else if (i == 11) col = float3(21, 63, 63);
	else if (i == 12) col = float3(63, 21, 21);
	else if (i == 13) col = float3(63, 21, 63);
	else if (i == 14) col = float3(63, 63, 21);
	else if (i == 15) col = float3(63, 63, 63);
	else if (i == 16) col = float3(0, 0, 0);
	else if (i == 17) col = float3(5, 5, 5);
	else if (i == 18) col = float3(8, 8, 8);
	else if (i == 19) col = float3(11, 11, 11);
	else if (i == 20) col = float3(14, 14, 14);
	else if (i == 21) col = float3(17, 17, 17);
	else if (i == 22) col = float3(20, 20, 20);
	else if (i == 23) col = float3(24, 24, 24);
	else if (i == 24) col = float3(28, 28, 28);
	else if (i == 25) col = float3(32, 32, 32);
	else if (i == 26) col = float3(36, 36, 36);
	else if (i == 27) col = float3(40, 40, 40);
	else if (i == 28) col = float3(45, 45, 45);
	else if (i == 29) col = float3(50, 50, 50);
	else if (i == 30) col = float3(56, 56, 56);
	else if (i == 31) col = float3(63, 63, 63);
	else if (i == 32) col = float3(0, 0, 63);
	else if (i == 33) col = float3(16, 0, 63);
	else if (i == 34) col = float3(31, 0, 63);
	else if (i == 35) col = float3(47, 0, 63);
	else if (i == 36) col = float3(63, 0, 63);
	else if (i == 37) col = float3(63, 0, 47);
	else if (i == 38) col = float3(63, 0, 31);
	else if (i == 39) col = float3(63, 0, 16);
	else if (i == 40) col = float3(63, 0, 0);
	else if (i == 41) col = float3(63, 16, 0);
	else if (i == 42) col = float3(63, 31, 0);
	else if (i == 43) col = float3(63, 47, 0);
	else if (i == 44) col = float3(63, 63, 0);
	else if (i == 45) col = float3(47, 63, 0);
	else if (i == 46) col = float3(31, 63, 0);
	else if (i == 47) col = float3(16, 63, 0);
	else if (i == 48) col = float3(0, 63, 0);
	else if (i == 49) col = float3(0, 63, 16);
	else if (i == 50) col = float3(0, 63, 31);
	else if (i == 51) col = float3(0, 63, 47);
	else if (i == 52) col = float3(0, 63, 63);
	else if (i == 53) col = float3(0, 47, 63);
	else if (i == 54) col = float3(0, 31, 63);
	else if (i == 55) col = float3(0, 16, 63);
	else if (i == 56) col = float3(31, 31, 63);
	else if (i == 57) col = float3(39, 31, 63);
	else if (i == 58) col = float3(47, 31, 63);
	else if (i == 59) col = float3(55, 31, 63);
	else if (i == 60) col = float3(63, 31, 63);
	else if (i == 61) col = float3(63, 31, 55);
	else if (i == 62) col = float3(63, 31, 47);
	else if (i == 63) col = float3(63, 31, 39);
	else if (i == 64) col = float3(63, 31, 31);
	else if (i == 65) col = float3(63, 39, 31);
	else if (i == 66) col = float3(63, 47, 31);
	else if (i == 67) col = float3(63, 55, 31);
	else if (i == 68) col = float3(63, 63, 31);
	else if (i == 69) col = float3(55, 63, 31);
	else if (i == 70) col = float3(47, 63, 31);
	else if (i == 71) col = float3(39, 63, 31);
	else if (i == 72) col = float3(31, 63, 31);
	else if (i == 73) col = float3(31, 63, 39);
	else if (i == 74) col = float3(31, 63, 47);
	else if (i == 75) col = float3(31, 63, 55);
	else if (i == 76) col = float3(31, 63, 63);
	else if (i == 77) col = float3(31, 55, 63);
	else if (i == 78) col = float3(31, 47, 63);
	else if (i == 79) col = float3(31, 39, 63);
	else if (i == 80) col = float3(45, 45, 63);
	else if (i == 81) col = float3(49, 45, 63);
	else if (i == 82) col = float3(54, 45, 63);
	else if (i == 83) col = float3(58, 45, 63);
	else if (i == 84) col = float3(63, 45, 63);
	else if (i == 85) col = float3(63, 45, 58);
	else if (i == 86) col = float3(63, 45, 54);
	else if (i == 87) col = float3(63, 45, 49);
	else if (i == 88) col = float3(63, 45, 45);
	else if (i == 89) col = float3(63, 49, 45);
	else if (i == 90) col = float3(63, 54, 45);
	else if (i == 91) col = float3(63, 58, 45);
	else if (i == 92) col = float3(63, 63, 45);
	else if (i == 93) col = float3(58, 63, 45);
	else if (i == 94) col = float3(54, 63, 45);
	else if (i == 95) col = float3(49, 63, 45);
	else if (i == 96) col = float3(45, 63, 45);
	else if (i == 97) col = float3(45, 63, 49);
	else if (i == 98) col = float3(45, 63, 54);
	else if (i == 99) col = float3(45, 63, 58);
	else if (i == 100) col = float3(45, 63, 63);
	else if (i == 101) col = float3(45, 58, 63);
	else if (i == 102) col = float3(45, 54, 63);
	else if (i == 103) col = float3(45, 49, 63);
	else if (i == 104) col = float3(0, 0, 28);
	else if (i == 105) col = float3(7, 0, 28);
	else if (i == 106) col = float3(14, 0, 28);
	else if (i == 107) col = float3(21, 0, 28);
	else if (i == 108) col = float3(28, 0, 28);
	else if (i == 109) col = float3(28, 0, 21);
	else if (i == 110) col = float3(28, 0, 14);
	else if (i == 111) col = float3(28, 0, 7);
	else if (i == 112) col = float3(28, 0, 0);
	else if (i == 113) col = float3(28, 7, 0);
	else if (i == 114) col = float3(28, 14, 0);
	else if (i == 115) col = float3(28, 21, 0);
	else if (i == 116) col = float3(28, 28, 0);
	else if (i == 117) col = float3(21, 28, 0);
	else if (i == 118) col = float3(14, 28, 0);
	else if (i == 119) col = float3(7, 28, 0);
	else if (i == 120) col = float3(0, 28, 0);
	else if (i == 121) col = float3(0, 28, 7);
	else if (i == 122) col = float3(0, 28, 14);
	else if (i == 123) col = float3(0, 28, 21);
	else if (i == 124) col = float3(0, 28, 28);
	else if (i == 125) col = float3(0, 21, 28);
	else if (i == 126) col = float3(0, 14, 28);
	else if (i == 127) col = float3(0, 7, 28);
	else if (i == 128) col = float3(14, 14, 28);
	else if (i == 129) col = float3(17, 14, 28);
	else if (i == 130) col = float3(21, 14, 28);
	else if (i == 131) col = float3(24, 14, 28);
	else if (i == 132) col = float3(28, 14, 28);
	else if (i == 133) col = float3(28, 14, 24);
	else if (i == 134) col = float3(28, 14, 21);
	else if (i == 135) col = float3(28, 14, 17);
	else if (i == 136) col = float3(28, 14, 14);
	else if (i == 137) col = float3(28, 17, 14);
	else if (i == 138) col = float3(28, 21, 14);
	else if (i == 139) col = float3(28, 24, 14);
	else if (i == 140) col = float3(28, 28, 14);
	else if (i == 141) col = float3(24, 28, 14);
	else if (i == 142) col = float3(21, 28, 14);
	else if (i == 143) col = float3(17, 28, 14);
	else if (i == 144) col = float3(14, 28, 14);
	else if (i == 145) col = float3(14, 28, 17);
	else if (i == 146) col = float3(14, 28, 21);
	else if (i == 147) col = float3(14, 28, 24);
	else if (i == 148) col = float3(14, 28, 28);
	else if (i == 149) col = float3(14, 24, 28);
	else if (i == 150) col = float3(14, 21, 28);
	else if (i == 151) col = float3(14, 17, 28);
	else if (i == 152) col = float3(20, 20, 28);
	else if (i == 153) col = float3(22, 20, 28);
	else if (i == 154) col = float3(24, 20, 28);
	else if (i == 155) col = float3(26, 20, 28);
	else if (i == 156) col = float3(28, 20, 28);
	else if (i == 157) col = float3(28, 20, 26);
	else if (i == 158) col = float3(28, 20, 24);
	else if (i == 159) col = float3(28, 20, 22);
	else if (i == 160) col = float3(28, 20, 20);
	else if (i == 161) col = float3(28, 22, 20);
	else if (i == 162) col = float3(28, 24, 20);
	else if (i == 163) col = float3(28, 26, 20);
	else if (i == 164) col = float3(28, 28, 20);
	else if (i == 165) col = float3(26, 28, 20);
	else if (i == 166) col = float3(24, 28, 20);
	else if (i == 167) col = float3(22, 28, 20);
	else if (i == 168) col = float3(20, 28, 20);
	else if (i == 169) col = float3(20, 28, 22);
	else if (i == 170) col = float3(20, 28, 24);
	else if (i == 171) col = float3(20, 28, 26);
	else if (i == 172) col = float3(20, 28, 28);
	else if (i == 173) col = float3(20, 26, 28);
	else if (i == 174) col = float3(20, 24, 28);
	else if (i == 175) col = float3(20, 22, 28);
	else if (i == 176) col = float3(0, 0, 16);
	else if (i == 177) col = float3(4, 0, 16);
	else if (i == 178) col = float3(8, 0, 16);
	else if (i == 179) col = float3(12, 0, 16);
	else if (i == 180) col = float3(16, 0, 16);
	else if (i == 181) col = float3(16, 0, 12);
	else if (i == 182) col = float3(16, 0, 8);
	else if (i == 183) col = float3(16, 0, 4);
	else if (i == 184) col = float3(16, 0, 0);
	else if (i == 185) col = float3(16, 4, 0);
	else if (i == 186) col = float3(16, 8, 0);
	else if (i == 187) col = float3(16, 12, 0);
	else if (i == 188) col = float3(16, 16, 0);
	else if (i == 189) col = float3(12, 16, 0);
	else if (i == 190) col = float3(8, 16, 0);
	else if (i == 191) col = float3(4, 16, 0);
	else if (i == 192) col = float3(0, 16, 0);
	else if (i == 193) col = float3(0, 16, 4);
	else if (i == 194) col = float3(0, 16, 8);
	else if (i == 195) col = float3(0, 16, 12);
	else if (i == 196) col = float3(0, 16, 16);
	else if (i == 197) col = float3(0, 12, 16);
	else if (i == 198) col = float3(0, 8, 16);
	else if (i == 199) col = float3(0, 4, 16);
	else if (i == 200) col = float3(8, 8, 16);
	else if (i == 201) col = float3(10, 8, 16);
	else if (i == 202) col = float3(12, 8, 16);
	else if (i == 203) col = float3(14, 8, 16);
	else if (i == 204) col = float3(16, 8, 16);
	else if (i == 205) col = float3(16, 8, 14);
	else if (i == 206) col = float3(16, 8, 12);
	else if (i == 207) col = float3(16, 8, 10);
	else if (i == 208) col = float3(16, 8, 8);
	else if (i == 209) col = float3(16, 10, 8);
	else if (i == 210) col = float3(16, 12, 8);
	else if (i == 211) col = float3(16, 14, 8);
	else if (i == 212) col = float3(16, 16, 8);
	else if (i == 213) col = float3(14, 16, 8);
	else if (i == 214) col = float3(12, 16, 8);
	else if (i == 215) col = float3(10, 16, 8);
	else if (i == 216) col = float3(8, 16, 8);
	else if (i == 217) col = float3(8, 16, 10);
	else if (i == 218) col = float3(8, 16, 12);
	else if (i == 219) col = float3(8, 16, 14);
	else if (i == 220) col = float3(8, 16, 16);
	else if (i == 221) col = float3(8, 14, 16);
	else if (i == 222) col = float3(8, 12, 16);
	else if (i == 223) col = float3(8, 10, 16);
	else if (i == 224) col = float3(11, 11, 16);
	else if (i == 225) col = float3(12, 11, 16);
	else if (i == 226) col = float3(13, 11, 16);
	else if (i == 227) col = float3(15, 11, 16);
	else if (i == 228) col = float3(16, 11, 16);
	else if (i == 229) col = float3(16, 11, 15);
	else if (i == 230) col = float3(16, 11, 13);
	else if (i == 231) col = float3(16, 11, 12);
	else if (i == 232) col = float3(16, 11, 11);
	else if (i == 233) col = float3(16, 12, 11);
	else if (i == 234) col = float3(16, 13, 11);
	else if (i == 235) col = float3(16, 15, 11);
	else if (i == 236) col = float3(16, 16, 11);
	else if (i == 237) col = float3(15, 16, 11);
	else if (i == 238) col = float3(13, 16, 11);
	else if (i == 239) col = float3(12, 16, 11);
	else if (i == 240) col = float3(11, 16, 11);
	else if (i == 241) col = float3(11, 16, 12);
	else if (i == 242) col = float3(11, 16, 13);
	else if (i == 243) col = float3(11, 16, 15);
	else if (i == 244) col = float3(11, 16, 16);
	else if (i == 245) col = float3(11, 15, 16);
	else if (i == 246) col = float3(11, 13, 16);
	else if (i == 247) col = float3(11, 12, 16);
	else if (i == 248) col = float3(0, 0, 0);
	else if (i == 249) col = float3(0, 0, 0);
	else if (i == 250) col = float3(0, 0, 0);
	else if (i == 251) col = float3(0, 0, 0);
	else if (i == 252) col = float3(0, 0, 0);
	else if (i == 253) col = float3(0, 0, 0);
	else if (i == 254) col = float3(0, 0, 0);
	else if (i == 255) col = float3(0, 0, 0);

	return float4(col / 63, 1.);
}

#define MAX 16

float mandel2(float2 pos)
{
	pos = 4 * (pos - 0.5);

	const float2x2 c = {
		pos.x, -pos.y,
		pos.y,  pos.x
	};

	float2x2 z = 0.;
	for (int i = 0; i <= MAX; i++) {
		z = mul(z, z) + c;
		if (determinant(z) > 4.) return float(i) / float(MAX);
	}
	return -1.;
}



float4 mandel(float2 pos)
{
	float m = mandel2(pos);
	//float4 color = m <= 0 ? float4(.6, 0, .4, 0.) : pos.yxxx * m;
	float4 color = m <= 0 ? float4(0,0,0,0) : VGA(floor(m * MAX));
	return color;
}

float4 col(float2 pos)
{
	const float2 res = float2(1*640, 1*480);

	float4 color = mandel(trunc(res * pos) / res);
	//color = mandel(pos);
	return color;
}


float4 mask(float2 pos)
{
	const float dark = .5;
	pos.x += 3 * pos.y;
	float4 m = float4(dark, dark, dark, 1.);
	float x = frac(pos.x * 1.0 / 6.0);
	if (x < .333) m.r = 1.0;
	else if (x < .666) m.g = 1.0;
	else m.b = 1.0;
	return m;
}

float Scanline(float y, float fBlur)
{
	float fResult = sin(y * 2 * PI - PI/2) * 0.45 +0.55;
	return lerp(fResult, 1.0f, min(1., fBlur));
}

float GetScanline(float2 vUV)
{
	//vUV.y *= 0.25;
	vUV.y *= 480;
	float2 dx = ddx(vUV);
	float2 dy = ddy(vUV);
	float dV = length(float2(dx.y, dy.y));
	if (dV <= 0.0) return 1.0;
	return Scanline(vUV.y, dV * 1.3);
}


float4 main(PixelInput input) : SV_TARGET
{
	ShaderMaterial material = GetMaterial();

    float4 uvsets = input.GetUVSets();
	
	write_mipmap_feedback(push.materialIndex, ddx_coarse(uvsets), ddy_coarse(uvsets));

	float dx = abs(ddx(uvsets.x));
	float dy = abs(ddy(uvsets.y));

	float2 ul = uvsets.xy + 1 * float2(-dx, -dy);
	float2 ur = uvsets.xy + 1 * float2(+dx, -dy);
	float2 ll = uvsets.xy + 1 * float2(-dx, +dy);
	float2 lr = uvsets.xy + 1 * float2(+dx, +dy);

	float4 fincol = (col(ul) + col(ur) + col(ll) + col(lr)) / 4;
	
	//fincol += 3 * col(uvsets);
	//fincol /= 4;
	//fincol = col(uvsets);
	
	fincol *= mask(input.pos);
	fincol *= 1 * GetScanline(uvsets);

	float4 cc = fincol;//(pow(fincol, 1 / 2.2));
	return cc;
	//return ((frac(xx) < fac) && (frac(yy) < fac)) ? cc : cc * .0;
}
