#include "scene.hpp"
#include "model.hpp"
#include "shadermanager.hpp"
#include "shaderprogram.hpp"
#include "transform.hpp"
#include<stack>
#include <imgui/imgui.h>
#include "camera.hpp"
namespace graf
{
    int id = 1000;
    using namespace std;
    Scene::Scene()
    {
        m_root = new SceneObject();
        m_root->m_name = "Root";
    }
    void Scene::drawScene(ViewPort* viewPort, std::vector<graf::Camera*>& cameras)
    {

        stack<SceneObject*> objects;
        objects.push(m_root);


        for (auto cam : cameras) 
        {
            // İstersen burada "Kamera kendi kendini görmesin" diyebilirsin (FPS oyunları için bazen gerekir)
            // Ama genellikle silahın gözükmesi istendiği için filtrelemeden hepsini eklemek en iyisidir.
            objects.push(cam);
        }

        while(objects.empty()!=true)
        {
                auto next = objects.top();
                objects.pop();

                next->transform->updateRotation();
                next->transform->update();
                next->updateChilds();

                if(next->getType()==SceneObjectType::Model)
                {
                    Model* model = (Model*)next;
                    string shaderName = model->getShaderName();
                    string textureName = model->getTextureName();
                    ShaderManager::getProgram(shaderName)->use();
                    viewPort->actiaveCamera(ShaderManager::getProgram(shaderName));
                    model->draw(ShaderManager::getProgram(shaderName));
                    ShaderManager::getProgram(shaderName)->unuse();
                }
                for(auto nextChild:next->m_childs )
                {
                    objects.push(nextChild);
                }
        }


    }

    void Scene::drawGui(std::vector<graf::Camera*>& cameras)
    {
        id = 1000; // ID çakışmasını önlemek için başlangıç değeri

        // ---------------------------------------------------------
        // 1. SCENE PENCERESİ (Hiyerarşi ve Ekleme Butonları)
        // ---------------------------------------------------------
        ImGui::Begin("Scene");

        // --- Nesne Ekleme Butonları ---
        ImGui::Text("Yeni Nesne Ekle:");

        auto addObj = [&](std::string modelName) {
    
            graf::Model* newModel = graf::Model::loadModel(modelName);
            if(!newModel) return;

            // 1. Pozisyon ve Rotasyonu Sıfırla (Merkeze gelsin)
            newModel->transform->position = {0.0f, 0.0f, 0.0f};
            newModel->transform->euler    = {0.0f, 0.0f, 0.0f};
            
            // Scale'i başlangıçta 1 yapıyoruz
            newModel->transform->scale    = {1.0f, 1.0f, 1.0f};

            if (m_selectedObject) {
                m_selectedObject->addChild(newModel);

                // --- GLOBAL SCALE HESAPLAMA ---
                // Sadece babanın değil, tüm ataların boyutlarının çarpımını buluyoruz.
                float totalScaleX = 1.0f;
                float totalScaleY = 1.0f;
                float totalScaleZ = 1.0f;

                graf::SceneObject* iterator = m_selectedObject;
                
                // Root'a kadar yukarı tırman
                while(iterator != nullptr) {
                    if(iterator->transform) {
                        totalScaleX *= iterator->transform->scale.x;
                        totalScaleY *= iterator->transform->scale.y;
                        totalScaleZ *= iterator->transform->scale.z;
                    }
                    // Bir üstteki babaya geç (SceneObject.h'a eklediğimiz pointer)
                    iterator = iterator->m_parent; 
                }

                // --- TERSİNİ UYGULA (NÖTRLEME) ---
                // Eğer toplam büyüklük 5 ise, biz 1/5 yapalım ki sonuç 1 olsun.
                if(totalScaleX != 0) newModel->transform->scale.x = 1.0f / totalScaleX;
                if(totalScaleY != 0) newModel->transform->scale.y = 1.0f / totalScaleY;
                if(totalScaleZ != 0) newModel->transform->scale.z = 1.0f / totalScaleZ;

            } 
            else {
                // Eğer hiç kimse seçili değilse direkt Root'a ekle
                m_root->addChild(newModel);
            }

            addSceneObject(newModel);
        };
        
        if (ImGui::Button("Cube +"))        addObj("Cube_UnlitTextured");
        ImGui::SameLine();
        if (ImGui::Button("Cone +"))        addObj("Cone_UnlitTextured");
        ImGui::SameLine();
        if (ImGui::Button("Circle +"))      addObj("Circle_UnlitTextured");
        ImGui::SameLine();
        if (ImGui::Button("Piramit +"))     addObj("Pyramid_UnlitTextured"); 
        ImGui::SameLine();
        if (ImGui::Button("Silindir +"))    addObj("Cylinder_UnlitTextured");

        ImGui::Separator();
        ImGui::Text("Sahne Hiyerarsisi:");
        
        // Hiyerarşiyi çizmeye Root'tan başla
        if (m_root) {
            // Başlangıçta hiç seçim yoksa root seçili olsun (isteğe bağlı)
            if (m_selectedObject == nullptr) m_selectedObject = m_root; 
            
            drawNextGuiNode(m_root);
        }

        if (!cameras.empty()) 
        {
            ImGui::Separator();
            ImGui::Text("Kameralar");
            
            for (auto& cam : cameras) 
            {
                // Kamerayı recursive fonksiyonuna gönderiyoruz.
                // Böylece kameranın çocuğu (child) varsa onu da çizecek!
                drawNextGuiNode(cam); 
            }
        }

        ImGui::End();

        // ---------------------------------------------------------
        // 2. PROPERTIES PENCERESİ (Dinamik)
        // ---------------------------------------------------------
        ImGui::Begin("Properties");

        // Sadece bir nesne seçiliyse özelliklerini göster
        if (m_selectedObject != nullptr) 
        {
            ImGui::Text("Secili Obje: %s", m_selectedObject->getName().c_str());
            ImGui::Separator();

            ImGui::PushID(id++);
            // Başlık her zaman açık olsun diye DefaultOpen ekledim
            if(ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                // Artık m_root değil, m_selectedObject kontrol ediliyor
                ImGui::DragFloat3("Position", &m_selectedObject->transform->position.x, 0.1f);
                ImGui::DragFloat3("Rotation", &m_selectedObject->transform->euler.x, 1.0f);
                ImGui::DragFloat3("Scale", &m_selectedObject->transform->scale.x, 0.1f);
            }
            ImGui::PopID();
        }
        else 
        {
            ImGui::Text("Lutfen listeden bir nesne secin.");
        }

        ImGui::End();         
    }

