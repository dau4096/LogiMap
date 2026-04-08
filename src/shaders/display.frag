/* uv.frag */
#version 460 core


layout(binding=0) uniform usampler2D logiMap;

in vec2 fragUV;
out vec4 fragColour;

void main() {
	//Simply outputs the pixel's screen UV.
	vec4 gateData = texture(logiMap, fragUV);
	fragColour = vec4(gateData.xyz, 1.0f);
}