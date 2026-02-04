#define GLM_ENABLE_EXPERIMENTAL
#include "meshloader.hpp"
#include <fstream>
#include<iostream>
#include<nlohmann/json.hpp>
#include<glm/gtx/transform.hpp>
#include <vector>
#include "vertexbuffer.hpp"
#include "vertexarrayobject.hpp"
#include "indexbuffer.hpp"
namespace graf
{
    string ShapeType::Circle = "Circle";
    string ShapeType::Quad = "Quad";
    string ShapeType::Cylinder = "Cylinder";
    string ShapeType::Cone = "Cone";
    string ShapeType::Pyramid  = "Pyramid";

    MeshLoader* MeshLoader::m_instance=nullptr;
    MeshLoader::MeshLoader()
    {
        
    }
    VertexArrayObject* MeshLoader::loadMeshFromFile(const string& fileName)
    {
        VertexArrayObject* vao=nullptr;

        auto loader = getInstance();

        if(loader->m_vaos.count(fileName)==0)
        {

       
            string meshpath = "./meshes/"+fileName;
            ifstream file(meshpath);

            if(file.is_open())
            {
                cout<<"<------MeshLoader readling file "<<fileName<<"------->"<<endl;
                using namespace nlohmann;

                json data = json::parse(file);

                /*---------------------Parsing Mesh File--------------------------*/

                string meshName = data["MeshName"];
                int vertexAttributeCount = data["VertexAttributeCount"];
                vector<string> vertexAttributes = data["VertexAttributes"];
                int triangleCount = data["TriangleCount"];
                vector<float> vertices = data["Vertices"];
                vector<unsigned int> indices = data["Indices"];
                int vertexFloatCount = getVertexFloatCount(vertexAttributes);


                /*---------------------Creating Buffers--------------------------*/
                VertexBuffer* vbo = new VertexBuffer();
                vbo->create(&vertices[0],vertices.size()*sizeof(float));

                IndexBuffer* ibo = new IndexBuffer();
                ibo->create(indices.data(),indices.size()*sizeof(unsigned int));


                vao = new VertexArrayObject();
                vao->create();
                vao->setVertexBuffer(vbo);
                vao->setIndexBuffer(ibo);
                for(auto nextAttribute:vertexAttributes)
                    vao->addAttributes(toAttributeType(nextAttribute));
                vao->activateAttributes();

                /*---------------------Printing Log Info--------------------------*/

                cout<<setw(20)<<std::left<<"MeshName"<<":"<<meshName<<endl;
                cout<<setw(20)<<std::left<<"VertexAttributeCount"<<":"<<vertexAttributeCount<<endl;
                cout<<setw(20)<<std::left<<"VertexAttributes"<<":";
                for(auto nextAttribute:vertexAttributes)
                    cout<<nextAttribute<<" ";
                cout<<endl;
                cout<<setw(20)<<std::left<<"TriangleCount"<<":"<<triangleCount<<endl;
                
                cout<<setw(30)<<"Vertices"<<":"<<endl;
                    
                for(int i=0;i<vertices.size();i+=vertexFloatCount)
                {
                    cout<<std::right<<setw(10)<<"Vertex"<<""<<setw(10)<<i/vertexFloatCount<<setw(5)<<" =>";
                    for(int j =0;j<vertexFloatCount;j++)
                    {
                        cout<<setw(6)<<std::right<<vertices[i+j];
                    }
                    cout<<endl;
                }           
            }
            else
            {
                cout<<"<------MeshLoader cannot open "<<fileName<<"------->"<<endl;
            }
        }
        else
        {
            vao = loader->m_vaos[fileName];
        }
        return vao;

    }
    VertexArrayObject *MeshLoader::createMesh(const string &shapeType)
    {
        if(shapeType==ShapeType::Circle)
            return createCircle();
        if(shapeType==ShapeType::Cone)
            return createCone();
        if(shapeType == ShapeType::Pyramid)
            return createPyramid();
        if(shapeType == ShapeType::Cylinder)
            return createCylinder();
        return nullptr;
    }
    VertexArrayObject* MeshLoader::createCone()
    {
        VertexArrayObject* vao=nullptr;

        auto loader = getInstance();

        if(loader->m_vaos.count(ShapeType::Cone)==0)
        {
            vector<VertexUnlit> vertices;
            vector<unsigned int> indices;

            float angle =10;

            int vertexCount = 360.0f/angle;
            vertices.resize(vertexCount*2);
            for(int i=0;i<vertexCount;i++)
            {
                float nextAngle = i*angle;

                VertexUnlit vertex;
                vertex.position.x = cosf(glm::radians(nextAngle));
                vertex.position.y = 0;
                vertex.position.z = sinf(glm::radians(nextAngle));

                vertex.texCoords.s = 0.5f+0.5f*cosf(glm::radians(nextAngle));
                vertex.texCoords.t = 0.5f+0.5f*sinf(glm::radians(nextAngle));
                vertices[i] = vertex;

                vertices[i+vertexCount] = vertex;


            }
            float r = sqrtf(2);
            float totalAngle = 360.0f/r;
            angle = totalAngle/(vertexCount-1);


            VertexUnlit nextVertex;
            nextVertex.position = vertices[vertexCount].position;
            
            vertices.push_back(nextVertex);

            for(int i=0;i<=vertexCount;i++)
            {
                float nextAngle = angle*i;
                vertices[i+vertexCount].texCoords.s = 0.5f+0.5f*cosf(glm::radians(nextAngle));
                vertices[i+vertexCount].texCoords.t = 0.5f+0.5f*sinf(glm::radians(nextAngle));
           
            }


            VertexUnlit topVertex;
            topVertex.position = glm::vec3(0.0f,1.0f,0.0f);
            topVertex.texCoords = glm::vec2(0.5f,0.5f);
            vertices.push_back(topVertex);

            

            //----------------------------------------------------------------
            //---------------------Tabanın indeksleri bulunuyor---------------
            //----------------------------------------------------------------
            for(int i=0;i<vertexCount-2;i++)
            {
                indices.push_back(0);
                indices.push_back(i+1);
                indices.push_back(i+2);
            }
            //----------------------------------------------------------------
            //-----------------Yan yüzeyin indeksleri bulunuyor---------------
            //----------------------------------------------------------------   
            
            int topIndex = vertices.size()-1;
            for(int i=0;i<vertexCount-1;i++)
            {
                indices.push_back(topIndex);
                indices.push_back(vertexCount+i);
                indices.push_back(vertexCount+i+1);
            }
            indices.push_back(topIndex);
            indices.push_back(topIndex-1);
            indices.push_back(topIndex-2);

            VertexBuffer* vbo = new VertexBuffer();
            vbo->create(&vertices[0],vertices.size()*sizeof(VertexUnlit));

            IndexBuffer* ibo = new IndexBuffer();
            ibo->create(indices.data(),indices.size()*sizeof(unsigned int));


            vao = new VertexArrayObject();
            vao->create();
            vao->setVertexBuffer(vbo);
            vao->setIndexBuffer(ibo);
            vao->addAttributes(VertexAttributeType::Position);
            vao->addAttributes(VertexAttributeType::Texture);
            vao->activateAttributes();           

            loader->m_vaos[ShapeType::Cone] = vao;
        }
        else
        {
            vao = loader->m_vaos[ShapeType::Cone];
        }
        return vao;        
    }
    int MeshLoader::getVertexFloatCount(const vector<string> &attributes)
    {
        int floatCount =0;

        for(auto nextAttribute:attributes)
        {
            if( nextAttribute=="Position"||
                nextAttribute=="Normal"
                )
                floatCount+=3;
            if(nextAttribute=="Color")
                floatCount+=4;       
            if(nextAttribute=="Texture")
                floatCount+=2;            
        }

        return floatCount;
    }

