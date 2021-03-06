#include "nta/Animation2D.h"
#include "nta/Logger.h"

#define INVALID_LEN_MSG "Animation2D Error: Tried creating animation with 0 length"
#define INVALID_IDX_MSG "Animation2D Error: Tried creating animation start with nonexistent index"

namespace nta {
    SpriteSheet::SpriteSheet(ContextData& context, crstring file_path, crivec2 dims,
                             GLint minFilt, GLint magFilt) : dims(dims) {
        tex = context.getTexture(file_path, glm::vec2(0), minFilt, magFilt)
                     .get_data_or(GLTexture::no_texture());
    }
    void SpriteSheet::read_sprite_pixels(GLubyte* pixels, std::size_t index, 
                                         GLenum format) const {
        const std::size_t r = index/num_cols;
        const std::size_t c = index%num_cols;
        const glm::ivec2 dims = sprite_dims();

        const float top = dims.y * r;
        const float left = dims.x * c;

        GLuint fbo;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               tex.id, 0);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(left, top, dims.x, dims.y, format, GL_UNSIGNED_BYTE, pixels);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDeleteFramebuffers(1, &fbo);
    }

    Animation2D::Animation2D(const SpriteSheet& sheet, std::size_t start, std::size_t length, float speed) :
        m_sheet(sheet), m_time(0), m_speed(speed), m_start_index(start), m_length(length) {
        if (m_length == 0) {
            Logger::writeErrorToLog(INVALID_LEN_MSG, INVALID_VALUE);
        }
        if (m_start_index >= m_sheet.num_rows*m_sheet.num_cols) {
            Logger::writeErrorToLog(INVALID_IDX_MSG, INVALID_VALUE);
        }
    }
    Animation2D::Animation2D(ContextData& context, crstring file_path, crivec2 dims, std::size_t start, std::size_t length, float speed) :
        Animation2D(SpriteSheet(context, file_path, dims), start, length, speed) {
    }
    Animation2D::Animation2D(ContextData& context, crstring file_path, int num_cols, std::size_t start, std::size_t length, float speed) :
        Animation2D(SpriteSheet(context, file_path, num_cols), start, length, speed) {
    }
    glm::vec4 Animation2D::get_uv() const {
        return m_sheet.get_uv(get_index());
    }
    glm::vec4 Animation2D::get_flipped_uv() const {
        return m_sheet.get_flipped_uv(get_index());
    }
    glm::vec2 Animation2D::get_frame_dims() const {
        return m_sheet.get_frame_dims();
    }
    GLuint Animation2D::get_tex_id() const {
        return m_sheet.tex.id;
    }
    std::size_t Animation2D::get_index() const {
        return m_start_index + ((std::size_t)m_time)%m_length;
    }
    std::size_t Animation2D::get_start() const {
        return m_start_index;
    }
    std::size_t Animation2D::get_length() const {
        return m_length;
    }
    float Animation2D::get_speed() const {
        return m_speed;
    }
    float Animation2D::get_time() const {
        return m_time;
    }
    void Animation2D::switch_animation(std::size_t start, std::size_t length, float speed) {
        m_start_index = start;
        m_length = length;
        m_time = 0;
        m_speed = speed;

        if (m_length == 0) {
            Logger::writeErrorToLog(INVALID_LEN_MSG, INVALID_VALUE);
        }
        if (m_start_index >= m_sheet.num_rows*m_sheet.num_cols) {
            Logger::writeErrorToLog(INVALID_IDX_MSG, INVALID_VALUE);
        }
    }
    void Animation2D::set_speed(float speed) {
        m_speed = speed;
    }
    void Animation2D::step(float dt) {
        m_time += m_speed * dt;
    }
}