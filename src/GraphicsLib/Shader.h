#pragma once

namespace Shader {
    GLuint CreateProgramFromFiles(
            const std::string& vertex_shader, const std::string& fragment_shader);
    GLuint CreateProgramFromSource(
            const std::string& vertex_shader, const std::string& fragment_shader);
}