    VertexArrayObject *MeshLoader::createCircle()
    {
        VertexArrayObject* vao=nullptr;

        auto loader = getInstance();

        if(loader->m_vaos.count(ShapeType::Circle)==0)
        {
            vector<VertexUnlit> vertices;
            vector<unsigned int> indices;

            float angle =10;

            int vertexCount = 360.0f/angle;

            for(int i=0;i<vertexCount;i++)
            {
                float nextAngle = i*angle;

                VertexUnlit vertex;
                vertex.position.x = cosf(glm::radians(nextAngle));
                vertex.position.y = sinf(glm::radians(nextAngle));
                vertex.position.z = 0.0f;

                vertex.texCoords.s = 0.5f+0.5f*cosf(glm::radians(nextAngle));
                vertex.texCoords.t = 0.5f+0.5f*sinf(glm::radians(nextAngle));
                vertices.push_back(vertex);
            }


            for(int i=0;i<vertexCount-2;i++)
            {
                indices.push_back(0);
                indices.push_back(i+1);
                indices.push_back(i+2);
            }


            VertexBuffer* vbo = new VertexBuffer();
            vbo->create(&vertices[0],vertices.size()*sizeof(VertexUnlit));

            IndexBuffer* ibo = new IndexBuffer();
            ibo->create(indices.data(),indices.size()*sizeof(unsigned int));


            vao = new VertexArrayObject();
            vao->create();
            vao->setVertexBuffer(vbo);
            vao->setIndexBuffer(ibo);
            vao->addAttributes(VertexAttributeType::Position);
            vao->addAttributes(VertexAttributeType::Texture);
            vao->activateAttributes();           

            loader->m_vaos[ShapeType::Circle] = vao;
        }
        else
        {
            vao = loader->m_vaos[ShapeType::Circle];
        }
        return vao;

    }

