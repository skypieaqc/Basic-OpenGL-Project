#define GLM_ENABLE_EXPERIMENTAL
#include "glwindow.hpp"

#include<glm/glm.hpp>
#include "shaderprogram.hpp"
#include "vertexbuffer.hpp"
#include "vertexarrayobject.hpp"
#include "meshloader.hpp"
#include "texturemanager.hpp"
#include "transform.hpp"
#include "shadermanager.hpp"
#include "model.hpp"
#include "camera.hpp"
#include "viewport.hpp"
#include "scene.hpp"
#include <glm/gtx/transform.hpp>
#include <imgui/imgui.h>
#include <vector>
#include <string>


using namespace std;
void printMatrix(const glm::mat4& mtx)
{
    if (ImGui::BeginTable("Matrix", 4, ImGuiTableFlags_Resizable | ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_Borders))
    {
                
                
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("%-.3f",mtx[0][0]);
        ImGui::TableNextColumn(); ImGui::Text("%-.3f",mtx[1][0]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[2][0]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[3][0]);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[0][1]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[1][1]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[2][1]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[3][1]);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[0][2]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[1][2]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[2][2]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[3][2]);
        ImGui::TableNextRow();
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[0][3]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[1][3]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[2][3]);
        ImGui::TableNextColumn(); ImGui::Text("%.3f",mtx[3][3]);
        ImGui::EndTable();
    }
}
glm::mat4 projectionMatrix(float fov,float aspect,float n,float f)
{
    glm::mat4 mtxProj(0);
    float tanHalfFov = tan(fov/2.0f);
    mtxProj[0][0] = 1.0f/(aspect*tanHalfFov); //parantezi unutma sonuç değişiyor.
    mtxProj[1][1] = 1.0f/tanHalfFov;
    mtxProj[2][2] = (f+n)/(f-n);
    mtxProj[2][3] = 1.0f;
    mtxProj[3][2] = -2*f*n/(f-n);
    return mtxProj;
}

void CreateCameraActor(graf::Scene& scene, std::vector<graf::Model*>& allModels, std::vector<graf::Camera*>& allCameras, float aspect) {
    
    // 1. Kamerayı Oluştur
    graf::Camera* newCam = new graf::Camera(60.0f, aspect, 0.1f, 100.0f);
    
    // Yeni kamerayı rastgele veya belirgin bir konuma koy
    // (Her basışta üst üste gelmesin diye count kullanabiliriz veya sabit bir offset)
    static int camCount = 0;
    camCount++;
    newCam->transform->position = glm::vec3(0.0f, 0.0f, 0.0f); 
    newCam->transform->scale = glm::vec3(1.0f); 

    // Kamerayı listeye ekle
    allCameras.push_back(newCam);

    // 2. Parça Ekleme Lambda'sı
    auto addPartToCamera = [&](std::string modelName, glm::vec3 localOffset, glm::vec3 targetScale, glm::vec3 targetEuler, std::string textureName) {
        graf::Model* newModel = graf::Model::loadModel(modelName);
        if(!newModel) return;

        // --- DÜZELTME 1: KONUMLANDIRMA ---
        // Eğer bir parent (newCam) varsa, position artık "Local Position"dır.
        // Yani kameranın merkezinden ne kadar uzakta olacağıdır.
        // localOffset örneğin (0, 0, -5.0f) gelirse kameranın 5 birim önünde oluşur.
        newModel->transform->position = localOffset; 

        newModel->transform->euler = targetEuler;
      
       // PARENT ATAMA (Burası doğruydu, aynen kalıyor)
        newCam->addChild(newModel); 
        newModel->m_parent = newCam;

        // GLOBAL SCALE HESAPLAMA (Senin Kodun - Aynen kalıyor)
        float totalScaleX = 1.0f, totalScaleY = 1.0f, totalScaleZ = 1.0f;
        graf::SceneObject* iterator = newCam; 
        
        while(iterator != nullptr) {
            if(iterator->transform) {
                totalScaleX *= iterator->transform->scale.x;
                totalScaleY *= iterator->transform->scale.y;
                totalScaleZ *= iterator->transform->scale.z;
            }
            iterator = iterator->m_parent; 
        }

        if(totalScaleX != 0) newModel->transform->scale.x = (1.0f / totalScaleX) * targetScale.x;
        if(totalScaleY != 0) newModel->transform->scale.y = (1.0f / totalScaleY) * targetScale.y;
        if(totalScaleZ != 0) newModel->transform->scale.z = (1.0f / totalScaleZ) * targetScale.z;

        // --- DÜZELTME 2: LİSTEYE EKLEME ---
        // Child objeleri de ana sahne listesine ekliyoruz ki çizilsinler.
        // ANCAK: Update fonksiyonunda "Parent varsa matrix'i parent'la çarp" mantığı yoksa
        // cisim kamerayla beraber hareket etmez.
        scene.addSceneObject(newModel);
        allModels.push_back(newModel);
    };

    // A. GÖVDE (Küp): Yatay dikdörtgen prizma
    addPartToCamera(
        "Cube_UnlitTextured", 
        glm::vec3(0.0f, 0.0f, -0.7f),       
        glm::vec3(0.5f, 0.3f, 0.4f),      
        glm::vec3(0.0f, -90.0f, 0.0f),
        "wall.jpg"      
    );

    addPartToCamera(
        "Pyramid_UnlitTextured",
        glm::vec3(0.0f, 0.0f, -0.2f),      
        glm::vec3(0.4f, 0.4f, 0.4f),       
        glm::vec3(-90.0f, 0.0f, 0.0f),
        "camera_black.jpg"       
    );

    addPartToCamera(
        "Cylinder_UnlitTextured",
        glm::vec3(0.0f, 0.450f, -0.5f),    
        glm::vec3(0.8f, 0.150f, 0.5f),      
        glm::vec3(-90.0f, 0.0f, 90.0f),
        "camera_black.jpg"      
    );

    // D. MAKARA 2 (Silindir - Arkadaki)
    // İstenen: Pos(0, 0.45, -1.1) | Rot(-90, 0, 90) | Scale(0.8, 0.15, 0.5)
    addPartToCamera(
        "Cylinder_UnlitTextured",
        glm::vec3(0.0f, 0.450f, -1.1f),     
        glm::vec3(0.8f, 0.150f, 0.5f),
        glm::vec3(-90.0f, 0.0f, 90.0f),
        "camera_black.jpg"      
    );

}

