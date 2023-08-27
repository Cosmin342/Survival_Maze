#include "tema2.h"
#include "transf2D.h"

#include <vector>
#include <string>
#include <iostream>

using namespace std;
using namespace m1;

#define MAZE_SIZE           15
#define HEALTH_BAR_Y        8
#define TIME_BAR_Y          7.2f
#define SPEED_PLAYER        4.0f
#define BAR_DIM_X           2.5f
#define BAR_DIM_Y           0.5f
#define BULLET_RADIUS       0.4f
#define ENEMY_MOVE          0.05f
#define MAZE_START_X        -16.5f
#define MAZE_START_Z        -19.5f

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Maze::Maze()
{
}


Maze::~Maze()
{
}


void Maze::Init()
{
    renderCameraTarget = true;
    inc_dec = false;

    rotation_angle = 0;
    rotation_bullet = 0;
    bullets_nr = 0;
    maze_length = MAZE_SIZE;

    off_z = 0;
    off_x = 0;
    life = 1;
    scaleX = BAR_DIM_X;
    enemies_touched = 0;

    factTime = 1;
    scaleTime = BAR_DIM_X;
    time = 0;

    //Se initializeaza labirintul cu ziduri peste tot
    for (int i = 0; i < MAZE_SIZE; i++) {
        vector<int> line;
        for (int j = 0; j < MAZE_SIZE; j++) {
            line.push_back(1);
        }
        maze.push_back(line);
    }

    //Se genereaza drumurile in labirint
    Maze::generate_maze();

    //Se alege random o celula libera unde va fi pozitionat player-ul
    int random_position = rand() % free_slots.size();
    coord_player = free_slots[random_position];
    maze[coord_player.first][coord_player.second] = 3;
    free_slots.erase(free_slots.begin() + random_position);

    //Se vor genera inamici intr-un sfert din celulele libere, si acestea fiind alese random
    int enem_nr = (int) free_slots.size() / 4;
    for (int i = 0; i < enem_nr; i++) {
        random_position = rand() % free_slots.size();
        maze[free_slots[random_position].first][free_slots[random_position].second] = 2;
        enem_slots.push_back(make_pair(free_slots[random_position].first, free_slots[random_position].second));
        free_slots.erase(free_slots.begin() + random_position);
    }

    perspProj = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

    /*
    Se pozitioneaza camera in pozitia player - ului din labirint (labirintul pleaca de la (-16.5, 2, -19.5),
    pe z se merge in sens negativ, pe x in sens pozitiv, iar centrele cuburilor sunt la distanta 3 unul fata
    de altul)
    */
    camera = new implemented::Camera();
    camera->Set(glm::vec3(MAZE_START_X + coord_player.second * 3, 2, MAZE_START_Z + coord_player.first * 3 + 2),
        glm::vec3(MAZE_START_X + coord_player.second * 3, 1, MAZE_START_Z + coord_player.first * 3), glm::vec3(0, 1, 0));

    glm::ivec2 resolution = window->GetResolution();

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Shader* shader = new Shader("LabShader");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "VertexShader.glsl"),
            GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "FragmentShader.glsl"),
            GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        Shader* deform_shader = new Shader("DeformShader");
        deform_shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "VertexDeformShader.glsl"),
            GL_VERTEX_SHADER);
        deform_shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M1, "tema2", "shaders", "FragDeformShader.glsl"),
            GL_FRAGMENT_SHADER);
        deform_shader->CreateAndLink();
        shaders[deform_shader->GetName()] = deform_shader;
    }

    projectionMatrix = glm::perspective(RADIANS(60), window->props.aspectRatio, 0.01f, 200.0f);

    //Se seteaza a doua camera pentru HUD
    resolution = window->GetResolution();
    auto camera2 = GetSceneCamera();
    camera2->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera2->SetPosition(glm::vec3(0, 0, 50));
    camera2->SetRotation(glm::vec3(0, 0, 0));
    camera2->Update();
    GetCameraInput()->SetActive(false);

    logicSpace.x = 0;       // logic x
    logicSpace.y = 0;       // logic y
    logicSpace.width = 16;   // logic width
    logicSpace.height = 9;  // logic height

    //Se adauga mesh-uri pentru dreptunghiurile de la HUD
    Mesh* health = transf2D::CreateRectangle("health", glm::vec3(0, 0, 0), BAR_DIM_X, BAR_DIM_Y,
        glm::vec3(0, 0.5f, 1), false);
    AddMeshToList(health);

    Mesh* healthb = transf2D::CreateRectangle("healthb", glm::vec3(0, 0, 0), BAR_DIM_X, BAR_DIM_Y,
        glm::vec3(0, 0.5f, 1), true);
    AddMeshToList(healthb);

    Mesh* timeRectangle = transf2D::CreateRectangle("time", glm::vec3(0, 0, 0), BAR_DIM_X, BAR_DIM_Y,
        glm::vec3(0.8f, 0, 0), false);
    AddMeshToList(timeRectangle);

    Mesh* timeb = transf2D::CreateRectangle("timeb", glm::vec3(0, 0, 0), BAR_DIM_X, BAR_DIM_Y,
        glm::vec3(0.8f, 0, 0), true);
    AddMeshToList(timeb);
}

