#pragma once

#include "components/simple_scene.h"
#include "lab_camera.h"


namespace m1
{
    class Maze : public gfxc::SimpleScene
    {
    public:
        struct ViewportSpace
        {
            ViewportSpace() : x(0), y(0), width(1), height(1) {}
            ViewportSpace(int x, int y, int width, int height)
                : x(x), y(y), width(width), height(height) {}
            int x;
            int y;
            int width;
            int height;
        };

        struct LogicSpace
        {
            LogicSpace() : x(0), y(0), width(1), height(1) {}
            LogicSpace(float x, float y, float width, float height)
                : x(x), y(y), width(width), height(height) {}
            float x;
            float y;
            float width;
            float height;
        };
     public:
        Maze();
        ~Maze();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;
        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color);
        void generate_maze();
        glm::mat4 RotateOY(float radians);
        bool check_collision(glm::vec3 player_position);
        void check_collision_enem(glm::vec3 player_position);
        bool check_bul_collision_enem(glm::vec3 player_position);
        bool check_bul_collision_walls(glm::vec3 player_position);
        glm::mat3 VisualizationTransf2D(const LogicSpace& logicSpace, const ViewportSpace& viewSpace);
        void SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor, bool clear);
        void DrawHUD(glm::mat3 visMatrix);
        Mesh* CreateRectangle(const std::string& name, glm::vec3 leftBottomCorner, float x_l, float y_l, glm::vec3 color,
            bool fill);
        void RenderDeformMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color,
            float deltaTimeSeconds);

     protected:
        ViewportSpace viewSpace;
        LogicSpace logicSpace;
        implemented::Camera *camera, *camera_third;
        glm::mat4 projectionMatrix;
        glm::mat4 perspProj;
        glm::vec3 old_camera_position;
        std::vector<float> bullets_off_z;
        std::vector<glm::vec3> bullets;
        std::vector<float> rotation_angles;
        std::vector<std::vector<int>> maze;
        std::vector<std::pair<int, int>> free_slots;
        std::vector<std::pair<int, int>> enem_slots;
        std::vector<std::pair<int, int>> hitted_enem;
        std::vector<float> timer_enem;
        std::pair<int, int> coord_player;
        std::pair<int, int> exit_enemy;
        std::pair<int, int> exit_coord;
        bool renderCameraTarget;

        // TODO(student): If you need any other class variables, define them here.

        float rotation_angle, rotation_bullet;
        float off_z, off_x, life, scaleX, enemies_touched, time, scaleTime, factTime;
        int bullets_nr, maze_length;
        bool bullet_add, inc_dec;
    };
}   // namespace m1
