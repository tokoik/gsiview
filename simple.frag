#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

// テクスチャ
uniform sampler2D cmap;                             // 拡散反射色

// ラスタライザから受け取る頂点属性の補間値
in vec4 idiff;                                      // 拡散反射光強度
in vec4 ispec;                                      // 鏡面反射光強度
in vec2 t;                                          // テクスチャ座標

// フレームバッファに出力するデータ
layout (location = 0) out vec4 fc;                  // フラグメントの色

void main()
{
  fc = texture(cmap, t) * idiff + ispec;
}