glm::mat3 Maze::VisualizationTransf2D(const LogicSpace& logicSpace, const ViewportSpace& viewSpace)
{
    float sx, sy, tx, ty;
    sx = viewSpace.width / logicSpace.width;
    sy = viewSpace.height / logicSpace.height;
    tx = viewSpace.x - sx * logicSpace.x;
    ty = viewSpace.y - sy * logicSpace.y;

    return glm::transpose(glm::mat3(
        sx, 0.0f, tx,
        0.0f, sy, ty,
        0.0f, 0.0f, 1.0f));
}

void Maze::SetViewportArea(const ViewportSpace& viewSpace, glm::vec3 colorColor, bool clear)
{
    glViewport(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    glEnable(GL_SCISSOR_TEST);
    glScissor(viewSpace.x, viewSpace.y, viewSpace.width, viewSpace.height);

    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(colorColor.r, colorColor.g, colorColor.b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    GetSceneCamera()->SetOrthographic((float)viewSpace.x, (float)(viewSpace.x + viewSpace.width), (float)viewSpace.y, (float)(viewSpace.y + viewSpace.height), 0.1f, 400);
    GetSceneCamera()->Update();
}

void Maze::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0.5f, 1);
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

//Metoda pentru generarea labirintului
void Maze::generate_maze() {
    vector<pair<pair<int, int>, pair<int, int>>> cells;
    //Se va alege o celula random de la (1, 1) la (13, 13) (labirintul va avea zid pe margine)
    int x = (rand() % 12) + 1;
    int y = (rand() % 12) + 1;

    /*
    x si y vor fi impare pentru ca vecinii la o distanta de o celula sa nu fie la
    marginea hartii
    */
    if (x % 2 == 0) {
        x--;
    }
    if (y % 2 == 0) {
        y--;
    }
    pair<int, int> coord = make_pair(x, y);
    //Celula aleasa va fi libera si se va adauga in vectorul ce retine celulele libere
    maze[x][y] = 0;

    free_slots.push_back(coord);
    /*
    Se adauga vecinii care sunt la o distanta de o celula si care nu sunt in afara
    distantei permise in vectorul cells
    */
    if (x - 2 > 0) {
        cells.push_back(make_pair(make_pair(x - 2, y), coord));
    }
    if (y - 2 > 0) {
        cells.push_back(make_pair(make_pair(x, y - 2), coord));
    }
    if (x + 2 < maze_length) {
        cells.push_back(make_pair(make_pair(x + 2, y), coord));
    }
    if (y + 2 < maze_length) {
        cells.push_back(make_pair(make_pair(x, y + 2), coord));
    }

    /*
    Cat timp vectorulul nu este gol, se va selecta o celula random si se va incerca
    adaugarea ei intre cele libere, cat si a celei dintre ea si cea care se invecineaza
    cu aceasta
    */
    while (!cells.empty()) {
        int selection = rand() % cells.size();

        //Daca a fost eliberata celula anterior, se va selecta alta
        if (maze[cells[selection].first.first][cells[selection].first.second] == 0) {
            cells.erase(cells.begin() + selection);
            continue;
        }
        maze[cells[selection].first.first][cells[selection].first.second] = 0;

        //Se adauga celula eliberata
        free_slots.push_back(cells[selection].first);
        pair<int, int> neighbour = cells[selection].second;
        
        /*
        Se elibereaza si celula dintre vecin si cea actuala, verificandu-se intai
        in ce directie se regaseste
        */
        if (cells[selection].first.first - neighbour.first == 0) {
            if (cells[selection].first.second - neighbour.second > 0) {
                maze[neighbour.first][cells[selection].first.second - 1] = 0;
                free_slots.push_back(make_pair(neighbour.first, cells[selection].first.second - 1));
            }
            else {
                maze[neighbour.first][neighbour.second - 1] = 0;
                free_slots.push_back(make_pair(neighbour.first, neighbour.second - 1));
            }
        }
        else {
            if (cells[selection].first.first - neighbour.first > 0) {
                maze[cells[selection].first.first - 1][neighbour.second] = 0;
                free_slots.push_back(make_pair(cells[selection].first.first - 1, neighbour.second));
            }
            else {
                maze[neighbour.first - 1][neighbour.second] = 0;
                free_slots.push_back(make_pair(neighbour.first - 1, neighbour.second));
            }
        }
        pair<int, int> point = cells[selection].first;
        cells.erase(cells.begin() + selection);

        //Se adauga in vector si vecinii celulei actuale
        if (point.first - 2 > 0) {
            if (maze[point.first - 2][point.second] == 1) {
                cells.push_back(make_pair(make_pair(point.first - 2, point.second), point));
            }
        }
        if (point.second - 2 > 0) {
            if (maze[point.first][point.second - 2] == 1) {
                cells.push_back(make_pair(make_pair(point.first, point.second - 2), point));
            }
        }
        if (point.first + 2 < maze_length) {
            if (maze[point.first + 2][point.second] == 1) {
                cells.push_back(make_pair(make_pair(point.first + 2, point.second), point));
            }
        }
        if (point.second + 2 < maze_length) {
            if (maze[point.first][point.second + 2] == 1) {
                cells.push_back(make_pair(make_pair(point.first, point.second + 2), point));
            }
        }
    }

    /*
    Se va adauga si o iesire pe prima linie, in functie de i - ul la care se gaseste prima
    celula libera de pe linia urmatoare
    */
    for (int i = 0; i < maze_length; i++) {
        if (maze[1][i] == 0) {
            //Pe iesire se pune 2 pentru a putea fi pazita de un inamic
            maze[0][i] = 2;
            exit_coord.first = 0;
            exit_coord.second = i;
            enem_slots.push_back(make_pair(0, i));
            break;
        }
    }
}

//Metoda pentru desenarea HUD-ului in functie de logicSpace
void Maze::DrawHUD(glm::mat3 visMatrix) {
    glm::mat3 modelMatrix;

    modelMatrix = visMatrix;
    modelMatrix *= transf2D::Translate(logicSpace.x + 1, logicSpace.y + HEALTH_BAR_Y);
    RenderMesh2D(meshes["health"], shaders["VertexColor"], modelMatrix);

    modelMatrix = visMatrix;
    modelMatrix *= transf2D::Translate(logicSpace.x + 1, logicSpace.y + HEALTH_BAR_Y) *
        transf2D::Scale(life, 1);
    RenderMesh2D(meshes["healthb"], shaders["VertexColor"], modelMatrix);

    modelMatrix = visMatrix;
    modelMatrix *= transf2D::Translate(logicSpace.x + 1, logicSpace.y + TIME_BAR_Y);
    RenderMesh2D(meshes["time"], shaders["VertexColor"], modelMatrix);

    modelMatrix = visMatrix;
    modelMatrix *= transf2D::Translate(logicSpace.x + 1, logicSpace.y + TIME_BAR_Y) *
        transf2D::Scale(factTime, 1);
    RenderMesh2D(meshes["timeb"], shaders["VertexColor"], modelMatrix);
}

void Maze::Update(float deltaTimeSeconds)
{
    glm::ivec2 resolution = window->GetResolution();
    glm::mat3 visMatrix;

    viewSpace = ViewportSpace(0, 0, resolution.x, resolution.y);
    SetViewportArea(viewSpace, glm::vec3(0, 0, 0.5f), true);

    visMatrix = glm::mat3(1);
    visMatrix *= VisualizationTransf2D(logicSpace, viewSpace);

    DrawHUD(visMatrix);

    glm::mat4 modelMatrix;

    time += (deltaTimeSeconds / 60);
    factTime = (scaleTime - time) / scaleTime;
    //Daca viata player-ului ajunge la 0, jocul se incheie
    if (fabs(factTime - 0.00f) < 0.01f) {
        cout << "Ai pierdut!\n";
        exit(0);
    }

    //Se deseneaza podeaua
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.1f, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(70, 0.2f, 70));
    RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0, 0.3f, 0));

    //offset_z si offset_x sunt folosite pentru determinarea coordonatelor unei celule
    float offset_z = -MAZE_START_Z - 1.5f;
    float offset_x = 0;
    //color_change este folosita pentru schimbarea tipului de gri utilizat la ziduri
    bool color_change = true;
    glm::vec3 color = glm::vec3(0.45f, 0.45f, 0.45f);
    for (int i = 0; i < MAZE_SIZE; i++) {
        for (int j = 0; j < MAZE_SIZE; j++) {
            if (color_change) {
                color_change = false;
                color = glm::vec3(0.45f, 0.45f, 0.45f);
            }
            else {
                color_change = true;
                color = glm::vec3(0.65f, 0.65f, 0.65f);
            }
            //Daca o celula este marcata cu 1, se va desena un zid
            if (maze[i][j] == 1) {
                modelMatrix = glm::mat4(1);
                modelMatrix = glm::translate(modelMatrix, glm::vec3(MAZE_START_X + offset_x, 2, -1.5f - offset_z));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(3, 4, 3));
                RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, color);
            }
            //Daca o celula este marcata cu 2, se va desena un inamic
            if (maze[i][j] == 2) {
                modelMatrix = glm::mat4(1);
                modelMatrix = glm::translate(modelMatrix, glm::vec3(MAZE_START_X + offset_x - off_x, 1.5f,
                    -1.5f - offset_z - off_z));
                modelMatrix = glm::scale(modelMatrix, glm::vec3(1, 1, 1));
                vector<pair<int, int>>::iterator index = find(hitted_enem.begin(), hitted_enem.end(), make_pair(i, j));
                //Daca inamicul a fost lovit anterior, va fi deformat
                if (index != hitted_enem.end()) {
                    int ind = (int) (index - hitted_enem.begin());
                    //Se va verifica insa daca au trecut 3 secunde de cand inamicul a fost lovit
                    if (timer_enem[ind] <= 0) {
                        maze[i][j] = 0;
                        hitted_enem.erase(hitted_enem.begin() + ind);
                        timer_enem.erase(timer_enem.begin() + ind);
                        offset_x += 3;
                        index = find(enem_slots.begin(), enem_slots.end(), make_pair(i, j));
                        ind = (int) (index - enem_slots.begin());
                        enem_slots.erase(enem_slots.begin() + ind);
                        continue;
                    }
                    RenderDeformMesh(meshes["box"], shaders["DeformShader"], modelMatrix,
                        glm::vec3(0.4f, 0, 0), deltaTimeSeconds);
                    timer_enem[ind] -= deltaTimeSeconds;
                }
                //Altfel, se deseneaza normal
                else {
                    RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.4f, 0, 0));
                }
            }
            offset_x += 3;
        }
        offset_x = 0;
        offset_z -= 3;
    }

    /*
    In functie de inc_dec, inamicii se vor incrementa / decrementa off_x si off_z
    utilizate la miscarea inamicilor
    */
    if (!inc_dec) {
        off_x += ENEMY_MOVE;
        off_z += ENEMY_MOVE;
    }
    else {
        off_x -= ENEMY_MOVE;
        off_z -= ENEMY_MOVE;
    }

    //Daca off_x ajunge la -1, se trece la incrementare
    if (fabs(off_x - 1.00f) < 0.01f) {
        inc_dec = true;
    }
    //Daca ajunge la 1, se trece la decrementare
    if (fabs(off_x + 1.00f) < 0.01f) {
        inc_dec = false;
    }

    //Daca renderCameraTarget este true, camera este de tip third person si player-ul va fi desenat 
    if (renderCameraTarget)
    {
        glm::vec3 camera_position = camera->GetTargetPosition();
        
        //Se deseneaza intai picioarele
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(camera_position.x, 2, camera_position.z));
        modelMatrix *= RotateOY(rotation_angle);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.2f, -1.65f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.7f, 0.2f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.2f, 0.2f, 1));

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(camera_position.x, 2, camera_position.z));
        modelMatrix *= RotateOY(rotation_angle);
        modelMatrix = glm::translate(modelMatrix, glm::vec3( 0.2f, -1.65f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.7f, 0.2f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0.2f, 0.2f, 1));

        //Apoi corpul si mainile
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(camera_position.x, 2, camera_position.z));
        modelMatrix *= RotateOY(rotation_angle);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.95f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.6f, 0.7f, 0.2f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0, 0.8f, 0.4f));

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(camera_position.x, 2, camera_position.z));
        modelMatrix *= RotateOY(rotation_angle);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.4f, -0.75f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.3f, 0.2f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0, 0.8f, 0.4f));

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(camera_position.x, 2, camera_position.z));
        modelMatrix *= RotateOY(rotation_angle);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.4f, -0.75f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.3f, 0.2f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(0, 0.8f, 0.4f));

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(camera_position.x, 2, camera_position.z));
        modelMatrix *= RotateOY(rotation_angle);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.4f, -1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(1, 1, 0.9f));

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(camera_position.x, 2, camera_position.z));
        modelMatrix *= RotateOY(rotation_angle);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.4f, -1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(1, 1, 0.9f));

        //Si in final capul
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(camera_position.x, 2, camera_position.z));
        modelMatrix *= RotateOY(rotation_angle);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, -0.5f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f));
        RenderSimpleMesh(meshes["box"], shaders["LabShader"], modelMatrix, glm::vec3(1, 1, 0.9f));
    }
    else {
        //Daca suntem in modul first, se vor desena gloantele aferente, daca exista
        for (int i = 0; i < bullets_nr; i++) {
            modelMatrix = glm::mat4(1);
            modelMatrix = glm::translate(modelMatrix, bullets[i]);
            modelMatrix *= RotateOY(rotation_angles[i]);
            modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0.5f, -1.5f - bullets_off_z[i]));
            modelMatrix = glm::scale(modelMatrix, glm::vec3(0.4f, 0.4f, 0.4f));
            RenderSimpleMesh(meshes["sphere"], shaders["LabShader"], modelMatrix, glm::vec3(0, 0, 1));

            bullets_off_z[i] += 0.1f;
            
            /*
            Daca actualul proiectil atinge distanta maxima, sau exista coliziune cu zidurile sau un inamic,
            proiectilul va fi sters
            */
            if (bullets_off_z[i] > 5 ||
                check_bul_collision_enem(glm::vec3(bullets[i].x + sin(rotation_angles[i]) * (-1.5f - bullets_off_z[i]),
                    bullets[i].y + 0.5f, bullets[i].z + cos(rotation_angles[i]) * (-1.5f - bullets_off_z[i]))) ||
                check_bul_collision_walls(glm::vec3(bullets[i].x + sin(rotation_angles[i]) * (-1.5f - bullets_off_z[i]),
                    bullets[i].y + 0.5f, bullets[i].z + cos(rotation_angles[i]) * (-1.5f - bullets_off_z[i])))) {
                bullets.erase(bullets.begin() + i);
                rotation_angles.erase(rotation_angles.begin() + i);
                bullets_off_z.erase(bullets_off_z.begin() + i);
                bullets_nr--;
            }
        }
    }
}

