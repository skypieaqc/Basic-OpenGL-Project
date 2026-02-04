cmake_minimum_required(VERSION 3.10)

include(CMakePrintHelpers)

# Proje ismi klasör adından alınıyor
string(FIND ${CMAKE_CURRENT_SOURCE_DIR} / BaslangicIndeks REVERSE)
math(EXPR BaslangicIndeks "${BaslangicIndeks}+1")
string(SUBSTRING ${CMAKE_CURRENT_SOURCE_DIR} ${BaslangicIndeks} -1 ProjeIsmi)

project(${ProjeIsmi})

set(Project_Dir ${CMAKE_SOURCE_DIR})

# Kutuphaneler klasörü (Proje klasörünün bir üstünde)
set(Kutuphane_Dir ${Project_Dir}/../Kutuphaneler)

set(Project_Include_Dir ${Project_Dir}/include)
set(Project_Src_Dir ${Project_Dir}/src)

# ImGui Dosyaları
set(ImGui_Source_files  
    ${Kutuphane_Dir}/imgui/imgui.cpp
    ${Kutuphane_Dir}/imgui/imgui_draw.cpp
    ${Kutuphane_Dir}/imgui/imgui_tables.cpp
    ${Kutuphane_Dir}/imgui/imgui_widgets.cpp
    ${Kutuphane_Dir}/imgui/backends/imgui_impl_glfw.cpp
    ${Kutuphane_Dir}/imgui/backends/imgui_impl_opengl3.cpp
)

# Proje Kaynak Dosyaları
set(Project_Source_Files 
    ${Project_Src_Dir}/main.cpp
    ${Project_Src_Dir}/glad.c
    ${Project_Src_Dir}/glwindow.cpp
    ${Project_Src_Dir}/shaderfile.cpp
    ${Project_Src_Dir}/shaderprogram.cpp
    ${Project_Src_Dir}/vertexbuffer.cpp
    ${Project_Src_Dir}/vertexarrayobject.cpp
    ${Project_Src_Dir}/meshloader.cpp        
    ${Project_Src_Dir}/texturemanager.cpp  
    ${Project_Src_Dir}/transform.cpp 
    ${Project_Src_Dir}/indexbuffer.cpp         
    ${Project_Src_Dir}/shadermanager.cpp  
    ${Project_Src_Dir}/sceneobject.cpp                    
    ${Project_Src_Dir}/model.cpp         
    ${Project_Src_Dir}/camera.cpp     
    ${Project_Src_Dir}/viewport.cpp             
    ${Project_Src_Dir}/scene.cpp                             
    ${ImGui_Source_files}    
)

# Include Dizinleri (Elle GLFW/include ekledik)
include_directories(
    ${Project_Include_Dir}
    ${Kutuphane_Dir}
    ${Kutuphane_Dir}/imgui
    ${Kutuphane_Dir}/GLFW/include
)

# Library Dizinleri (Elle GLFW/lib ekledik)
link_directories(
    ${Kutuphane_Dir}/GLFW/lib
)

add_executable(${PROJECT_NAME} ${Project_Source_Files})

# Linkleme: find_package yerine doğrudan dosya ismini veriyoruz.
# libglfw3dll.a -> DLL kullanmak için import kütüphanesi
# opengl32 -> Windows için gerekli
target_link_libraries(${PROJECT_NAME} glfw3dll opengl32)

# --- DLL KOPYALAMA İŞLEMİ ---
# Derleme bittiğinde glfw3.dll dosyasını .exe'nin yanına kopyalar.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${Kutuphane_Dir}/GLFW/lib/glfw3.dll"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
    COMMENT "glfw3.dll kopyalaniyor..."
)