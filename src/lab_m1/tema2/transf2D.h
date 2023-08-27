#pragma once

#include "utils/glm_utils.h"


namespace transf2D
{
    // Translate matrix
    inline glm::mat3 Translate(float translateX, float translateY)
    {
        return glm::transpose(
            glm::mat3(1, 0, translateX,
                        0, 1, translateY,
                        0, 0, 1));

    }

    // Scale matrix
    inline glm::mat3 Scale(float scaleX, float scaleY)
    {
        return glm::transpose(
            glm::mat3(scaleX, 0, 0,
                0, scaleY, 0,
                0, 0, 1));

    }

    // Rotate matrix
    inline glm::mat3 Rotate(float radians)
    {
        return glm::transpose(
            glm::mat3(cos(radians), (-1) * sin(radians), 0,
                sin(radians), cos(radians), 0,
                0, 0, 1));

    }


    Mesh* CreateRectangle(const std::string& name, glm::vec3 leftBottomCorner, float x_l, float y_l, glm::vec3 color,
        bool fill)
    {
        glm::vec3 corner = leftBottomCorner;

        std::vector<VertexFormat> vertices =
        {
            VertexFormat(corner, color),
            VertexFormat(corner + glm::vec3(x_l, 0, 0), color),
            VertexFormat(corner + glm::vec3(x_l, y_l, 0), color),
            VertexFormat(corner + glm::vec3(0, y_l, 0), color)
        };

        Mesh* square = new Mesh(name);
        std::vector<unsigned int> indices = { 0, 1, 2, 3 };

        if (!fill) {
            square->SetDrawMode(GL_LINE_LOOP);
        }
        else {
            indices.push_back(0);
            indices.push_back(2);
        }

        square->InitFromData(vertices, indices);
        return square;
    }
}   // namespace transform2D
