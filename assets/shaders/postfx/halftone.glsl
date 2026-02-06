#uniforms
uniform sampler2D u_texture_slot;
uniform float u_time;
uniform ivec2 u_output_resolution;

#vertex
#version 440 core

layout(location = 0) in vec2 pos;
out vec2 v_tex_coord;
void main() {
    gl_Position = vec4(pos, 0, 1);
    v_tex_coord = pos/2+.5;
}

#fragment
#version 440 core

precision lowp float;

in vec2 v_tex_coord;
out vec4 color;
uniform sampler2D u_texture_slot;
uniform float u_time;
uniform ivec2 u_output_resolution;

//just used as an array
const mat4x2 color_component_offsets = mat4x2(
    vec2(.1, .25), //c
    vec2(.25, .9), //y
    vec2(.75, .1), //m
    vec2(.9, .75)  //k
);

const float max_distance = 1.4;// radius in source pixel space for a dot of a cmyk component of value 1

const vec4 color_correction_cmyk = vec4(1.0, 1.2, 0.95, 1.0);

float norm_squared(vec2 v);
vec4 rgb_to_cmyk(vec3 rgb);
vec3 cmyk_to_rgb(vec4 cmyk);

const float upscaling_factor = 2.0; // the output/input resolution ratio; should not be manually calculated because it would yield imprecise results that cause artifacts. should be a uniform but isn't as of now

void main() {
    vec2 src_res = textureSize(u_texture_slot,0);
    vec2 src_pixel_space_pos = (u_output_resolution / upscaling_factor) * v_tex_coord;
    vec2 out_pos_in_src_pixel = fract(src_pixel_space_pos);

    vec4 src_rgba = texture(u_texture_slot, v_tex_coord);
    vec4 src_cmyk = rgb_to_cmyk(src_rgba.rgb);
    src_cmyk *= color_correction_cmyk;

    vec4 color_cmyk = vec4(0,0,0,0);

    for(int c = 0; c < 4; c++) {
        vec2 circle_center = color_component_offsets[c];

        float squared_distance = norm_squared(circle_center - out_pos_in_src_pixel);

        float max_squared_distance = src_cmyk[c] * max_distance * max_distance;
        if (squared_distance < max_squared_distance) {
            color_cmyk[c] = src_cmyk[c];
        }
    }

    color.rgb = cmyk_to_rgb(color_cmyk);
    color.a = 1.0;
}

vec4 rgb_to_cmyk(in vec3 rgb) {
    float k = 1 - max(max(rgb.r, rgb.g), rgb.b);
    vec3 cmy = (1 - rgb - k) / (1 - k);
    return vec4(cmy, k);
}

vec3 cmyk_to_rgb(in vec4 cmyk) {
    return (1-cmyk.xyz) * (1-cmyk.w);
}

float norm_squared(in vec2 v) {
    return dot(v, v);
}