glm::mat4 Maze::RotateOY(float radians)
{
    // TODO(student): Implement the rotation matrix
    return glm::transpose(
        glm::mat4(cos(radians), 0, sin(radians), 0,
            0, 1, 0, 0,
            (-1) * sin(radians), 0, cos(radians), 0,
            0, 0, 0, 1));

}

void Maze::FrameEnd()
{
    //DrawCoordinateSystem(camera->GetViewMatrix(), projectionMatrix);
}

//Metoda pentru randarea unui mesh cu o anumita culoare, utilizand shaderele normale
void Maze::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    glUseProgram(shader->program);

    //Se trimit culoarea si ceilalti parametri specifici
    glUniform3fv(glGetUniformLocation(shader->program, "object_color"), 1, glm::value_ptr(color));

    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    mesh->Render();
}

//Metoda pentru randarea unui inamic lovit, utilizand shader-ul de deformare
void Maze::RenderDeformMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color,
    float deltaTimeSeconds)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    glUseProgram(shader->program);

    //Se trimit culoarea si ceilalti parametri specifici
    glUniform3fv(glGetUniformLocation(shader->program, "object_color"), 1, glm::value_ptr(color));

    glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
    glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
    glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    /*
    In plus, va fi trimisa si diferenta dintre cadre pentru a se deforma inamicul cu o valoare diferita
    */
    GLint seconds = glGetUniformLocation(shader->program, "seconds");
    glUniform1f(seconds, deltaTimeSeconds);

    mesh->Render();
}

