#pragma once

#include "math_define.h"
#include "transform_util.h"
#include <glm/glm.hpp>

class BoundingBox
{
public:
    BoundingBox()
    {
        bb_min = glm::vec3(INF, INF, INF);
        bb_max = glm::vec3(NEG_INF, NEG_INF, NEG_INF);
    }

    explicit BoundingBox(const glm::vec3& p)
    {
        bb_min = p;
        bb_max = p;
    }

    BoundingBox(const glm::vec3& p1, const glm::vec3& p2)
    {
        glm::max(1, 5);
        bb_min = glm::vec3(glm::min(p1.x, p2.x), glm::min(p1.y, p2.y), glm::min(p1.z, p2.z));
        bb_max = glm::vec3(glm::max(p1.x, p2.x), glm::max(p1.y, p2.y), glm::max(p1.z, p2.z));
    }

    glm::vec3 get_center() const
    {
        return (bb_max + bb_min) * 0.5f;
    }

    glm::vec3 get_size() const
    {
        return (bb_max - bb_min);
    }

    float gt_surface_area() const
    {
        glm::vec3 size = get_size();
        return (size.x * size.y + size.x * size.z + size.y * size.z) * 2;
    }

    float get_volume() const
    {
        glm::vec3 size = get_size();
        return size.x * size.y * size.z;
    }

    float get_max_extent() const
    {
        glm::vec3 size = get_size();
        if (size.x > size.y && size.x > size.z)
        {
            return size.x;
        }
        else if (size.y > size.z)
        {
            return size.y;
        }
        else
        {
            return size.z;
        }
    }

    bool is_empty() const
    {
        bool ret = false;
        for (int i = 0; i < 3; ++i)
        {
            ret |= bb_min[i] >= bb_max[i];
        }
        return ret;
    }

    void get_corners(glm::vec3 corners[8]) const
    {
        corners[0] = { bb_min.x, bb_min.y, bb_min.z };
        corners[1] = { bb_max.x, bb_min.y, bb_min.z };
        corners[2] = { bb_min.x, bb_max.y, bb_min.z };
        corners[3] = { bb_max.x, bb_max.y, bb_min.z };
        corners[4] = { bb_min.x, bb_min.y, bb_max.z };
        corners[5] = { bb_max.x, bb_min.y, bb_max.z };
        corners[6] = { bb_min.x, bb_max.y, bb_max.z };
        corners[7] = { bb_max.x, bb_max.y, bb_max.z };
    }

    void merge(const BoundingBox& bbox)
    {
        bb_min.x = glm::min(bb_min.x, bbox.bb_min.x);
        bb_min.y = glm::min(bb_min.y, bbox.bb_min.y);
        bb_min.z = glm::min(bb_min.z, bbox.bb_min.z);
        bb_max.x = glm::max(bb_max.x, bbox.bb_max.x);
        bb_max.y = glm::max(bb_max.y, bbox.bb_max.y);
        bb_max.z = glm::max(bb_max.z, bbox.bb_max.z);
    }

    void merge(const glm::vec3& point)
    {
        bb_min.x = glm::min(bb_min.x, point.x);
        bb_min.y = glm::min(bb_min.y, point.y);
        bb_min.z = glm::min(bb_min.z, point.z);
        bb_max.x = glm::max(bb_max.x, point.x);
        bb_max.y = glm::max(bb_max.y, point.y);
        bb_max.z = glm::max(bb_max.z, point.z);
    }

    static BoundingBox merge(const BoundingBox& bbox1, const BoundingBox& bbox2)
    {
        BoundingBox ret;
        ret.bb_min.x = glm::min(bbox1.bb_min.x, bbox2.bb_min.x);
        ret.bb_min.y = glm::min(bbox1.bb_min.y, bbox2.bb_min.y);
        ret.bb_min.z = glm::min(bbox1.bb_min.z, bbox2.bb_min.z);
        ret.bb_max.x = glm::max(bbox1.bb_max.x, bbox2.bb_max.x);
        ret.bb_max.y = glm::max(bbox1.bb_max.y, bbox2.bb_max.y);
        ret.bb_max.z = glm::max(bbox1.bb_max.z, bbox2.bb_max.z);
        return ret;
    }

    static BoundingBox merge(const BoundingBox& bbox, const glm::vec3& point)
    {
        BoundingBox ret;
        ret.bb_min.x = glm::min(bbox.bb_min.x, point.x);
        ret.bb_min.y = glm::min(bbox.bb_min.y, point.y);
        ret.bb_min.z = glm::min(bbox.bb_min.z, point.z);
        ret.bb_max.x = glm::max(bbox.bb_max.x, point.x);
        ret.bb_max.y = glm::max(bbox.bb_max.y, point.y);
        ret.bb_max.z = glm::max(bbox.bb_max.z, point.z);
        return ret;
    }

    void grow(float amount)
    {
        bb_min.x -= amount;
        bb_min.y -= amount;
        bb_min.z -= amount;
        bb_max.x += amount;
        bb_max.y += amount;
        bb_max.z += amount;
    }

    BoundingBox transform(const glm::mat4 m) const
    {
        glm::vec3 new_bb_min = glm::vec3(INF, INF, INF);
        glm::vec3 new_bb_max = glm::vec3(NEG_INF, NEG_INF, NEG_INF);

        glm::vec3 corners[8];
        get_corners(corners);
        for (int i = 0; i < 8; i++)
        {
            glm::vec3 p = TransformUtil::transform_point(corners[i], m);
            new_bb_min = glm::min(new_bb_min, p);
            new_bb_max = glm::max(new_bb_max, p);
        }
        return BoundingBox(new_bb_min, new_bb_max);
    }

public:
    glm::vec3 bb_min{};
    glm::vec3 bb_max{};
};