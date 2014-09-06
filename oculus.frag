#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

// テクスチャ
uniform sampler2D ocuFboColor;

// レンズの歪みの補正係数
uniform vec4 lensDistortion;

// レンズの拡大縮の補正係数
uniform float lensScale;

// ラスタライザから受け取る頂点属性の補間値
in vec2 t;                                          // テクスチャ座標

// フレームバッファに出力するデータ
layout (location = 0) out vec4 fc;                  // フラグメントの色

void main()
{
  vec4 r;
  r.x = 1.0;
  r.y = dot(t, t);	// r^2
  r.z = r.y * r.y;	// r^4
  r.w = r.y * r.z;	// r^6
#if 1
  vec2 tc = t * dot(r, lensDistortion);
  fc = texture(ocuFboColor, tc * lensScale * 0.5 + 0.5);
#else
  vec2 tr = t * dot(r, lensDistortion);
  vec2 tg = t * dot(r, lensDistortion + vec4(0.0, 0.05, 0.04, 0.0));
  vec2 tb = t * dot(r, lensDistortion + vec4(0.0, 0.1, 0.08, 0.0));
  fc.r = texture(ocuFboColor, tr * lensScale * 0.5 + 0.5).r;
  fc.g = texture(ocuFboColor, tg * lensScale * 0.5 + 0.5).g;
  fc.b = texture(ocuFboColor, tb * lensScale * 0.5 + 0.5).b;
  fc.a = 1.0;
#endif
}
