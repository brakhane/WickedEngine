#define OBJECTSHADER_LAYOUT_COMMON
#include "objectHF.hlsli"

float mandel2(float2 pos)
{
	pos = 4 * (pos - 0.5);

	const float2x2 c = {
		pos.x, -pos.y,
		pos.y,  pos.x
	};
#define MAX 100

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
	float4 color = m <= 0 ? float4(.6, 0, .4, 0.) : pos.yxxx * m;
	return color;
}

float4 col(float2 pos)
{
	const float2 res = float2(320, 240);

	float4 color = mandel(trunc(res*pos)/res);
	//color = mandel(pos);
	if ( res.x * abs(ddx_fine(pos.x)) < .166 && res.y * abs(ddy_fine(pos.x)) < .166)
	{
		const float xx = res.x * pos.x;
		const float yy = res.y * pos.y;

		/*
		if (frac(xx) < 0.30) color.yz = 0;
		else if (frac(xx) < 0.33) color = 0;
		else if (frac(xx) < 0.63) color.xz = 0;
		else if (frac(xx) < 0.66) color = 0;
		else if (frac(xx) < 0.96) color.xy = 0;
		else color = 0;
		*/
		if (frac(xx) < 0.33) color.yz = 0;
		else if (frac(xx) < 0.66) color.xz = 0;
		else color.xy = 0;

	}
	return color;
}


float4 main(PixelInput input) : SV_TARGET
{
	ShaderMaterial material = GetMaterial();

	float4 uvsets = input.GetUVSets();
	
	write_mipmap_feedback(push.materialIndex, ddx_coarse(uvsets), ddy_coarse(uvsets));

	float2 d = float2(ddx(uvsets.x), ddy(uvsets.y));

	float2 ul = uvsets.xy + 1*float2(-ddx(uvsets.x), -ddy(uvsets.y));
	float2 ur = uvsets.xy + 1*float2(+ddx(uvsets.x), -ddy(uvsets.y));
	float2 ll = uvsets.xy + 1*float2(-ddx(uvsets.x), +ddy(uvsets.y));
	float2 lr = uvsets.xy + 1*float2(+ddx(uvsets.x), +ddy(uvsets.y));

	float4 fincol = (col(ul) + col(ur) + col(ll) + col(lr)) / 4;
	fincol = col(uvsets);

	float4 cc = (pow(fincol, 1 / 2.2) * 5);
	return cc;
	//return ((frac(xx) < fac) && (frac(yy) < fac)) ? cc : cc * .0;
}