//Metoda ce verifica o coliziune dintre un glont si un inamic
bool Maze::check_bul_collision_enem(glm::vec3 bullet_position) {
    float y_cen = 1.5f;
    for (int i = 0; i < enem_slots.size(); i++) {
        pair <int, int> enem_coord = enem_slots[i];
        //Se calculeaza x-ul si z-ul centrului unui inamic
        float x_cen = MAZE_START_X + enem_coord.second * 3 - off_x;
        float z_cen = MAZE_START_Z + enem_coord.first * 3 - off_z;

        /*
        Apoi se va calcula distanta pana la inamic si se va verifica
        daca este mai mica decat raza unui proiectil
        */
        float x = max(x_cen - 0.5f, min(bullet_position.x, x_cen + 0.5f));
        float y = max(y_cen - 0.5f, min(bullet_position.y, y_cen + 0.5f));
        float z = max(z_cen - 0.5f, min(bullet_position.z, z_cen + 0.5f));
        float distance = sqrt((x - bullet_position.x) * (x - bullet_position.x) +
            (y - bullet_position.y) * (y - bullet_position.y) +
            (z - bullet_position.z) * (z - bullet_position.z));

        if (distance < BULLET_RADIUS) {
            //In caz afirmativ, se va verifica si faptul ca inamicul sa nu fi fost lovit deja
            if (find(hitted_enem.begin(), hitted_enem.end(), enem_coord) == hitted_enem.end()) {
                hitted_enem.push_back(enem_coord);
                timer_enem.push_back(3);
            }
            return true;
        }
    }
    return false;
}

