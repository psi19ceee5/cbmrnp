/*
 * Copyright (C) 2013
 * Computer Graphics Group, University of Siegen
 * Written by Martin Lambers <martin.lambers@uni-siegen.de>
 * All rights reserved.
 */

#version 400

in vec4 pos;
in vec3 color;

uniform float unitime;

smooth out vec3 vcolor;
out float vtime;

void main(void)
{
    vcolor = color;
    float omegaz = 0.3159278;
    float omegay = 0.18402592;
    float omegax = 0.05402454592;
    float phiz = omegaz*unitime;
    float phiy = omegay*unitime;
    float phix = omegax*unitime;
    float windowwidth = 1080;
    float windowheight = 720;
    float windowratio = windowwidth/windowheight;
    mat4 rotationz = mat4(vec4(cos(phiz),   -sin(phiz),0,0),
                          vec4(sin(phiz),    cos(phiz),0,0),
                          vec4(0,           0,        1,0),
                          vec4(0,           0,        0,1)
                         );
    mat4 rotationy = mat4(vec4(cos(phiy),0,-sin(phiy),0),
                          vec4(0,        1,0,         0),
                          vec4(sin(phiy),0,cos(phiy), 0),
                          vec4(0,        0,0,         1)
                         );
    mat4 rotationx = mat4(vec4(1,0,        0,           0),
                          vec4(0,cos(phix),-sin(phix),  0),
                          vec4(0,sin(phix),cos(phix),   0),
                          vec4(0,0,        0,           1)
                         );

    mat4 stretch = mat4(vec4(1/windowratio,0,0,0),
                        vec4(0,1,0,0),
                        vec4(0,0,1,0),
                        vec4(0,0,0,1)
                        );

    gl_Position = stretch*rotationz*rotationy*rotationx*pos;
}
