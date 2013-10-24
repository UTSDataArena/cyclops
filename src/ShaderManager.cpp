/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory,  
 * University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials provided 
 * with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 * A manager that stores all programs and shaders that make p an independent 
 * shading environment. Multiple shading environments can exist at the same time
 * in a cyclops scene (for instance for sub-scenes with different light
 * settings)
 ******************************************************************************/
#include "cyclops/ShaderManager.h"

// We need to include this instead of fstream or we get duplicate symbols on
// linking.
#include <osgDB/ReadFile>

using namespace omega;
using namespace cyclops;

///////////////////////////////////////////////////////////////////////////////
ShaderManager::ShaderManager()
{
	// Standard shaders
#ifdef APPLE
	setShaderMacroToFile("surfaceShader", "cyclops/common/forward/default_osx.frag");
#else 
	setShaderMacroToFile("surfaceShader", "cyclops/common/forward/default.frag");
#endif
	setShaderMacroToFile("vertexShader", "cyclops/common/forward/default.vert");

	// Standard shaders
	setShaderMacroToFile("tangentSpaceSurfaceShader", "cyclops/common/forward/tangentSpace.frag");
	setShaderMacroToFile("tangentSpaceVertexShader", "cyclops/common/forward/tangentSpace.vert");
	//setShaderMacroToFile("tangentSpaceFragmentLightSection", "cyclops/common/forward/tangentSpaceLight.frag");

	setShaderMacroToFile("vsinclude envMap", "cyclops/common/envMap/noEnvMap.vert");
	setShaderMacroToFile("fsinclude envMap", "cyclops/common/envMap/noEnvMap.frag");

	setShaderMacroToFile("fsinclude lightFunctions", "cyclops/common/forward/lightFunctions.frag");

	setShaderMacroToString("customFragmentDefs", "");
	setShaderMacroToFile("postLightingSection", "cyclops/common/postLighting/default.frag");

	setShaderMacroToString("unlit", 
		"$@fragmentLightSection\n"
		"{\n" 
		"litSurfData.luminance = surf.albedo;\n"
		"}\n" 	
		"$\n");
}

