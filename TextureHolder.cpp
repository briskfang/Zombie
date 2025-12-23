#include <assert.h>
#include "TextureHolder.h"

using namespace sf;
using namespace std;

TextureHolder* TextureHolder::m_s_Instance = nullptr;

TextureHolder::TextureHolder()
{
    assert(m_s_Instance == nullptr );
    m_s_Instance = this;
}

sf::Texture& TextureHolder::getTexture(std::string const& filename)
{
    auto& m = m_s_Instance->m_Textures;  //map<std::string, Texture>
    auto keyValuePair = m.find(filename);

    if(keyValuePair != m.end()) //found
    {
        return keyValuePair->second;
    }
    else
    {
        auto& texture = m[filename];
        texture.loadFromFile(filename);
        return texture;
    }

}



