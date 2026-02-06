#uniforms
uniform sampler2D u_texture_slot;
uniform mat4 u_mvp;
uniform mat4 u_model;
uniform mat4 u_view;

#vertex
#version 440 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex_coord;
out vec2 v_tex_coord;
out vec4 v_pos;

uniform mat4 u_mvp;
uniform mat4 u_model;
uniform mat4 u_view;

void main() {
    gl_Position = u_mvp * vec4(pos, 1);
    v_pos = u_view * u_model * vec4(pos, 1);
    v_tex_coord = tex_coord;
}

#fragment
#version 330 core
in vec2 v_tex_coord;
in vec4 v_pos;
out vec4 color;

uniform sampler2D u_texture_slot;

void main() {
    float distance = length(v_pos);
    float distance_darkening = ((distance - 100.) / 300.);
    color = texture(u_texture_slot, v_tex_coord);
    color.xyz *= clamp(1. - distance_darkening, 0.2, 1.0);

    // color.xyz = vec3(v_pos.z/256.0);
}