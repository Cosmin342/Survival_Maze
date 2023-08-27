#version 330

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;


// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform float seconds;

uniform vec3 object_color;

// Output
// TODO(student): Output values to fragment shader
out vec3 color;

void main()
{
    color = object_color;

    //Se modifica v_position pentru animare utilizand deltatimeseconds (second in shader)
    vec3 position = v_position * seconds * 100;

    gl_Position = Projection * View * Model * vec4(position, 1.0);
}
