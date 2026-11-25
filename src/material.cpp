

#include "material.hpp"


PBRMaterial::PBRMaterial()
    : folderPath("")
    , albedo(0)
    , normal(0)
    , roughness(0)
    , ao(0)
{
}

PBRMaterial::PBRMaterial(const std::string& folderPath)
    : folderPath("")
    , albedo(0)
    , normal(0)
    , roughness(0)
    , ao(0)
{
    loadFromFolder(folderPath);
}

PBRMaterial::~PBRMaterial() {
    cleanup();
}

PBRMaterial::PBRMaterial(PBRMaterial&& other) noexcept
    : folderPath(std::move(other.folderPath))
    , albedo(other.albedo)
    , normal(other.normal)
    , roughness(other.roughness)
    , ao(other.ao)
{
    other.albedo = 0;
    other.normal = 0;
    other.roughness = 0;
    other.ao = 0;
}

int PBRMaterial::loadTexture(const std::string& path) {
	int w, h, channels;
	unsigned int id;
	float maxAniso = 0.0f;

	unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);   // force RGBA

	if (!data){
		std::cout << "texture load failed: " << path << "\n";
		return -1;
	}

	glGenTextures(1, &id); 
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	// Mipmaps
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //trilinear filtering

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_REPEAT);

	// Anisotropic filtering
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAniso * 0.5f);
	
	stbi_image_free(data);
	return id;
}

PBRMaterial& PBRMaterial::operator=(PBRMaterial&& other) noexcept {
    if (this != &other) {
        cleanup();
        folderPath = std::move(other.folderPath);
        albedo = other.albedo;
        normal = other.normal;
        roughness = other.roughness;
        ao = other.ao;

        other.albedo = 0;
        other.normal = 0;
        other.roughness = 0;
        other.ao = 0;
    }
    return *this;
}

void PBRMaterial::cleanup() {
    if (albedo != 0) { glDeleteTextures(1, &albedo); albedo = 0; }
    if (normal != 0) { glDeleteTextures(1, &normal); normal = 0; }
    if (roughness != 0) { glDeleteTextures(1, &roughness); roughness = 0; }
    if (ao != 0) { glDeleteTextures(1, &ao); ao = 0; }
}

std::string PBRMaterial::resolveMapPath(const std::string& folder, const std::string& mapType) const {
    std::vector<std::string> patterns;
    if (mapType == "albedo") {
        patterns = {"albedo.png", "Color.png", "color.png", "Albedo.png", "diffuse.png"};
    } else if (mapType == "normal") {
        patterns = {"normal.png", "NormalGL.png", "Normal.png", "normalGL.png"};
    } else if (mapType == "roughness") {
        patterns = {"roughness.png", "Roughness.png", "rough.png"};
    } else if (mapType == "ao") {
        patterns = {"ao.png", "AmbientOcclusion.png", "AO.png", "occlusion.png"};
    }

    DIR* dir = opendir(folder.c_str());
    if (!dir) {
        std::cerr << "PBRMaterial: Cannot open folder: " << folder << "\n";
        return "";
    }

    struct dirent* entry;
    std::string matchedPath = "";

    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        for (const auto& pattern : patterns) {
            // Check if filename ends with/matches pattern
            if (filename.length() >= pattern.length()) {
                if (filename.compare(filename.length() - pattern.length(), pattern.length(), pattern) == 0) {
                    matchedPath = folder + "/" + filename;
                    break;
                }
            }
        }
        if (!matchedPath.empty()) break;
    }
    closedir(dir);
    if (matchedPath.empty()) {
        std::cerr << "cannot find required map of type *"<< patterns[0] <<"\n";
    }
    return matchedPath;
}

bool PBRMaterial::loadFromFolder(const std::string& path) {

    std::string albedoPath = resolveMapPath(path, "albedo");
    std::string normalPath = resolveMapPath(path, "normal");
    std::string roughnessPath = resolveMapPath(path, "roughness");
    std::string AOPath = resolveMapPath(path, "ao");

    if(albedoPath.empty() || normalPath.empty() || roughnessPath.empty() || AOPath.empty()) {
        return false;
    }    

    GLuint albedo = loadTexture(albedoPath);
    GLuint normal = loadTexture(normalPath);
    GLuint roughness = loadTexture(roughnessPath);
    GLuint ao = loadTexture(AOPath);
    if(albedo == -1 || normal == -1 || roughness == -1 || ao == -1){
        return false;
    }

    cleanup();
    this->albedo = albedo;
    this-> normal = normal;
    this-> roughness = roughness;
    this-> ao = ao;
    this-> folderPath = path;


    return true;
}

bool PBRMaterial::reload() {
    if (folderPath.empty()) {
        std::cerr << "PBRMaterial: No folder loaded to reload.\n";
        return false;
    }
    std::cout << "PBRMaterial: Reloading from " << folderPath << "...\n";
    return loadFromFolder(folderPath);
}

void PBRMaterial::bind() const {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedo);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, roughness);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, ao);
}