int main(int arc,char** argv)
{
    float width = 1600;
    float height = 900;
    graf::GLWindow window;
    window.create(width,height);

    // --- KAMERA YÖNETİMİ ---
    // Kameraları tutacak listemiz
    std::vector<graf::Camera*> cameras;


    // Hangi kamerayı kontrol ediyoruz? (Genelde büyük ekrandaki)
    int activeCamIndex = 0; 
    // İkinci viewport hangi kamerayı gösteriyor?
    int pipCamIndex = 1;

    std::vector<graf::Model*> addedModels;

    graf::ViewPort* viewport[2];
    graf::Scene scene;

    float aspect = (float)width / (float)height;

    CreateCameraActor(scene, addedModels, cameras, aspect);
    CreateCameraActor(scene, addedModels, cameras, aspect);
    
    // Viewport 0: Tam Ekran -> Başlangıçta 0. kamerayı (cam1) kullanıyor
    viewport[0] = new graf::ViewPort(0, 0, width, height);
    viewport[0]->attachCamera(cameras[activeCamIndex]);
    
    // Viewport 1: Küçük Ekran (Sağ Alt) -> Başlangıçta 1. kamerayı (cam2) kullanıyor
    float smallW = width / 4.0f;
    float smallH = height / 4.0f;
    viewport[1] = new graf::ViewPort(width - smallW, height - smallH, smallW, smallH);
    viewport[1]->attachCamera(cameras[pipCamIndex]);


    scene.addViewPort(viewport[0]);
    scene.addViewPort(viewport[1]);
    //scene.addSceneObject(model2);


    auto addModelToScene = [&](std::string modelName) {
        graf::Model* newModel = graf::Model::loadModel(modelName);
        
        if (newModel) {
            // İstersen yeni eklenenler üst üste binmesin diye rastgele konum verebilirsin
            // newModel->transform->position.x = (rand() % 10) - 5; 
            
            scene.addSceneObject(newModel); // Sahneye ekle (çizilmesi için)
            addedModels.push_back(newModel); // Listeye ekle (yönetmek için)
        }
    };

    //---------------------------------------------------------------------------
    //                  KLAVYE FONKSİYONU
    //---------------------------------------------------------------------------    



    window.setKeyboardFunction([&](int key,int action){

        graf::Camera* currentCam = cameras[activeCamIndex];
        bool moved = false;

        if(key==GLFW_KEY_A)
        {
            currentCam->transform->moveLeft();
            moved = true;
        } 
        if(key==GLFW_KEY_D) 
        {
            currentCam->transform->moveRight();
            moved = true;
        } 
        if(key==GLFW_KEY_W)
        {
            currentCam->transform->moveForward();
            moved = true;
        } 
        if(key==GLFW_KEY_S)
        {
            currentCam->transform->moveBackward();
            moved = true;
        }    

        if (moved) {
            // 1. Önce kameranın yeni x,y,z değerlerinden yeni MATRİSİNİ oluştur.
            currentCam->transform->update(); 

            // 2. Şimdi bu yeni matrisi çocuklara gönder.
            currentCam->updateChilds();
        }

    });
    double oldX;
    double oldY;
    window.mouseCursorPosition(oldX,oldY);
    bool isRightButtonPressed = false;
    window.setMouseButtonFunction([&](int button, int action, int mods ){
        
    
    });



    window.setMouseMoveFunction([&](double x,double y){

        double dx = x-oldX;
        double dy = y-oldY;
        if(window.isMouseRightButtonPressed())
        {
            graf::Camera* currentCam = cameras[activeCamIndex];
            currentCam->transform->euler.y+=dx;
            currentCam->transform->euler.x+=dy;
        }

        oldX = x;
        oldY =y ;
    });

    


    window.setGuiFunction([&](){

        scene.drawGui(cameras);
        
        // 2. Kamera Yönetim Penceresi
        ImGui::Begin("Kamera Yonetimi");

        // A) Yeni Kamera Ekleme Butonu
        if (ImGui::Button("Yeni Kamera Ekle (+)"))
        {
            CreateCameraActor(scene, addedModels, cameras, aspect);
        }

        ImGui::Separator();

        // B) Ana Ekran (Viewport 0) Hangi Kamerayı Göstersin?
        // Bu aynı zamanda klavyenin kontrol ettiği kameradır.
        std::string comboLabelMain = "Ana Ekran (Cam " + std::to_string(activeCamIndex) + ")";
        if (ImGui::BeginCombo("Main Viewport", comboLabelMain.c_str()))
        {
            for (int i = 0; i < cameras.size(); i++)
            {
                bool isSelected = (activeCamIndex == i);
                std::string label = "Camera " + std::to_string(i);
                if (ImGui::Selectable(label.c_str(), isSelected))
                {
                    activeCamIndex = i;
                    // Viewport 0'ın kamerasını değiştir
                    viewport[0]->attachCamera(cameras[activeCamIndex]);
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // C) Küçük Ekran (Viewport 1) Hangi Kamerayı Göstersin?
        std::string comboLabelPip = "Kucuk Ekran (Cam " + std::to_string(pipCamIndex) + ")";
        if (ImGui::BeginCombo("PIP Viewport", comboLabelPip.c_str()))
        {
            for (int i = 0; i < cameras.size(); i++)
            {
                bool isSelected = (pipCamIndex == i);
                std::string label = "Camera " + std::to_string(i);
                if (ImGui::Selectable(label.c_str(), isSelected))
                {
                    pipCamIndex = i;
                    // Viewport 1'in kamerasını değiştir
                    viewport[1]->attachCamera(cameras[pipCamIndex]);
                }
                if (isSelected) ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        // D) Seçili Kameranın Bilgisi
        ImGui::Text("Aktif Kamera Pos: %.1f, %.1f, %.1f", 
            cameras[activeCamIndex]->transform->position.x,
            cameras[activeCamIndex]->transform->position.y,
            cameras[activeCamIndex]->transform->position.z);

        ImGui::End();
    });


    auto createPartFromJson = [&](std::string jsonFileName, glm::vec3 pos, glm::vec3 scale, std::string displayName) 
    {
        // 1. JSON dosyasından modeli yükle (ground veya wall)
        // loadModel fonksiyonun zaten "./models/" + isim + ".json" yapıyor.
        graf::Model* part = graf::Model::loadModel(jsonFileName); 
        
        if (part) 
        {
            // 2. İsim Çakışmasını Önleme (ÖNEMLİ)
            // JSON'dan gelen isim "WallBase"dir. 
            // Biz bunu "Sol Duvar", "Zemin" vb. olarak güncelliyoruz ki panelde karışmasın.
            part->setName(displayName); // setName fonksiyonunu eklediğini varsayıyorum
            
            // 3. Transform Ayarları
            part->transform->position = pos;
            part->transform->scale = scale;
            part->transform->euler = {0.0f, 0.0f, 0.0f};

            // 4. Sahneye Ekle
            scene.addSceneObject(part);
        }
    };

    // --- ODA ÖLÇÜLERİ ---
    float roomW = 20.0f;    
    float roomL = 20.0f;    
    float roomH = 6.0f;     
    float thickness = 0.5f;

    // 1. ZEMİN (Ground)
    // Zemin hala 20x20 boyutunda
    createPartFromJson("ground", 
        glm::vec3(0.0f, -thickness/2.0f, 0.0f), 
        glm::vec3(roomW, thickness, roomL), 
        "Zemin");

    // 2. ARKA DUVAR
    // Position: Z = -20, Y = 6 (İstediğin gibi)
    // Scale: roomW (20) + Kenar Payları
    createPartFromJson("wall", 
        glm::vec3(0.0f, 6.0f, -20.0f), 
        glm::vec3(roomW + (2 * thickness), roomH, thickness), 
        "Arka Duvar");

    // 3. SOL DUVAR
    // Position: X = -19, Y = 6
    createPartFromJson("wall", 
        glm::vec3(-19.0f, 6.0f, 0.0f), 
        glm::vec3(thickness, roomH, roomL), 
        "Sol Duvar");

    // 4. SAĞ DUVAR
    // Position: X = 19, Y = 6
    createPartFromJson("wall", 
        glm::vec3(19.0f, 6.0f, 0.0f), 
        glm::vec3(thickness, roomH, roomL), 
        "Sag Duvar");

        
    window.setRenderFunction([&](){
        scene.draw(cameras);

    });

    window.begin();
    exit(EXIT_SUCCESS);
}