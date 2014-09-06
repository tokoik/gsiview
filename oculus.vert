#version 150 core
#extension GL_ARB_explicit_attrib_location : enable

// Oculus Rift のアスペクト比
uniform float ocuAspect;

// Oculus Rift のレンズの中心の画面の中心からのずれ
uniform float projectionCenterOffset;

// 頂点属性
layout (location = 0) in vec4 pv;

// ラスタライザに送る頂点属性
out vec2 t;

void main()
{
	t = vec2(pv.x * ocuAspect + projectionCenterOffset, pv.y);
	gl_Position = pv;
}
