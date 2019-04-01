#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "GLProgram.hpp"
#include "ViewController.hpp"
#include "CommonLighting.hpp"
#include "GLMaterialProgram.hpp"

namespace glmlv
{

class GLForwardRenderingProgram: public GLMaterialProgram, public GLCommonLightingProgram {

public:
    GLForwardRenderingProgram(const fs::path& vs, const fs::path& fs):
        GLProgram(compileProgram({ vs.string(), fs.string() })),
        GLMaterialProgram(),
        GLCommonLightingProgram()
        {}
};

} // namespace glmlv
