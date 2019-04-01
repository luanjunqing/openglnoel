#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "filesystem.hpp"
#include "load_obj.hpp"
#include "GLProgram.hpp"
#include "GLMaterialProgram.hpp"
#include "ViewController.hpp"

namespace glmlv
{

class GLDeferredGPassProgram: public GLMaterialProgram {

public:
    GLDeferredGPassProgram(const fs::path& vs, const fs::path& fs):
        GLProgram(compileProgram({ vs.string(), fs.string() })),
        GLMaterialProgram()
        {}
};

} // namespace glmlv