    VertexArrayObject* MeshLoader::createPyramid()
    {
        VertexArrayObject* vao = nullptr;
        auto loader = getInstance();

        // 1. Önbellek Kontrolü
        if(loader->m_vaos.count(ShapeType::Pyramid) == 0)
        {
            vector<VertexUnlit> vertices;
            vector<unsigned int> indices;

            // Piramidin 5 ana noktası vardır ama her yüzeyin (Taban, Ön, Sağ, Arka, Sol)
            // Texture koordinatları farklı olduğu için vertexleri tekrar tanımlıyoruz.

            // --- TABAN (Kare - 2 Üçgen) ---
            // Sol Alt
            vertices.push_back({ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) }); // 0
            // Sağ Alt
            vertices.push_back({ glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) }); // 1
            // Sağ Üst
            vertices.push_back({ glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 1.0f) }); // 2
            // Sol Üst
            vertices.push_back({ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 1.0f) }); // 3

            // Taban İndeksleri
            indices.push_back(0); indices.push_back(2); indices.push_back(1);
            indices.push_back(0); indices.push_back(3); indices.push_back(2);

            // --- YAN YÜZEYLER (4 Üçgen) ---
            // Tepe noktası (0, 0.5, 0)
            
            // Ön Yüz
            vertices.push_back({ glm::vec3( 0.0f,  0.5f,  0.0f), glm::vec2(0.5f, 1.0f) }); // 4 (Tepe)
            vertices.push_back({ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f) }); // 5 (Sol Alt)
            vertices.push_back({ glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f) }); // 6 (Sağ Alt)
            indices.push_back(4); indices.push_back(5); indices.push_back(6);

            // Sağ Yüz
            vertices.push_back({ glm::vec3( 0.0f,  0.5f,  0.0f), glm::vec2(0.5f, 1.0f) }); // 7
            vertices.push_back({ glm::vec3( 0.5f, -0.5f,  0.5f), glm::vec2(0.0f, 0.0f) }); // 8
            vertices.push_back({ glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) }); // 9
            indices.push_back(7); indices.push_back(8); indices.push_back(9);

            // Arka Yüz
            vertices.push_back({ glm::vec3( 0.0f,  0.5f,  0.0f), glm::vec2(0.5f, 1.0f) }); // 10
            vertices.push_back({ glm::vec3( 0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) }); // 11
            vertices.push_back({ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(1.0f, 0.0f) }); // 12
            indices.push_back(10); indices.push_back(11); indices.push_back(12);

            // Sol Yüz
            vertices.push_back({ glm::vec3( 0.0f,  0.5f,  0.0f), glm::vec2(0.5f, 1.0f) }); // 13
            vertices.push_back({ glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec2(0.0f, 0.0f) }); // 14
            vertices.push_back({ glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec2(1.0f, 0.0f) }); // 15
            indices.push_back(13); indices.push_back(14); indices.push_back(15);

            // 2. Buffer Oluşturma (Senin Kodunla Aynı)
            VertexBuffer* vbo = new VertexBuffer();
            vbo->create(&vertices[0], vertices.size() * sizeof(VertexUnlit));

            IndexBuffer* ibo = new IndexBuffer();
            ibo->create(indices.data(), indices.size() * sizeof(unsigned int));

            vao = new VertexArrayObject();
            vao->create();
            vao->setVertexBuffer(vbo);
            vao->setIndexBuffer(ibo);
            vao->addAttributes(VertexAttributeType::Position);
            vao->addAttributes(VertexAttributeType::Texture);
            vao->activateAttributes();

            loader->m_vaos[ShapeType::Pyramid] = vao;
        }
        else
        {
            vao = loader->m_vaos[ShapeType::Pyramid];
        }
        return vao;
    }

    VertexArrayObject* MeshLoader::createCylinder()
    {
        VertexArrayObject* vao = nullptr;
        auto loader = getInstance();

        if(loader->m_vaos.count(ShapeType::Cylinder) == 0)
        {
            vector<VertexUnlit> vertices;
            vector<unsigned int> indices;

            int segments = 32; // Yuvarlaklık kalitesi
            float radius = 0.5f;
            float height = 1.0f;
            float halfHeight = height / 2.0f;

            // --- YAN YÜZEYLER (Gövde) ---
            // Bir şerit gibi açılmış dikdörtgen düşünün
            for(int i = 0; i <= segments; i++)
            {
                float theta = (float)i / (float)segments * 2.0f * 3.14159f;
                float x = cosf(theta) * radius;
                float z = sinf(theta) * radius;
                float u = (float)i / (float)segments; // Texture yatay sarma

                // Üst Kenar Noktası
                VertexUnlit vTop;
                vTop.position = glm::vec3(x, halfHeight, z);
                vTop.texCoords = glm::vec2(u, 1.0f);
                vertices.push_back(vTop);

                // Alt Kenar Noktası
                VertexUnlit vBottom;
                vBottom.position = glm::vec3(x, -halfHeight, z);
                vBottom.texCoords = glm::vec2(u, 0.0f);
                vertices.push_back(vBottom);
            }

            // Yan Yüzey İndeksleri (Triangle Strip Mantığı)
            for(int i = 0; i < segments; i++)
            {
                int top1 = i * 2;
                int bottom1 = i * 2 + 1;
                int top2 = top1 + 2;
                int bottom2 = bottom1 + 2;

                // 1. Üçgen
                indices.push_back(top1);
                indices.push_back(bottom1);
                indices.push_back(top2);

                // 2. Üçgen
                indices.push_back(bottom1);
                indices.push_back(bottom2);
                indices.push_back(top2);
            }

            // --- KAPAKLAR (Daireler) ---
            // Yan yüzey bitti, kapaklar için vertices listesine ekleme yapıyoruz.
            
            // Üst Kapak Merkezi
            int topCenterIndex = vertices.size();
            vertices.push_back({ glm::vec3(0, halfHeight, 0), glm::vec2(0.5f, 0.5f) });

            // Alt Kapak Merkezi
            int bottomCenterIndex = vertices.size();
            vertices.push_back({ glm::vec3(0, -halfHeight, 0), glm::vec2(0.5f, 0.5f) });

            // Kapak Çember Noktaları
            for(int i = 0; i <= segments; i++)
            {
                float theta = (float)i / (float)segments * 2.0f * 3.14159f;
                float x = cosf(theta) * radius;
                float z = sinf(theta) * radius;
                
                // Texture mapping: (-1,1) aralığını (0,1) aralığına çekiyoruz
                float u = (x / radius) * 0.5f + 0.5f;
                float v = (z / radius) * 0.5f + 0.5f;

                // Üst Çember Noktası
                vertices.push_back({ glm::vec3(x, halfHeight, z), glm::vec2(u, v) });
                // Alt Çember Noktası
                vertices.push_back({ glm::vec3(x, -halfHeight, z), glm::vec2(u, v) });
            }

            // Kapak İndeksleri (Fan Mantığı)
            int startRingIndex = bottomCenterIndex + 1;
            for(int i = 0; i < segments; i++)
            {
                // Üst Kapak
                indices.push_back(topCenterIndex);
                indices.push_back(startRingIndex + (i * 2));
                indices.push_back(startRingIndex + ((i + 1) * 2));

                // Alt Kapak
                indices.push_back(bottomCenterIndex);
                indices.push_back(startRingIndex + ((i + 1) * 2) + 1);
                indices.push_back(startRingIndex + (i * 2) + 1);
            }

            // 2. Buffer Oluşturma
            VertexBuffer* vbo = new VertexBuffer();
            vbo->create(&vertices[0], vertices.size() * sizeof(VertexUnlit));

            IndexBuffer* ibo = new IndexBuffer();
            ibo->create(indices.data(), indices.size() * sizeof(unsigned int));

            vao = new VertexArrayObject();
            vao->create();
            vao->setVertexBuffer(vbo);
            vao->setIndexBuffer(ibo);
            vao->addAttributes(VertexAttributeType::Position);
            vao->addAttributes(VertexAttributeType::Texture);
            vao->activateAttributes();

            loader->m_vaos[ShapeType::Cylinder] = vao;
        }
        else
        {
            vao = loader->m_vaos[ShapeType::Cylinder];
        }
        return vao;
    }


    MeshLoader *MeshLoader::getInstance()
    {
        if(!m_instance)
            m_instance = new MeshLoader;
        return m_instance;
    }
}