#ifndef MATERIAL_HPP
#define MATERIAL_HPP

#include <string>
#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>

#include "stb_image.h"



class PBRMaterial {
public:
    PBRMaterial();
    explicit PBRMaterial(const std::string& folderPath);
    ~PBRMaterial();

    int loadTexture(const std::string& path);

    // Delete copy operations (textures own OpenGL GPU handles)
    PBRMaterial(const PBRMaterial&) = delete;
    PBRMaterial& operator=(const PBRMaterial&) = delete;

    // Move operations
    PBRMaterial(PBRMaterial&& other) noexcept;
    PBRMaterial& operator=(PBRMaterial&& other) noexcept;

    // Core functionality
    bool loadFromFolder(const std::string& folderPath);
    bool reload();
    void bind() const;

    bool isValid() const {
        return albedo != 0 && normal != 0 && roughness != 0 && ao != 0;
    }

    const std::string& getFolderPath() const { return folderPath; }
    GLuint getAlbedo() const { return albedo; }
    GLuint getNormal() const { return normal; }
    GLuint getRoughness() const { return roughness; }
    GLuint getAO() const { return ao; }

private:
    std::string folderPath;

    GLuint albedo;
    GLuint normal;
    GLuint roughness;
    GLuint ao;

    void cleanup();
    std::string resolveMapPath(const std::string& folder, const std::string& mapType) const;
    void errPath(const std::string& path);
    void errTexture(const std::string path);
};

#endif
