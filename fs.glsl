/*
 * Copyright (C) 2013
 * Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
 * All rights reserved.
 */

#version 400

smooth in vec3 vcolor;
in float vtime;

uniform float unitime;

out vec4 fcolor;

void main(void)
{
    fcolor = vec4(vcolor.x*sin(3*unitime+2)+0.3, vcolor.y*cos(2*unitime)+0.3, vcolor.z*sin(4*unitime)+0.3, 1.0);
}
