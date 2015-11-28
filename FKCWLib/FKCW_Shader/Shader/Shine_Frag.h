//*************************************************************************
//	��������:	2014-10-29
//	�ļ�����:	Shine_Frag.h
//  �� �� ��:   ������ FreeKnight	
//	��Ȩ����:	MIT
//	˵    ��:	
//*************************************************************************

#pragma once

//-------------------------------------------------------------------------
#include "cocos2d.h"
USING_NS_CC;
//-------------------------------------------------------------------------
#define kCCUniform_shineWidth		"CC_shineWidth"
#define kCCUniform_shineXY1			"CC_shineXY1"
#define kCCUniform_shineXY2			"CC_shineXY2"
#define kCCUniform_shineColor1		"CC_shineColor1"
#define kCCUniform_shineColor2		"CC_shineColor2"
#define kCCUniform_shineColor3		"CC_shineColor3"
#define kCCUniform_shinePositions	"CC_shinePositions"
#define kCCUniform_shineTime		"CC_shineTime"

static GLint sUniform_pos_CC_shineWidth = -1;
static GLint sUniform_pos_CC_shineTime = -1;
static GLint sUniform_pos_CC_shineXY1 = -1;
static GLint sUniform_pos_CC_shineXY2 = -1;
static GLint sUniform_pos_CC_shineColor1 = -1;
static GLint sUniform_pos_CC_shineColor2 = -1;
static GLint sUniform_pos_CC_shineColor3 = -1;
static GLint sUniform_pos_CC_shinePositions = -1;

const char* ShaderShineFrag = "\n\
								  #ifdef GL_ES\n\
								  precision lowp float;\n\
								  #endif\n\
								  \n\
								  varying vec4 v_fragmentColor;\n\
								  varying vec2 v_texCoord;\n\
								  varying vec4 v_position;\n\
								  uniform sampler2D CC_Texture0;\n\
								  uniform float CC_shineWidth;\n\
								  uniform float CC_shineTime;\n\
								  uniform vec2 CC_shineXY1;\n\
								  uniform vec2 CC_shineXY2;\n\
								  uniform vec4 CC_shineColor1;\n\
								  uniform vec4 CC_shineColor2;\n\
								  uniform vec4 CC_shineColor3;\n\
								  uniform vec3 CC_shinePositions;\n\
								  \n\
								  void main() {\n\
								  gl_FragColor = v_fragmentColor * texture2D(CC_Texture0, v_texCoord); \n\
								  \n\
								  // pattern width\n\
								  float patternWidth = abs(CC_shineXY2.x - CC_shineXY1.x);\n\
								  \n\
								  // pattern offset\n\
								  float patternOffsetX = CC_shineTime * (patternWidth + CC_shineWidth) - patternWidth;\n\
								  \n\
								  // get checking area\n\
								  float minX = min(CC_shineXY1.x, CC_shineXY2.x) + patternOffsetX - patternWidth;\n\
								  float minY = min(CC_shineXY1.y, CC_shineXY2.y);\n\
								  float maxX = max(CC_shineXY1.x, CC_shineXY2.x) + patternOffsetX + patternWidth;\n\
								  float maxY = max(CC_shineXY1.y, CC_shineXY2.y);\n\
								  \n\
								  // if pixel is in rect\n\
								  if(v_position.x >= minX && v_position.x <= maxX && v_position.y >= minY && v_position.y <= maxY) {\n\
								  // get gradient position\n\
								  vec2 v = CC_shineXY2 - CC_shineXY1;\n\
								  vec2 pv = v_position.xy - vec2(CC_shineXY1.x + patternOffsetX, CC_shineXY1.y);\n\
								  float vLen = length(v);\n\
								  float gradient = dot(v, pv) / vLen / vLen;\n\
								  \n\
								  // calculate color\n\
								  vec4 color;\n\
								  if(gradient >= CC_shinePositions.x && gradient <= CC_shinePositions.y) {\n\
								  float g = (gradient - CC_shinePositions.x) / (CC_shinePositions.y - CC_shinePositions.x);\n\
								  color = CC_shineColor1 * (1.0 - g) + CC_shineColor2 * g;\n\
								  gl_FragColor.xyz += gl_FragColor.a * color.a * color.xyz;\n\
								  } else if(gradient > CC_shinePositions.y && gradient <= CC_shinePositions.z) {\n\
								  float g = (gradient - CC_shinePositions.y) / (CC_shinePositions.z - CC_shinePositions.y);\n\
								  color = CC_shineColor2 * (1.0 - g) + CC_shineColor3 * g;\n\
								  gl_FragColor.xyz += gl_FragColor.a * color.a * color.xyz;\n\
								  }\n\
								  }\n\
								  }";