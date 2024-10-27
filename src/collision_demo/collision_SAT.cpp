#include "collision_SAT.hpp"

#include <imgui.h>
#include <slogga/log.hpp>


using namespace glm;

static std::optional<float> proj_on_axis_and_find_collision(const vec3& axis, const std::vector<vec3>& a_verts, const std::vector<vec3>& b_verts, mat4 b_to_a_space_trans);


static bool project_and_update_min(const vec3& ax, const std::vector<vec3>& a_v, const std::vector<vec3> b_v, mat4 b_to_a, std::unordered_set<vec3>& dont_repeat, float& min_coll, vec3& min_coll_vec, mat4 trans_b4_saving) {
    if(dont_repeat.contains(ax))
        return true;
    dont_repeat.insert(ax);

    std::optional<float> coll = proj_on_axis_and_find_collision(ax, a_v, b_v, b_to_a);
    if(coll) {
        if(std::abs(*coll) < std::abs(min_coll)) {
            min_coll_vec = trans_b4_saving * vec4(ax, 0);
            min_coll = *coll;
        }
        return true;
    }
    return false;
}

collision_result check_collision(const collision_shape& a, mat4 a_trans, const collision_shape& b, mat4 b_trans) {
    std::unordered_set<vec3> dont_repeat;

    mat4 b_to_a_space_trans = inverse(a_trans) * b_trans;

    float min_col = std::numeric_limits<float>::max();
    vec3 min_col_dir;

    for(vec3 face_normal : a.face_normals) {
        if(!project_and_update_min(face_normal, a.verts, b.verts, b_to_a_space_trans, dont_repeat, min_col, min_col_dir, a_trans))
            return collision_result::null();

        if(min_col == 0.f)
            return collision_result(min_col_dir, min_col);
    }

    for(vec3 face_normal_b_space : b.face_normals) {
        vec3 face_normal = b_to_a_space_trans * vec4(face_normal_b_space, 0.0);

        if(!project_and_update_min(face_normal, a.verts, b.verts, b_to_a_space_trans, dont_repeat, min_col, min_col_dir, a_trans))
            return collision_result::null();

        if(min_col == 0.f)
            return collision_result(min_col_dir, min_col);
    }


    for(vec3 a_edge : a.edges) {
        for(vec3 b_edge_b_space : b.edges) {
            if(min_col == 0.f)
                return collision_result(min_col_dir, min_col);

            vec3 b_edge = b_to_a_space_trans * vec4(b_edge_b_space, 0.0);

            vec3 axis = normalize_without_verse(glm::cross(vec3(a_edge), vec3(b_edge)));
            if (std::isnan(axis.x)) { continue; }

            if(!project_and_update_min(axis, a.verts, b.verts, b_to_a_space_trans, dont_repeat, min_col, min_col_dir, a_trans)) {
                return collision_result::null();
            }
        }
    }

    return collision_result(min_col_dir, min_col);
}

static std::optional<float> proj_on_axis_and_find_collision(const vec3& axis, const std::vector<vec3>& a_verts, const std::vector<vec3>& b_verts, mat4 b_to_a_space_trans) {
    float min_a_proj = std::numeric_limits<float>::max();
    float max_a_proj = std::numeric_limits<float>::lowest();
    for(const vec3& v : a_verts) {
        float proj = glm::dot(axis, v);
        if(proj < min_a_proj) min_a_proj = proj;
        if(proj > max_a_proj) max_a_proj = proj;
    }

    float min_b_proj = std::numeric_limits<float>::max();
    float max_b_proj = std::numeric_limits<float>::lowest();
    for(const vec3& v_b_space : b_verts) {
        vec3 v = b_to_a_space_trans * vec4(v_b_space, 1);

        float proj = glm::dot(axis, v);
        if(proj < min_b_proj) min_b_proj = proj;
        if(proj > max_b_proj) max_b_proj = proj;
    }

    if (min_a_proj <= max_b_proj && min_b_proj <= max_a_proj) {
        //collision!
        float start_overlap = std::max(min_a_proj, min_b_proj);
        float end_overlap = std::min(max_a_proj, max_b_proj);
        float overlap = end_overlap - start_overlap;

        float double_mid_a = min_a_proj + max_a_proj;
        float double_mid_b = min_b_proj + max_b_proj;
        if(double_mid_b < double_mid_a)
            overlap = -overlap;

        return overlap;
    } else {
        return std::nullopt;
    }
}
