#include "UI.h"
#include "Ball.h"
#include "Gravity.h"

#include "imgui.h"
#include "imgui_impl_glut.h"
#include "imgui_impl_opengl3.h"

void RenderMenus(int height, int width, vector<BallInstance>& gBalls, int& gSelected, vector<BallInstance>& gAsteroids)
{
    static bool showPlanetCreator = false;

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGLUT_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Control Panel");

    if(ImGui::Button("Add Planet")){
        showPlanetCreator = true;
    }

    if(ImGui::Button("Reset")){
        gSelected = -1;
        gBalls.clear();
        gAsteroids.clear();
        gTrails.clear();
    }

    ImGui::SliderFloat("Time scale", &gTimeScale, 0.0f, 20.0f);

    if(ImGui::Button("Solar system")){
        gSelected = -1;
        SolarSystemDemo();
    }

    ImGui::Separator();
    for(int i = 0; i < (int)gBalls.size(); i++){
        if(gBalls[i].dead)  continue;
        char label[32];
        snprintf(label, sizeof(label), "Ball %d", i + 1);

        if(ImGui::Selectable(label, gSelected == i)){
            gSelected = i;
        }
    }

    if(gSelected >= 0 && gSelected < (int)gBalls.size()){
        ImGui::Separator();
        ImGui::ColorEdit3("Color", gBalls[gSelected].color);
        ImGui::DragFloat3("Position", gBalls[gSelected].pos, 0.05f);
        ImGui::DragFloat3("Velocity", gBalls[gSelected].vel, 0.05f);
        ImGui::DragFloat("Mass", &gBalls[gSelected].mass, 0.05f);

        if(ImGui::Button("Follow selected")) gFollowing = true;
        ImGui::SameLine();

        if(ImGui::Button("Stop following")) gFollowing = false;

        if(gFollowing) ImGui::Text("Following Ball %d", gSelected + 1);
    }

    ImGui::End();

    RenderPlanetCreatorMenu(gBalls, gSelected, showPlanetCreator);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void RenderPlanetCreatorMenu(vector<BallInstance>& gBalls, int& gSelected, bool& showPlanetCreator)
{
    if(!showPlanetCreator)  return;

    static float pos[3]  = { 10.0f, 0.0f, 0.8f };
    static float vel[3]  = { 0.0f, 0.0f, 0.0f };
    static float mass    = 0.05f;
    static float dist    = 10.0f;
    static bool  autoOrbit = true;

    if(ImGui::Begin("Planet Creator", &showPlanetCreator)){

        ImGui::Text("Create a new planet");
        ImGui::Separator();

        ImGui::InputFloat("Mass", &mass);
        ImGui::Checkbox("Start on a circular orbit around the heaviest ball", &autoOrbit);

        if(autoOrbit){
            ImGui::InputFloat("Distance", &dist);
        } else{
            ImGui::InputFloat3("Position", pos);
            ImGui::InputFloat3("Velocity", vel);
        }

        if(ImGui::Button("Add Planet")){
            static const float palette[6][3] = {
                {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f},
                {1.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 1.0f}
            };

            const int n = (int)gBalls.size();
            const int c = FindHeaviestBall();

            BallInstance b;

            b.mass = mass;
            memcpy(b.color, palette[n % 6], sizeof(b.color));

            if(autoOrbit && c >= 0 && dist > 0.1f){
                const BallInstance& cb = gBalls[c];
                const float v = sqrtf(GravitationalConstant * cb.mass / dist);

                b.pos[0] = cb.pos[0] + dist;
                b.pos[1] = cb.pos[1];
                b.pos[2] = cb.pos[2];

                b.vel[0] = cb.vel[0];
                b.vel[1] = cb.vel[1];
                b.vel[2] = cb.vel[2] + v;

            } else{
                for(int k = 0; k < 3; k++){
                    b.pos[k] = pos[k];
                    b.vel[k] = vel[k];
                }
            }

            gBalls.push_back(b);
            gSelected = n;
        }

        ImGui::SameLine();
        if(ImGui::Button("Cancel")) showPlanetCreator = false;
    }

    ImGui::End();
}
