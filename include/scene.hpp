#pragma once
#include "sceneobject.hpp"
#include "viewport.hpp"
#include <vector>

namespace graf
{

    class Model;
    class Scene
    {
    public:
        Scene();

        void drawScene(ViewPort* viewPort , std::vector<graf::Camera*>& cameras);
      
        void drawGui(std::vector<graf::Camera*>& cameras);
        void addModelFromFile(const string& fileName);
        void addSceneObject(SceneObject* object);
        void addViewPort(ViewPort*);
        void draw(std::vector<graf::Camera*>& cameras);
    private:
        void drawNextGuiNode(SceneObject* current);
        ViewPortList m_viewPortList;
        SceneObject* m_root;
        SceneObject* m_selectedObject = nullptr;
    };

}