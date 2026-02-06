#uniforms
uniform float u_custom;
uniform sampler2D u_texture_slot;
uniform mat4 u_mvp;

#vertex
#version 440 core
layout(location = 0) in vec3 pos;
layout(location = 1) in vec2 tex_coord;
out vec2 v_tex_coord;

uniform mat4 u_mvp;

void main() {
    gl_Position = u_mvp * vec4(pos, 1);
    v_tex_coord = tex_coord;
}

#fragment
#version 330 core
in vec2 v_tex_coord;
out vec4 color;

uniform sampler2D u_texture_slot;
uniform float u_custom;

void main() {
        color = texture(u_texture_slot, v_tex_coord + vec2(u_custom, 0));
}
