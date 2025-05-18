#include "ParticleSystemEditorPanel.h"

void ParticleSystemEditorPanel::Render()
{
    /* Asset Details */
    ImGui::SetNextWindowPos(ImVec2(0.f, 200.f));
    ImGui::SetNextWindowSize(ImVec2(200.f, 400.f));

    ImGui::Begin("Properties", nullptr);


    ImGui::End();


    /* Asset Browser */
    ImGui::SetNextWindowPos(ImVec2(200.f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(600.f, 600.f));

    ImGui::Begin("Emitters", nullptr);


    ImGui::End();
}

void ParticleSystemEditorPanel::OnResize(HWND hWnd)
{

}
