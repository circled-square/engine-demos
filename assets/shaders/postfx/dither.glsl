#uniforms
uniform sampler2D u_texture_slot;
uniform sampler2D u_dither_texture;

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
uniform sampler2D u_dither_texture;


const int quant_levels = 16;

void main() {
    vec4 sample = texture(u_texture_slot, v_tex_coord);

    vec2 src_res = textureSize(u_texture_slot, 0);
    ivec2 src_pixel_space_pos = ivec2(src_res * v_tex_coord);
    ivec2 dither_texture_size = textureSize(u_dither_texture, 0);

    ivec2 dither_texture_texel_x = src_pixel_space_pos % dither_texture_size;
    ivec2 dither_texture_texel_y = ivec2(dither_texture_size.x-1 - dither_texture_texel_x.x, dither_texture_texel_x.y);
    ivec2 dither_texture_texel_z = ivec2(dither_texture_texel_x.x, dither_texture_size.y-1 - dither_texture_texel_x.y);

    vec3 dither_noise = vec3(
       texelFetch(u_dither_texture, dither_texture_texel_x, 0).x,
       texelFetch(u_dither_texture, dither_texture_texel_y, 0).x,
       texelFetch(u_dither_texture, dither_texture_texel_z, 0).x
    ) - 0.5;

    const int max_quant_level = quant_levels - 1;

    // nonlinearity in the distribution: <1 means the distribution of quantization values skews toward lower values, which means 
    // there are more values to choose from between darker colors than there would otherwise be; this is to avoid quantization 
    // having a disproportionate effect on dark colors before even being noticeable on lighter colors
    float nonlinearity = 0.5;

    vec3 dithered_sample = dither_noise / (float(max_quant_level) * 1.0) + pow(sample.xyz, vec3(nonlinearity));

    vec3 quantized_sample = pow(round(dithered_sample * float(max_quant_level)) / float(max_quant_level), vec3(1./nonlinearity));

    color = vec4(quantized_sample, 1.0);
}