    void Scene::addModelFromFile(const string &fileName)
    {
        Model* model = Model::loadModel(fileName);

        addSceneObject(model);
    }
    void Scene::addSceneObject(SceneObject *object)
    {
        if (object->m_parent != nullptr) 
        {
            // Zaten bir yeri var, hiyerarşiyi bozma.
            // Eğer ayrı bir "Render Listesi" tutuyorsan (vektör vb.), buraya ekleme kodu yazabilirsin.
            return; 
        }
        m_root->addChild(object);
    }

    void Scene::addViewPort(ViewPort * viewPort)
    {
        m_viewPortList.push_back(viewPort);
    }
    void Scene::draw(std::vector<graf::Camera*>& cameras)
    {
        for(auto nextViewPort:m_viewPortList)
        {
            nextViewPort->activateScissor();
            nextViewPort->clearBackground(0.0f, 0.4f, 0.7f, 1.0f);  
            nextViewPort->activate();  
            
            drawScene(nextViewPort, cameras);
        }
    }

    void Scene::drawNextGuiNode(SceneObject *current)
    {
    ImGui::PushID(id++);
    
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
    
    if (current->m_childs.empty()) node_flags |= ImGuiTreeNodeFlags_Leaf;

    if (m_selectedObject == current) {
        node_flags |= ImGuiTreeNodeFlags_Selected; // Eğer bu node seçiliyse vurgula
    }

    // Node oluşturuluyor
    // current->getName().c_str() nesnenin adını yazar
    bool node_open = ImGui::TreeNodeEx(current->getName().c_str(), node_flags);

    // Tıklanma Kontrolü: Eğer bu yazıya tıklanırsa seçili obje bu olsun
    if (ImGui::IsItemClicked()) {
        m_selectedObject = current;
    }

    // Eğer node açıksa çocuklarını çiz (Recursive)
    if(node_open)
    {
        for(auto nextChild : current->m_childs)
        {
            if (nextChild->m_parent == current) 
            {
                drawNextGuiNode(nextChild);
            }
        }
        ImGui::TreePop();
    } 
    ImGui::PopID();       
}

}
