#uniforms
uniform sampler2D u_texture_slot;

#vertex
#version 440 core

layout(location = 0) in vec2 pos;
out vec2 v_tex_coord;
void main() {
    gl_Position = vec4(pos, 0, 1);
    v_tex_coord = pos/2+.5;
}
#fragment
#version 330 core
in vec2 v_tex_coord;
out vec4 color;
uniform sampler2D u_texture_slot;
void main() {
    color = texture(u_texture_slot, v_tex_coord);
}