//Metoda ce verifica daca exista coliziune intre un proiectil si zid
bool Maze::check_bul_collision_walls (glm::vec3 bullet_position) {
    float y_cen = 2;
    float offset_z = -MAZE_START_Z - 1.5f;
    float offset_x = 0;

    for (int i = 0; i < MAZE_SIZE; i++) {
        for (int j = 0; j < MAZE_SIZE; j++) {
            //Daca exista zid, se vor calcula x-ul si y-ul centrului
            if (maze[i][j] == 1) {
                float x_cen = MAZE_START_X + offset_x;
                float z_cen = -1.5f - offset_z;

                /*
                Apoi se va determina distanta pana la perete, iar daca este mai mica
                decat raza proiectilului exista coliziune
                */
                float x = max(x_cen - 1.5f, min(bullet_position.x, x_cen + 1.5f));
                float y = max(y_cen - 2, min(bullet_position.y, y_cen + 2));
                float z = max(z_cen - 1.5f, min(bullet_position.z, z_cen + 1.5f));
                float distance = sqrt((x - bullet_position.x) * (x - bullet_position.x) +
                    (y - bullet_position.y) * (y - bullet_position.y) +
                    (z - bullet_position.z) * (z - bullet_position.z));
                if (distance < BULLET_RADIUS) {
                    return true;
                }
            }
            offset_x += 3;
        }
        offset_x = 0;
        offset_z -= 3;
    }
    return false;
}

