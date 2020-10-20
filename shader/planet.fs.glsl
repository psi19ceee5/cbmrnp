#version 400

// get color from vertex shader
//in vec3 vcolor;
//texture coordinates from vertex shader
smooth in vec2 st;
in vec3 vcolor;

in vec3 norm;
in vec3 pos;

uniform mat4 projection_matrix;
uniform mat4 modelview_matrix;
uniform vec3 sunPosition;

uniform vec3 La;
uniform vec3 Ld;
uniform vec3 Ls;
uniform vec3 kd;
uniform vec3 ks;
uniform vec3 ka;
uniform float shininess;

uniform vec3 laserPosition;
uniform float laserAngle;
uniform vec3 laserDirection;


//
uniform sampler2D planetTex;

// send color to screen
layout(location = 0) out vec4 fcolor;

void main(void)
{

    //lighting

    //sun
    vec3 Mpos= vec3((modelview_matrix)*vec4(pos,1));

    vec3 normal = normalize(vec3(transpose(inverse(modelview_matrix))*vec4(norm,0)));
    vec3 sunLight= normalize(sunPosition-Mpos);

    vec3 view= normalize(-Mpos);
    vec3 r= (reflect(-view,normal));

    //ambient lighting
    vec3 ambient= ka*La;
    //diffuse lighting
    vec3 diffuse= kd*Ld* max(dot(sunLight,normal),0.0);

    //specular lighting

    vec3 spec= ks*Ls*pow(max(dot(r,sunLight),0.0),shininess);


    //laser
    vec3 laserColor;

    // calculate vector from object to laser position
    vec3 laserLight= normalize(laserPosition-Mpos);

    // calculate the angle between the inverted laser direction and light vector from object
    float lightAngle= acos(dot(laserLight,normalize(-laserDirection)));

    // if the angle is not higher than the laser cutoff angle, then set color to red
    if(lightAngle<=laserAngle)
    {
        laserColor=vec3(1.0f,0,0);
        diffuse+=laserColor*max(dot(normal,laserLight),0.0);
        spec+= laserColor*ks*pow(max(dot(r,laserLight),0.0),shininess);

    }

    vec4 texCol = texture2D(planetTex, st);
    vec3 color = (ambient+diffuse+spec);

    fcolor = (texCol*vec4(color,1));
}
