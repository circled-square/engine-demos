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
#include <slogga/asserts.hpp>

struct collision_shape {
    std::vector<glm::vec3> verts;
    std::vector<glm::vec3> face_normals;
    std::vector<glm::vec3> edges;
};

class collision_result {
    glm::vec3 m_versor;
    float m_depth;
public:
    collision_result() = delete;
    collision_result(const collision_result&) = default;
    collision_result(glm::vec3 versor, float depth) : m_versor(versor), m_depth(depth) {}
    static collision_result null() { return collision_result(glm::vec3(0), std::numeric_limits<float>::quiet_NaN()); }

    glm::vec3 get_versor() const { return m_versor; }
    float get_depth() const { return m_depth; }

    bool is_shallow() const {
        EXPECTS(this->operator bool());
        return m_depth == 0.f;
    }

    glm::vec3 get_min_move_vector() {
        EXPECTS(this->operator bool());

        return -m_depth * m_versor;
    }

    operator bool() const { return !std::isnan(m_depth); }
};

collision_result check_collision(const collision_shape& a, glm::mat4 a_trans, const collision_shape& b, glm::mat4 b_trans);
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
