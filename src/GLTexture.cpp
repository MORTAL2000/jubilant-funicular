#ifdef NTA_USE_DEVIL
    #include <IL/il.h>
    #include <IL/ilu.h>
#else
    #include <CImg.h>
#endif

#include "nta/GLTexture.h"
#include "nta/Logger.h"
#include "nta/utils.h"

namespace nta {
    void GLTexture::init(const RawTexture& raw, GLint minFilt, GLint magFilt) {
        GLuint old_id = id;
        if (id == 0) {
            Logger::writeToLog("Initializing GLTexture from RawTexture...");
        } else {
            Logger::writeToLog("Updating GLTexture using RawTexture...");
        }
        width = raw.width;
        height = raw.height;
        if (raw.data) {
            if (id == 0) glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, raw.width, raw.height, 0,
                         raw.format, GL_UNSIGNED_BYTE, raw.data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilt);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilt);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);
            if (old_id == 0) {
                Logger::writeToLog("Created GLTexture with id " + utils::to_string(id));
            } else {
                Logger::writeToLog("Updated GLTexture");
            }
        } else {
            Logger::writeToLog("The RawTexture was empty");
        }
    }
    GLTexture GLTexture::combine_horizontal(const GLTexture& lhs, const GLTexture& rhs) {
        if (!lhs.is_valid()) return rhs;
        if (!rhs.is_valid()) return lhs;
        
        GLTexture ret;
        glGenTextures(1, &ret.id);
        ret.width = lhs.width + rhs.width;
        ret.height = std::max(lhs.height, rhs.height);

        GLubyte* pixels = new GLubyte[ret.width*ret.height*4];
        GLubyte* lpixels = new GLubyte[lhs.width*lhs.height*4];
        GLubyte* rpixels = new GLubyte[rhs.width*rhs.height*4];

        glBindTexture(GL_TEXTURE_2D, lhs.id);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, lpixels);
        glBindTexture(GL_TEXTURE_2D, rhs.id);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rpixels);

        memset(pixels, 0, ret.width*ret.height*4);
        for (int r = 0; r < ret.height; r++) {
            if (r < lhs.height) {
                memcpy(&pixels[r*ret.width*4], &lpixels[r*lhs.width*4], lhs.width*4);
            }
            if (r < rhs.height) {
                memcpy(&pixels[r*ret.width*4 + lhs.width*4], &rpixels[r*rhs.width*4], 
                       rhs.width*4);
            }
        }

        glBindTexture(GL_TEXTURE_2D, ret.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ret.width, ret.height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        delete[] pixels;
        delete[] lpixels;
        delete[] rpixels;

        return ret;
    }
    GLTexture GLTexture::combine_vertical(const GLTexture& lhs, const GLTexture& rhs) {
        if (!lhs.is_valid()) return rhs;
        if (!rhs.is_valid()) return lhs;
        
        GLTexture ret;
        glGenTextures(1, &ret.id);
        ret.width = std::max(lhs.width, rhs.width);
        ret.height = lhs.height + rhs.height;

        GLubyte* pixels = new GLubyte[ret.width*ret.height*4];
        GLubyte* lpixels = new GLubyte[lhs.width*lhs.height*4];
        GLubyte* rpixels = new GLubyte[rhs.width*rhs.height*4];

        glBindTexture(GL_TEXTURE_2D, lhs.id);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, lpixels);
        glBindTexture(GL_TEXTURE_2D, rhs.id);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, rpixels);

        memset(pixels, 0, ret.width*ret.height*4);
        for (int r = 0; r < ret.height; r++) {
            if (r < lhs.height) {
                memcpy(&pixels[r*ret.width*4], &lpixels[r*lhs.width*4], lhs.width*4);
            } else {
                memcpy(&pixels[r*ret.width*4], &rpixels[(r-lhs.height)*rhs.width*4],
                       rhs.width*4);
            }
        }

        glBindTexture(GL_TEXTURE_2D, ret.id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ret.width, ret.height, 0,
                     GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);

        delete[] pixels;
        delete[] lpixels;
        delete[] rpixels;

        return ret;
    }
    RawTexture GLTexture::get_pixel_data() const {
        RawTexture ret = {.width = width, .height = height, .format = GL_RGBA};
        ret.data = new GLubyte[width*height*4];

        glBindTexture(GL_TEXTURE_2D, id);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, ret.data);
        glBindTexture(GL_TEXTURE_2D, 0);
        return ret;
    }
    void GLTexture::save_image(const utils::Path& path, bool overwrite) const {
        ScopeLog _(utils::format("Saving texture with id {} to file \"{}\"...",
                                 id, path),
                   "File saved");
        RawTexture raw = get_pixel_data();
        #ifdef NTA_USE_DEVIL
            ILuint imgID = 0;
            ilGenImages(1, &imgID);
            ilBindImage(imgID);
            ilTexImage(width, height, 0, 4, IL_RGBA, IL_UNSIGNED_BYTE, raw.data);
            iluFlipImage();
            if (overwrite) {
                ilEnable(IL_FILE_OVERWRITE);
            } else {
                ilDisable(IL_FILE_OVERWRITE);
            }
            ilSaveImage(path.to_cstr());
        #else
            Logger::writeErrorToLog(NOT_YET_IMPLEMENTED,
                                    "jubilant-funicular cannot save images using CImg");
        #endif

        delete[] raw.data;
    }

    Result<RawTexture> ImageLoader::readImage(crstring filePath, crvec2 dimensions) {
        Logger::writeToLog("Loading image \"" + filePath + "\" into RawTexture...");
        Logger::indent();
        #ifdef NTA_USE_DEVIL
            ILuint imgID = 0;
            ilGenImages(1, &imgID);
            ilBindImage(imgID);
            if (ilLoadImage(filePath.c_str()) == IL_FALSE) {
                ILenum error = ilGetError();
                // This is some jank indenting
                auto err = Logger::writeErrorToLog(
                            "DevIL failed to load image with error " +
                                utils::to_string(error) + ": " + iluErrorString(error),
                            DEVIL_FAILURE);
                return Result<RawTexture>::new_err(err);
            }
            ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
        #else
            cimg_library::CImg<GLubyte> image(filePath.c_str());
        #endif

        RawTexture ret;
        if (dimensions != glm::vec2(0)) {
            #ifdef NTA_USE_DEVIL
                iluScale(dimensions.x, dimensions.y, 1);
            #else
                image = image.resize(dimensions.x, dimensions.y);
            #endif
        }
        #ifdef NTA_USE_DEVIL
            ret.width = ilGetInteger(IL_IMAGE_WIDTH);
            ret.height = ilGetInteger(IL_IMAGE_HEIGHT);
            ret.format = GL_RGBA;

            ret.data = new GLubyte[ret.width*ret.height*4];
            memcpy(ret.data, ilGetData(), ret.width*ret.height*4);
        #else
            ret.width = image.width();
            ret.height = image.height();
            // I'd prefer it if I could convert any image to RGBA
            ret.format = image.spectrum() == 3 ? GL_RGB : GL_RGBA;

            // CImg stores images in a stupid way; this fixes that
            image.permute_axes("cxyz");
            ret.data = new GLubyte[ret.width*ret.height*image.spectrum()];
            memcpy(ret.data, image.data(), ret.width*ret.height*image.spectrum());
        #endif
        Logger::unindent();
        Logger::writeToLog("Loaded image");
        return Result<RawTexture>::new_ok(ret);
    }
    Result<GLTexture> ImageLoader::readImage(crstring filePath, GLint minFilt,
                                             GLint magFilt, crvec2 dimensions) {
        return readImage(filePath, dimensions).map<GLTexture>([&](const RawTexture& raw) {
            return GLTexture(raw, minFilt, magFilt);
        });
    }
}
