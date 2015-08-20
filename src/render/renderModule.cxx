#include "renderModule.h"

#include "decal.h"
#include "shaderTag.h"
#include "texture.h"
#include "uniform.h"
#include "vertexBuffer.h"

using namespace render;

/**
 * constructor
 *
 * @param currentDir
 */
RenderModule::RenderModule(const std::string & currentDir) :
		members(std::unordered_map<std::string, ScriptObjectPtr>{
				{ "Decal", Decal::getFactory(currentDir) },
				{ "Shader", ShaderTag::getFactory(currentDir) },
				{ "Texture", Texture::getFactory(currentDir) },
				{ "Uniform", Uniform::getFactory() },
				{ "VertexBuffer", VertexBuffer::getFactory(currentDir) } }) {
}

/**
 * destructor
 */
RenderModule::~RenderModule(){
}

/**
 * get named script object member
 *
 * @param execState  current script execution state
 * @param name       name of member
 *
 * @return           script object represented by name
 */
ScriptObjectPtr RenderModule::getMember(ScriptExecutionState &,
		const std::string & name) const {
	auto entry = members.find(name);
	if (entry != members.end()) {
		return entry->second;
	}
	return nullptr;
}