//Metoda ce verifica daca exista coliziune intre player si un inamic
void Maze::check_collision_enem(glm::vec3 player_position) {
    for (int i = 0; i < enem_slots.size(); i++) {
        pair <int, int> enem_coord = enem_slots[i];

        //Se calculeaza x-ul si z-ul unui inamic
        float x_cen = MAZE_START_X + enem_coord.second * 3 - off_x;
        float z_cen = MAZE_START_Z + enem_coord.first * 3 - off_z;

        /*
        Se verifica o intersectie intre player si inamic pe partea de x si z
        (cum player-ul nu sare, nu am mai luat in considerare si y-ul)
        */
        if ((x_cen - 0.5f <= player_position.x + 0.5f && x_cen + 0.5f >= player_position.x - 0.5f) &&
            (z_cen - 0.5f <= player_position.z + 0.5f && z_cen + 0.5f >= player_position.z - 0.5f)) {
            maze[enem_coord.first][enem_coord.second] = 0;
            enem_slots.erase(enem_slots.begin() + i);
            enemies_touched += 0.5f;
            //Se recalculeaza factorul de scalare pentru bara de viata
            life = (scaleX - enemies_touched) / scaleX;
            //Daca viata player-ului ajunge la 0, jocul se incheie
            if (fabs(life - 0.00f) < 0.01f) {
                cout << "Ai pierdut!\n";
                exit(0);
            }
            return;
        }
    }
}

