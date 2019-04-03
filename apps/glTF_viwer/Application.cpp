#include "Application.hpp"
#include <iostream>
#include <unordered_set>
#include <algorithm>
#include <imgui.h>
#include <glmlv/Image2DRGBA.hpp>
#include <glmlv/scene_loading.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <tiny_gltf.h>

int Application::run()
{
    float clearColor[3] = {0.2f, 0.3f, 0.3f};
    // Put here code to run before rendering loop
    // Loop until the user closes the window
    for (auto iterationCount = 0u; !m_GLFWHandle.shouldClose(); ++iterationCount)
    {
        const auto seconds = glfwGetTime();
        // Put here rendering code
        const auto viewportSize = m_GLFWHandle.framebufferSize();
        glViewport(0, 0, viewportSize.x, viewportSize.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0f);
        m_projMatrix = glm::perspective(70.0f, float(viewportSize.x) / viewportSize.y, 0.01f, 100.0f);
        m_viewMatrix = m_viewController.getViewMatrix();
        glUniform3fv(m_uDirectionalLightDirLocation, 1, glm::value_ptr(glm::vec3(m_viewMatrix * glm::vec4(glm::normalize(m_DirLightDirection), 0))));
        glUniform3fv(m_uDirectionalLightIntensityLocation, 1, glm::value_ptr(m_DirLightColor * m_DirLightIntensity));
        glUniform3fv(m_uPointLightPositionLocation, 1, glm::value_ptr(glm::vec3(m_viewMatrix * glm::vec4(m_PointLightPosition, 1))));
        glUniform3fv(m_uPointLightIntensityLocation, 1, glm::value_ptr(m_PointLightColor * m_PointLightIntensity));
        // 激活纹理
        glActiveTexture(GL_TEXTURE0);
        // 将uniform与位置0绑定
        glUniform1i(m_uKdSamplerLocation, 0);
        // 设置采样模式
        glBindSampler(0, m_uKdSamplerLocation);
        drawModel(m_model);
        // 解绑采样器
        glBindSampler(0, 0);
        // GUI code:
        glmlv::imguiNewFrame();
        {
            ImGui::Begin("GUI");
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            if (ImGui::ColorEdit3("clearColor", clearColor))
            {
                glClearColor(clearColor[0], clearColor[1], clearColor[2], 1.0f);
            }
            if (ImGui::CollapsingHeader("Directional Lighting"))
            {
                ImGui::ColorEdit3("DirLightColor", glm::value_ptr(m_DirLightColor));
                ImGui::DragFloat("DirLightIntensity", &m_DirLightIntensity, 0.1f, 0.0f, 100.0f);
                if (ImGui::DragFloat("Phi Angle", &m_DirLightPhiAngleDegrees, 1.0f, 0.0f, 360.0f) ||
                    ImGui::DragFloat("Theta Angle", &m_DirLightThetaAngleDegrees, 1.0f, 0.0f, 180.0f))
                {
                    m_DirLightDirection = computeDirectionVector(glm::radians(m_DirLightPhiAngleDegrees), glm::radians(m_DirLightThetaAngleDegrees));
                }
            }
            if (ImGui::CollapsingHeader("Specular Lighting"))
            {
                ImGui::ColorEdit3("SpecularLightingColor", glm::value_ptr(m_PointLightColor));
                ImGui::DragFloat("SpecularLightingIntensity", &m_PointLightIntensity, 0.1f, 0.0f, 15000.0f);
                ImGui::InputFloat3("Position", glm::value_ptr(m_PointLightPosition));
            }
            ImGui::End();
        }
        glmlv::imguiRenderFrame();
        // 事件监听与处理
        glfwPollEvents();
        // 交换缓冲区
        m_GLFWHandle.swapBuffers();
        auto ellapsedTime = glfwGetTime() - seconds;
        auto guiHasFocus = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
        if (!guiHasFocus)
        {
            // Put here code to handle user interactions
            m_viewController.update(float(ellapsedTime));
        }
    }
    return 0;
}

