#ifndef COLLISION_SAT_HPP
#define COLLISION_SAT_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <sstream>
#include <array>
#include <optional>

struct collision_shape {
    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> face_normals;
    std::vector<glm::vec3> edges;
};

std::optional<glm::vec3> check_collision(const collision_shape& a, glm::mat4 a_trans, const collision_shape& b, glm::mat4 b_trans);
collision_shape make_collision_shape_from_mesh(const auto& mesh_verts, const auto& mesh_indices);




template<>
struct std::hash<glm::vec3> {
    std::size_t operator()(const glm::vec3& v) const noexcept {
        std::size_t h1 = std::hash<float>{}(v.x);
        std::size_t h2 = std::hash<float>{}(v.y);
        std::size_t h3 = std::hash<float>{}(v.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2); // or use boost::hash_combine
    }
};
/*
template<>
struct std::hash<i8vec3> {
    std::size_t operator()(const i8vec3& v) const noexcept {
        std::size_t h1 = std::hash<int8_t>{}(v.x);
        std::size_t h2 = std::hash<int8_t>{}(v.y);
        std::size_t h3 = std::hash<int8_t>{}(v.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2); // or use boost::hash_combine
    }
};


//we care not for the orientation of the normal/edge vectors, so we reverse all with y<0 (some with y=0) so vecs inverse to each other are not counted (since everything goes through a unordered_set)
inline i8vec3 normalize_without_verse_i8(vec3 v) {
    v = glm::normalize(v);
    i8vec3 ret = glm::round(v * 127.f);
    if(ret.y < 0 || (ret.y == 0 && ret.x < 0) || (ret.x == 0 && ret.y == 0 && ret.z < 0))
        ret = -ret;
    return ret;
}
inline i8vec3 apply_trans_i8(mat4 m, i8vec3 v) { return normalize_without_verse_i8(m * vec4(v, 0)); }
inline vec3 conv_i8_to_f_normalized(i8vec3 v) { return vec3(v) / 127.f; }
inline i8vec3 cross_i8(i8vec3 v, i8vec3 u) {
    i16vec3 a=v, b=u;
    return i8vec3(
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    );
}
*/

//we care not for the orientation of the normal/edge vectors, so we reverse all with y<0 (some with y=0) so vecs inverse to each other are not counted (since everything goes through a unordered_set)
inline glm::vec3 normalize_without_verse(glm::vec3 v) {
    v = glm::normalize(v);
    if(v.y < 0 || (v.y == 0 && v.x < 0) || (v.x == 0 && v.y == 0 && v.z < 0))
        v = -v;
    return v;
}

collision_shape make_collision_shape_from_mesh(const auto& mesh_verts, const auto& mesh_indices){
    using namespace glm;

    //TODO: make these into unordered_set<vec3> instead
    std::unordered_set<vec3> verts;
    std::unordered_set<vec3> normals;
    std::unordered_set<vec3> edges;

    for(uvec3 indices : mesh_indices) {
        for(int i = 0; i < 3; i++) {
            verts.insert(mesh_verts[indices[i]].pos);
        }

        vec3 edge_1 = mesh_verts[indices.y].pos - mesh_verts[indices.x].pos;
        vec3 edge_2 = mesh_verts[indices.z].pos - mesh_verts[indices.x].pos;
        vec3 edge_3 = mesh_verts[indices.z].pos - mesh_verts[indices.y].pos;

        vec3 normal = glm::cross(edge_1, edge_2);

        normals.insert(normalize_without_verse(normal));


        //add edges
        edges.insert(normalize_without_verse(edge_1));
        edges.insert(normalize_without_verse(edge_2));
        edges.insert(normalize_without_verse(edge_3));
    }
    // TODO: possible optimization: precalculate min&max for each axis of the shape itself so we don't need to do it every time (but only need it for the other shape's axes
    // TODO: add support for AABB optimization, possibly skipping edge-edge collisions

    collision_shape ret;
    for(vec3 vert : verts)      ret.verts.push_back(vert);
    for(vec3 normal : normals)  ret.face_normals.push_back(normal);
    for(vec3 edge : edges)      ret.edges.push_back(edge);

    return ret;
}

#endif // COLLISION_SAT_HPP