//Metoda ce verifica o eventuala coliziune intre player si zid
bool Maze::check_collision(glm::vec3 player_position) {
    float length_x = 0.5f;
    float offset_z = -MAZE_START_Z - 1.5f;
    float offset_x = 0;
    for (int i = 0; i < MAZE_SIZE; i++) {
        for (int j = 0; j < MAZE_SIZE; j++) {
            if (maze[i][j] == 1 || (exit_coord.first == i && exit_coord.second == j)) {
                /*
                La fel ca la coliziunea cu un inamic, se vor calcula x si z pentru centru si se va
                verifica daca player-ul si zidul se intersecteaza
                */
                float x_cen = MAZE_START_X + offset_x;
                float z_cen = -1.5f - offset_z;
                //Daca se verifica iesirea, se va verifica de fapt prima celula din afara labirintului
                if (exit_coord.first == i && exit_coord.second == j) {
                    z_cen -= 3;
                }
                if ((x_cen - 1.5f <= player_position.x + 0.5f && x_cen + 1.5f >= player_position.x - 0.5f) &&
                    (z_cen - 1.5f <= player_position.z + 0.5f && z_cen + 1.5f >= player_position.z - 0.5f)) {
                    //Daca jucatorul iese din labirint, castiga
                    if (exit_coord.first == i && exit_coord.second == j) {
                        cout << "Ai castigat!\n";
                        exit(0);
                    }
                    return true;
                }
            }
            offset_x += 3;
        }
        offset_x = 0;
        offset_z -= 3;
    }
    return false;
}


void Maze::OnInputUpdate(float deltaTime, int mods)
{
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float cameraSpeed = SPEED_PLAYER;
        
        /*
        In functie de tastele apasate(w, a, s, d), camera se va deplasa pe o anumita directie,
        daca nu exista coliziuni
        */
        if (window->KeyHold(GLFW_KEY_W)) {
            /*
            Daca avem camera de tip third, se vor verifica eventualele coliziuni utilizand
            coordonatele locului unde este indreptata camera, deorece acolo este player-ul
            */
            if (renderCameraTarget && check_collision(camera->GetTargetPosition()) == false) {
                camera->MoveForward(deltaTime * cameraSpeed);
                if (check_collision(camera->GetTargetPosition()) == true)
                    camera->MoveForward(-deltaTime * cameraSpeed);
            }
            //Pentru camera first se verifica eventualele coliziuni utilizand coordonatele camerei
            if (!renderCameraTarget && check_collision(camera->position) == false) {
                camera->MoveForward(deltaTime * cameraSpeed);
                if (check_collision(camera->position) == true)
                    camera->MoveForward(-deltaTime * cameraSpeed);
            }
        }

        //Analog pentru restul tastelor
        if (window->KeyHold(GLFW_KEY_A)) {
            if (renderCameraTarget && check_collision(camera->GetTargetPosition()) == false) {
                camera->TranslateRight(-deltaTime * cameraSpeed);
                if (check_collision(camera->GetTargetPosition()) == true)
                    camera->TranslateRight(deltaTime * cameraSpeed);
            }
            if (!renderCameraTarget && check_collision(camera->position) == false) {
                camera->TranslateRight(-deltaTime * cameraSpeed);
                if (check_collision(camera->position) == true)
                    camera->TranslateRight(deltaTime * cameraSpeed);
            }
        }

        if (window->KeyHold(GLFW_KEY_S)) {
            if (renderCameraTarget && check_collision(camera->GetTargetPosition()) == false) {
                camera->MoveForward(-deltaTime * cameraSpeed);
                if (check_collision(camera->GetTargetPosition()) == true)
                    camera->MoveForward(deltaTime * cameraSpeed);
            }
            if (!renderCameraTarget && check_collision(camera->position) == false) {
                camera->MoveForward(-deltaTime * cameraSpeed);
                if (check_collision(camera->position) == true)
                    camera->MoveForward(deltaTime * cameraSpeed);
            }

        }

        if (window->KeyHold(GLFW_KEY_D)) {
            if (renderCameraTarget && check_collision(camera->GetTargetPosition()) == false) {
                camera->TranslateRight(deltaTime * cameraSpeed);
                if (check_collision(camera->GetTargetPosition()) == true)
                    camera->TranslateRight(-deltaTime * cameraSpeed);
            }
            if (!renderCameraTarget && check_collision(camera->position) == false) {
                camera->TranslateRight(deltaTime * cameraSpeed);
                if (check_collision(camera->position) == true)
                    camera->TranslateRight(-deltaTime * cameraSpeed);
            }
        }

        //Apoi se vor verifica si eventualele coliziuni cu inamicii
        if (renderCameraTarget) {
            check_collision_enem(camera->GetTargetPosition());
        }
        if (!renderCameraTarget) {
            check_collision_enem(camera->position);
        }
    }
}