Application::Application(int argc, char **argv) : m_AppPath{glmlv::fs::path{argv[0]}},
                                                  m_AppName{m_AppPath.stem().string()},
                                                  m_ImGuiIniFilename{m_AppName + ".imgui.ini"},
                                                  m_ShadersRootPath{m_AppPath.parent_path() / "shaders"}
{
    if (argc < 2)
    {
        std::cerr << "Enter the path to the model." << argv[0] << " <../>" << std::endl;
        exit(-1);
    }
    path = argv[1];
    ImGui::GetIO().IniFilename = m_ImGuiIniFilename.c_str(); // At exit, ImGUI will store its windows positions in this file
    // Put here initialization code
    const GLint positionAttrLocation = 0;
    const GLint normalAttrLocation = 1;
    const GLint texCoordsAttrLocation = 2;
    m_attribs["POSITION"] = positionAttrLocation;
    m_attribs["NORMAL"] = normalAttrLocation;
    m_attribs["TEXCOORD_0"] = texCoordsAttrLocation;

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_FRAMEBUFFER_SRGB);

    m_program = glmlv::compileProgram({m_ShadersRootPath / m_AppName / "forward.vs.glsl", m_ShadersRootPath / m_AppName / "forward.fs.glsl"});
    m_program.use();
    m_uModelViewProjMatrixLocation = glGetUniformLocation(m_program.glId(), "uModelViewProjMatrix");
    m_uModelViewMatrixLocation = glGetUniformLocation(m_program.glId(), "uModelViewMatrix");
    m_uNormalMatrixLocation = glGetUniformLocation(m_program.glId(), "uNormalMatrix");
    m_uDirectionalLightDirLocation = glGetUniformLocation(m_program.glId(), "uDirectionalLightDir");
    m_uDirectionalLightIntensityLocation = glGetUniformLocation(m_program.glId(), "uDirectionalLightIntensity");
    m_uPointLightPositionLocation = glGetUniformLocation(m_program.glId(), "uPointLightPosition");
    m_uPointLightIntensityLocation = glGetUniformLocation(m_program.glId(), "uPointLightIntensity");
    m_uKdLocation = glGetUniformLocation(m_program.glId(), "uKd");
    m_uKdSamplerLocation = glGetUniformLocation(m_program.glId(), "uKdSampler");
    m_viewController.setViewMatrix(glm::lookAt(glm::vec3(0, 0, -3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
    m_viewController.setSpeed(8.0f);
    glActiveTexture(GL_TEXTURE0);
    loadModel();
}

void Application::loadModel()
{
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;
    bool ret = loader.LoadASCIIFromFile(&m_model, &err, &warn, path);
    if (!warn.empty())
    {
        printf("Warning: %s\n", warn.c_str());
    }
    if (!err.empty())
    {
        printf("Oops! Error occurs: %s\n", err.c_str());
    }
    if (!ret)
    {
        printf("Failed to resolve glTF\n");
        return;
    }
    std::cout << "Property of current glTF:\n"
              << m_model.accessors.size() << " accessors\n"
              << m_model.animations.size() << " animations\n"
              << m_model.buffers.size() << " buffers\n"
              << m_model.bufferViews.size() << " bufferViews\n"
              << m_model.materials.size() << " materials\n"
              << m_model.meshes.size() << " meshes\n"
              << m_model.nodes.size() << " nodes\n"
              << m_model.textures.size() << " textures\n"
              << m_model.images.size() << " images\n"
              << m_model.skins.size() << " skins\n"
              << m_model.samplers.size() << " samplers\n"
              << m_model.cameras.size() << " cameras\n"
              << m_model.scenes.size() << " scenes\n"
              << m_model.lights.size() << " lights\n";
    // init and generate a vector containing all buffers
    std::vector<GLuint> scene_buffers(m_model.buffers.size());
    glGenBuffers(scene_buffers.size(), scene_buffers.data());
    // 加载VAO和VBO
    for (size_t i = 0; i < scene_buffers.size(); ++i)
    {
        const tinygltf::BufferView &bufferView = m_model.bufferViews[i];
        glBindBuffer(bufferView.target, scene_buffers[i]);
        glBufferStorage(bufferView.target, m_model.buffers[i].data.size(), m_model.buffers[i].data.data(), 0);
        glBindBuffer(bufferView.target, 0);
    }
    // 加载网格
    for (size_t i = 0; i < m_model.meshes.size(); ++i)
    {
        // 加载当前网格
        const tinygltf::Mesh &mesh = m_model.meshes[i];
        for (int j = 0; j < mesh.primitives.size(); ++j)
        {
            const tinygltf::Primitive &prim = mesh.primitives[j];
            GLuint vao_primitive = 0;
            glGenVertexArrays(1, &vao_primitive);
            glBindVertexArray(vao_primitive);
            // 初始化绑定IBO
            tinygltf::Accessor &indexAccessor = m_model.accessors[prim.indices];
            tinygltf::BufferView &bufferView = m_model.bufferViews[indexAccessor.bufferView];
            int &bufferId = bufferView.buffer;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, scene_buffers[bufferId]);
            std::map<std::string, int>::const_iterator it(prim.attributes.begin());
            std::map<std::string, int>::const_iterator itEnd(prim.attributes.end());
            for (; it != itEnd; it++)
            {
                tinygltf::Accessor &accessor = m_model.accessors[it->second];
                bufferView = m_model.bufferViews[accessor.bufferView];
                bufferId = bufferView.buffer;
                glBindBuffer(GL_ARRAY_BUFFER, scene_buffers[bufferId]);
                // 初始化组建数量 默认为1
                int size = 1;
                if (accessor.type == TINYGLTF_TYPE_SCALAR)
                {
                    size = 1;
                }
                else if (accessor.type == TINYGLTF_TYPE_VEC2)
                {
                    size = 2;
                }
                else if (accessor.type == TINYGLTF_TYPE_VEC3)
                {
                    size = 3;
                }
                else if (accessor.type == TINYGLTF_TYPE_VEC4)
                {
                    size = 4;
                }
                else
                {
                    fprintf(stderr, "Wrong tinygltf type.");
                    exit(EXIT_FAILURE);
                }
                if ((it->first.compare("POSITION") == 0) || (it->first.compare("NORMAL") == 0) || (it->first.compare("TEXCOORD_0") == 0))
                {
                    if (m_attribs[it->first] >= 0)
                    {
                        int byteStride = accessor.ByteStride(m_model.bufferViews[accessor.bufferView]);
                        if (byteStride == -1)
                        {
                            fprintf(stderr, "\n");
                            exit(EXIT_FAILURE);
                        }
                        glEnableVertexAttribArray(m_attribs[it->first]);
                        glVertexAttribPointer(m_attribs[it->first], size, accessor.componentType, accessor.normalized ? GL_TRUE : GL_FALSE, byteStride, (const GLvoid *)(bufferView.byteOffset + accessor.byteOffset));
                    }
                }

                glBindBuffer(GL_ARRAY_BUFFER, 0);
            }
            // 将缓存加入向量中
            m_vaos.push_back(vao_primitive);
            m_primitives.push_back(prim);
            // 初始化并载入纹理
            glBindVertexArray(0);
            // 初始化漫反射纹理缓存
            m_diffuseTex.push_back(0);
            if (prim.material < 0)
            {
                continue;
            }
            tinygltf::Material &mat = m_model.materials[prim.material];
            const auto &parameter = mat.values["baseColorTexture"];
            tinygltf::Texture &tex = m_model.textures[parameter.TextureIndex()];
            if (tex.source > -1 && tex.source < m_model.images.size())
            {
                tinygltf::Image &image = m_model.images[tex.source];
                GLuint texId;
                glGenTextures(1, &texId);
                glBindTexture(GL_TEXTURE_2D, texId);
                GLenum format = GL_RGBA;
                if (image.component == 1)
                {
                    format = GL_RED;
                }
                else if (image.component == 2)
                {
                    format = GL_RG;
                }
                else if (image.component == 3)
                {
                    format = GL_RGB;
                }
                else
                {
                    pass;
                }
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGB32F, image.width, image.height);
                glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_RGBA, GL_UNSIGNED_BYTE, &image.image[0]);
                glBindTexture(GL_TEXTURE_2D, 0);
                m_diffuseTex.back() = texId;
            }
        }
    }
    glGenSamplers(1, &m_textureSampler);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glSamplerParameteri(m_textureSampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Application::DrawNode(tinygltf::Model &model, const tinygltf::Node &node, glm::mat4 m)
{
    glm::mat4 modelMatrix = glm::mat4(1);
    if (node.matrix.size() == 16)
    {
        modelMatrix = glm::make_mat4(node.matrix.data());
    }
    else
    {
        if (node.translation.size() == 3)
        {
            glm::vec3 translate = glm::make_vec3(node.translation.data());
            modelMatrix = glm::translate(modelMatrix, translate);
        }
        if (node.rotation.size() == 4)
        {
            glm::vec3 rotate(node.rotation[0], node.rotation[1], node.rotation[2]);
            modelMatrix = glm::rotate(modelMatrix, (float)node.rotation[3], rotate);
        }
        if (node.scale.size() == 3)
        {
            glm::vec3 scale = glm::make_vec3(node.scale.data());
            modelMatrix = glm::scale(modelMatrix, scale);
        }
    }
    modelMatrix = modelMatrix * m;
    if (node.mesh > -1)
    {
        DrawMesh(node.mesh, modelMatrix);
    }
    // 绘制子节点
    for (size_t i = 0; i < node.children.size(); i++)
    {
        DrawNode(model, model.nodes[node.children[i]], modelMatrix);
    }
}

void Application::DrawMesh(int meshId, glm::mat4 modelMatrix)
{
    const auto mvMatrix = m_viewMatrix * modelMatrix;
    const auto mvpMatrix = m_projMatrix * mvMatrix;
    const auto normalMatrix = glm::transpose(glm::inverse(mvMatrix));
    glUniformMatrix4fv(m_uModelViewProjMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvpMatrix));
    glUniformMatrix4fv(m_uModelViewMatrixLocation, 1, GL_FALSE, glm::value_ptr(mvMatrix));
    glUniformMatrix4fv(m_uNormalMatrixLocation, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    // 仅漫反射颜色
    glUniform3fv(m_uKdLocation, 1, glm::value_ptr(glm::vec3(1, 1, 1)));
    glBindTexture(GL_TEXTURE_2D, m_diffuseTex[meshId]);
    const tinygltf::Accessor &indexAccessor = m_model.accessors[m_primitives[meshId].indices];
    glBindVertexArray(m_vaos[meshId]);
    glDrawElements(getglTFMode(m_primitives[meshId].mode), indexAccessor.count, indexAccessor.componentType, (const GLvoid *)indexAccessor.byteOffset);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    // 绘制网格
    for (size_t i = 0; i < m_vaos.size(); ++i)
    {
        const tinygltf::Accessor &indexAccessor = m_model.accessors[m_primitives[i].indices];
        glBindVertexArray(m_vaos[i]);
        glDrawElements(getglTFMode(m_primitives[i].mode), indexAccessor.count, indexAccessor.componentType, (const GLvoid *)indexAccessor.byteOffset);
    }
}

GLenum Application::getglTFMode(int mode)
{
    if (mode == TINYGLTF_MODE_TRIANGLES)
    {
        return GL_TRIANGLES;
    }
    else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP)
    {
        return GL_TRIANGLE_STRIP;
    }
    else if (mode == TINYGLTF_MODE_TRIANGLE_FAN)
    {
        return GL_TRIANGLE_FAN;
    }
    else if (mode == TINYGLTF_MODE_POINTS)
    {
        return GL_POINTS;
    }
    else if (mode == TINYGLTF_MODE_LINE)
    {
        return GL_LINES;
    }
    else if (mode == TINYGLTF_MODE_LINE_LOOP)
    {
        return GL_LINE_LOOP;
    }
    else
    {
        fprintf(stderr, "Wrong gltf mode.\n");
        exit(EXIT_FAILURE);
    }
}

void Application::drawModel(tinygltf::Model &model)
{
    if (model.scenes.size() < 0)
    {
        fprintf(stderr, "Scene has no model.");
        exit(EXIT_FAILURE);
    }
    int scene_to_display = model.defaultScene > -1 ? model.defaultScene : 0;
    const tinygltf::Scene &scene = model.scenes[scene_to_display];
    for (size_t i = 0; i < scene.nodes.size(); i++)
    {
        DrawNode(model, model.nodes[scene.nodes[i]], glm::mat4(1));
    }
}