///////////////////////////////////////////////////////////////////////////////
ShaderManager::~ShaderManager()
{
	myPrograms.clear();
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::addLightInstance(LightInstance* li)
{
	oassert(li);
	myActiveLights.push_back(li);
	recompileShaders();
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::removeLightInstance(LightInstance* li)
{
	myActiveLights.remove(li);
	recompileShaders();
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::setShaderMacroToString(const String& macroName, const String& macroString)
{
	myShaderMacros[macroName] = macroString;
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::setShaderMacroToFile(const String& macroName, const String& name)
{
	String path;
	if(DataManager::findFile(name, path))
	{
		std::ifstream t(path.c_str());
		std::stringstream buffer;
		buffer << t.rdbuf();
		setShaderMacroToString(macroName, buffer.str());
	}
	else
	{
		ofwarn("SceneManager::setShaderMacroToFile: could not find file %1%", %name);
	}
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::update()
{
	int i = 0;
	bool needShaderUpdate = false;
	foreach(LightInstance* l, myActiveLights)
	{
		if(l->getLight()->isEnabled())
		{
			l->setLightIndex(i++);
		}
		needShaderUpdate |= l->update();
	}

	// If the number of lights changed, reset the shaders
	if(myActiveLights.size() != myNumActiveLights || needShaderUpdate)
	{
		ofmsg("Lights changed (active lights: %1%)", %i);

		// Set the number of lights shader macro parameter.
		myNumActiveLights = myActiveLights.size();

		String numLightsString = ostr("%1%", %myNumActiveLights);
		setShaderMacroToString("numLights", numLightsString);
		recompileShaders();
	}
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::loadShader(osg::Shader* shader, const String& name)
{
	// If shader source is not in the cache, load it now.
	if(myShaderCache.find(name) == myShaderCache.end())
	{
		String path;
		if(DataManager::findFile(name, path))
		{
			ofmsg("Loading shader: %1%", %name);

			std::ifstream t(path.c_str());
			std::stringstream buffer;
			buffer << t.rdbuf();
			String shaderSrc = buffer.str();

			myShaderCache[name] = shaderSrc;
		}
		else
		{
			ofwarn("Could not find shader file %1%", %name);
		}
	}

	if(myShaderCache.find(name) != myShaderCache.end())
	{
		compileShader(shader, myShaderCache[name]);
	}
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::compileShader(osg::Shader* shader, const String& source)
{
	String shaderPreSrc = source;
	String lightSectionMacroName = "fragmentLightSection";

	// Replace shader macros.
	// Do a multiple replacement passes to process macros-within macros.
	int replacementPasses = 3;
	for(int i = 0; i < replacementPasses; i++)
	{
		foreach(ShaderMacroDictionary::Item macro, myShaderMacros)
		{
			if(macro.getKey() != lightSectionMacroName)
			{
				String macroName = ostr("@%1%", %macro.getKey());
				shaderPreSrc = StringUtils::replaceAll(shaderPreSrc, macroName, macro.getValue());
			}
		}
	}

	// Read local macro definitions (only supported one now is fragmentLightSection)
	String shaderSrc = "";
	Vector<String> segments = StringUtils::split(shaderPreSrc, "$");
	//ofmsg("segments %1%", %segments.size());
	foreach(String segment, segments)
	{
		if(StringUtils::startsWith(segment, "@"))
		{
			Vector<String> macroNames = StringUtils::split(segment, "\r\n\t ", 1);
				
			String macroContent = StringUtils::replaceAll(segment, macroNames[0], "");
				
			String macroName = macroNames[0].substr(1);
			//ofmsg("SEGMENT IDENTIFIED: %1%", %macroName);
			myShaderMacros[macroName] = macroContent;
		}
		else
		{
			shaderSrc.append(segment);
		}
	}

	// Special section: replicate lighting code as many times as the active lights
	String fragmentShaderLightCode = myShaderMacros[lightSectionMacroName];
	String fragmentShaderLightSection = "";
	foreach(LightInstance* li, myActiveLights)
	{
		Light* light = li->getLight();

		// Add the light index to the section
		String fragmentShaderLightCodeIndexed = StringUtils::replaceAll(
			fragmentShaderLightCode, 
			"@lightIndex", 
			boost::lexical_cast<String>(li->getLightIndex()));

		// Replace light function call with light function name specified for light.
		fragmentShaderLightCodeIndexed = StringUtils::replaceAll(fragmentShaderLightCodeIndexed,
			"@lightFunction", light->getLightFunction());

		fragmentShaderLightSection += fragmentShaderLightCodeIndexed;
	}
	shaderSrc = StringUtils::replaceAll(shaderSrc, 
		"@" + lightSectionMacroName, 
		fragmentShaderLightSection);

	//ofmsg("Loading shader file %1%", %name);
	// omsg("#############################################################");
	// omsg(shaderSrc);
	// omsg("#############################################################");
	shader->setShaderSource(shaderSrc);
}

///////////////////////////////////////////////////////////////////////////////
ProgramAsset* ShaderManager::getOrCreateProgram(const String& name, const String& vertexShaderName, const String& fragmentShaderName)
{
	// If program has been loaded already return it.
	if(myPrograms.find(name) != myPrograms.end())
	{
		return myPrograms[name];
	}

	ProgramAsset* asset = new ProgramAsset();
	asset->program = new osg::Program();
	asset->name = name;
	asset->program->setName(name);
	asset->fragmentShaderName = fragmentShaderName;
	asset->vertexShaderName = vertexShaderName;

	myPrograms[name] = asset;

	recompileShaders(asset, myShaderVariationName);

	return asset;
}

///////////////////////////////////////////////////////////////////////////////
ProgramAsset* ShaderManager::createProgramFromString(const String& name, const String& vertexShaderCode, const String& fragmentShaderCode)
{
	ProgramAsset* asset = new ProgramAsset();
	asset->program = new osg::Program();
	asset->name = name;
	asset->program->setName(name);
	asset->fragmentShaderSource = fragmentShaderCode;
	asset->fragmentShaderName = name + "Fragment";
	asset->vertexShaderSource = vertexShaderCode;
	asset->vertexShaderName = name + "Vertex";
	asset->embedded = true;

	myPrograms[name] = asset;

	recompileShaders(asset, myShaderVariationName);

	return asset;
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::addProgram(ProgramAsset* program)
{
	myPrograms[program->name] = program;
	program->program = new osg::Program();
	program->program->setName(program->name);
	recompileShaders(program, myShaderVariationName);
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::updateProgram(ProgramAsset* program)
{
	// Delete current shaders to force reload.
	String fullVertexShaderName = program->vertexShaderName + myShaderVariationName;
	String fullFragmentShaderName = program->fragmentShaderName + myShaderVariationName;

	// Delete binaries...
	myShaders[fullVertexShaderName] = NULL;
	myShaders[fullFragmentShaderName] = NULL;

	// Delete source cache...
	myShaderCache.erase(program->vertexShaderName);
	myShaderCache.erase(program->fragmentShaderName);

	recompileShaders(program, myShaderVariationName);
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::recompileShaders(ProgramAsset* program, const String& svariationName)
{
	String var = myShaderVariationName;
	if(svariationName != "") var = svariationName;

	osg::Program* osgProg = program->program;

	// Remove current shaders from program
	osgProg->removeShader(program->vertexShaderBinary);
	osgProg->removeShader(program->fragmentShaderBinary);
	osgProg->removeShader(program->geometryShaderBinary);
	//osgProg->releaseGLObjects();

	String fullVertexShaderName = program->vertexShaderName + var;
	osg::Shader* vertexShader = myShaders[fullVertexShaderName];
	// If the shader does not exist in the shader registry, we need to create it now.
	if(vertexShader == NULL)
	{
		ofmsg("Creating vertex shader %1%", %fullVertexShaderName);

		vertexShader = new osg::Shader( osg::Shader::VERTEX );
		// increase reference count to avoid being deallocated by osg program when deattached.
		vertexShader->ref();
		
		// If the program asset has embedded code, use the code from the asset instead of looking up a file.
		if(program->embedded)
		{
			compileShader(vertexShader, program->vertexShaderSource);
		}
		else
		{
			loadShader(vertexShader, program->vertexShaderName);
		}
		myShaders[fullVertexShaderName] = vertexShader;
	}
	program->vertexShaderBinary = vertexShader;
	osgProg->addShader(vertexShader);

	String fullFragmentShaderName = program->fragmentShaderName + var;
	osg::Shader* fragmentShader = myShaders[fullFragmentShaderName];
	// If the shader does not exist in the shader registry, we need to create it now.
	if(fragmentShader == NULL)
	{
		ofmsg("Creating fragment shader %1%", %fullFragmentShaderName);

		fragmentShader = new osg::Shader( osg::Shader::FRAGMENT );
		// increase reference count to avoid being deallocated by osg program when deattached.
		fragmentShader->ref();
		// If the program asset has embedded code, use the code from the asset instead of looking up a file.
		if(program->embedded)
		{
			compileShader(fragmentShader, program->fragmentShaderSource);
		}
		else
		{
			loadShader(fragmentShader, program->fragmentShaderName);
		}
		myShaders[fullFragmentShaderName] = fragmentShader;
	}
	program->fragmentShaderBinary = fragmentShader;
	osgProg->addShader(fragmentShader);

	// OPTIONAL geometry shader
	if(program->geometryShaderName != "")
	{
		String fullGeometryShaderName = program->geometryShaderName + var;
		osg::Shader* geometryShader = myShaders[fullGeometryShaderName];
		// If the shader does not exist in the shader registry, we need to create it now.
		if(geometryShader == NULL)
		{
			ofmsg("Creating geometry shader %1%", %fullGeometryShaderName);

			geometryShader = new osg::Shader( osg::Shader::GEOMETRY );
			// increase reference count to avoid being deallocated by osg program when deattached.
			geometryShader->ref();
			// If the program asset has embedded code, use the code from the asset instead of looking up a file.
			if(program->embedded)
			{
				compileShader(geometryShader, program->geometryShaderSource);
			}
			else
			{
				loadShader(geometryShader, program->geometryShaderName);
			}
			myShaders[fullGeometryShaderName] = geometryShader;
		}
		program->geometryShaderBinary = geometryShader;
		osgProg->addShader(geometryShader);
		// Set geometry shader parameters.
		osgProg->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, program->geometryOutVertices );
		osgProg->setParameter( GL_GEOMETRY_INPUT_TYPE_EXT, program->geometryInput );
		osgProg->setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT, program->geometryOutput );
	}
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::setActiveCacheId(const String& cacheId)
{
	myActiveCacheId = cacheId;
	recompileShaders();
}

///////////////////////////////////////////////////////////////////////////////
const String& ShaderManager::getActiveCacheId()
{
	return myActiveCacheId;
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::recompileShaders()
{
	// Add light functions to shader variation name
	String lightFunc = "";
	foreach(LightInstance* li, myActiveLights)
	{
		lightFunc.append(li->getLight()->getLightFunction());
	}
#ifdef OMEGA_OS_WIN
	std::hash<String> hashFx;
#else
	std::tr1::hash<String> hashFx;
#endif
	size_t lightFuncHash = hashFx(lightFunc);

	// Update the shader variation name
	myShaderVariationName = ostr(".%1%%2%-%3$x", %myActiveCacheId %myNumActiveLights %lightFuncHash);

	ofmsg("Recompiling shaders (variation: %1%)", %myShaderVariationName);

	typedef Dictionary<String, Ref<ProgramAsset> >::Item ProgramAssetItem;
	foreach(ProgramAssetItem item, myPrograms)
	{
		recompileShaders(item.getValue(), myShaderVariationName);
	}
}

///////////////////////////////////////////////////////////////////////////////
void ShaderManager::reloadAndRecompileShaders()
{
	myShaderCache.clear();
	myShaders.clear();
	recompileShaders();
}