void Maze::OnKeyPress(int key, int mods)
{
    //Daca se apasa tasta CTRL, se va schimba camera
    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL) {
        renderCameraTarget = !renderCameraTarget;
        //Daca se trece de pe first pe third, camera se va muta in locul unde se afla jucatorul
        if (!renderCameraTarget) {
            camera->TranslateForward(camera->distanceToTarget);
        }
        //Daca se trece de pe third pe first, camera se va muta in spatele jucatorului
        else {
            camera->TranslateForward(-camera->distanceToTarget);
        }
    }

    //Daca se apasa SPACE, se va lansa un nou proiectil
    if (key == GLFW_KEY_SPACE && !renderCameraTarget) {
        //Se retin unghiul curent de rotatie si pozitia unde se afla camera
        rotation_angles.push_back(rotation_angle);
        bullets.push_back(camera->position);
        bullets_off_z.push_back(0.0f);
        bullets_nr++;
    }
}


void Maze::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Maze::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    /*
    Daca se va tine apasat click dreapta, camera se va roti dupa mouse utilizand functiile aferente
    pentru third sau first
    */
    if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        float sensivityOX = 0.001f;
        float sensivityOY = 0.001f;
        //Se actualizeaza si unghiul de rotatie pentru a sti unde va fi desenat jucatorul
        rotation_angle -= deltaX * sensivityOX;

        if (!renderCameraTarget) {
            camera->RotateFirstPerson_OX(-deltaY * sensivityOX);
            camera->RotateFirstPerson_OY(-deltaX * sensivityOY);
        }

        if (renderCameraTarget) {
            camera->RotateThirdPerson_OX(-deltaY * sensivityOX);
            camera->RotateThirdPerson_OY(-deltaX * sensivityOY);
        }
    }
}


void Maze::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    //Daca se apasa click dreapta, cursorul dispare pentru a permite rotirea camerei
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT))
    {
        window->DisablePointer();
    }

    //Se pot lansa proiectile si folosind click stanga
    if (button == GLFW_MOUSE_BUTTON_2 && !renderCameraTarget)
    {
        rotation_angles.push_back(rotation_angle);
        bullets.push_back(camera->position);
        bullets_off_z.push_back(0.0f);
        bullets_nr++;
    }

    //De asemenea, camera se poate schimba la click dreapta
    if (button == GLFW_MOUSE_BUTTON_3)
    {
        renderCameraTarget = !renderCameraTarget;
        if (!renderCameraTarget) {
            camera->TranslateForward(camera->distanceToTarget);
        }
        else {
            camera->TranslateForward(-camera->distanceToTarget);
        }
    }
}


void Maze::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    //Daca nu se mai tine apasat click dreapta, cursorul reapare
    if (IS_BIT_SET(button, GLFW_MOUSE_BUTTON_RIGHT))
    {
        window->ShowPointer();
    }
}


void Maze::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}


void Maze::OnWindowResize(int width, int height)
{
}
