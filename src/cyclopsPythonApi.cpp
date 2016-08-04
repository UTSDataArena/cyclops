/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2015		Electronic Visualization Laboratory,
 *							University of Illinois at Chicago
 * Authors:
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2015, Electronic Visualization Laboratory,
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
 * What's in this file:
 *	The cyclops python API declarations and wrappers
 ******************************************************************************/
#include <cyclops/cyclops.h>

#include "omega/PythonInterpreter.h"
#include "omega/PythonInterpreterWrapper.h"
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>




// #include <osg/ref_ptr>
#include <osgGA/GUIEventAdapter>

using namespace cyclops;

///////////////////////////////////////////////////////////////////////////////
SceneManager* getSceneManager() { return SceneManager::instance(); }



////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Material_setTransparent, setTransparent, 1, 2)
BOOST_PYTHON_MODULE(cyclops)
{
    // SceneLoader
    class_<SceneLoader, boost::noncopyable >("SceneLoader", no_init)
        PYAPI_STATIC_REF_GETTER(SceneLoader, getLastLoadedEntity)
        ;

    // PrimitiveType
    PYAPI_ENUM(ProgramAsset::PrimitiveType, PrimitiveType)
        PYAPI_ENUM_VALUE(ProgramAsset, Points)
        PYAPI_ENUM_VALUE(ProgramAsset, Triangles)
        PYAPI_ENUM_VALUE(ProgramAsset, TriangleStrip);

    // ProgramAsset
    PYAPI_REF_BASE_CLASS_WITH_CTOR(ProgramAsset)
        .def_readwrite("name", &ProgramAsset::name)
        .def_readwrite("vertexShaderName", &ProgramAsset::vertexShaderName)
        .def_readwrite("fragmentShaderName", &ProgramAsset::fragmentShaderName)
        .def_readwrite("geometryShaderName", &ProgramAsset::geometryShaderName)
        .def_readwrite("vertexShaderSource", &ProgramAsset::vertexShaderSource)
        .def_readwrite("fragmentShaderSource", &ProgramAsset::fragmentShaderSource)
        .def_readwrite("geometryShaderSource", &ProgramAsset::geometryShaderSource)
        .def_readwrite("geometryOutVertices", &ProgramAsset::geometryOutVertices)
        .def_readwrite("geometryInput", &ProgramAsset::geometryInput)
        .def_readwrite("geometryOutput", &ProgramAsset::geometryOutput)
        .def_readwrite("embedded", &ProgramAsset::embedded);

    // ModelGeometry
    PYAPI_REF_BASE_CLASS(ModelGeometry)
        PYAPI_STATIC_REF_GETTER(ModelGeometry, create)
        PYAPI_METHOD(ModelGeometry, addVertex)
        PYAPI_METHOD(ModelGeometry, setVertex)
        PYAPI_GETTER(ModelGeometry, getVertex)
        PYAPI_METHOD(ModelGeometry, addColor)
        PYAPI_METHOD(ModelGeometry, setColor)
        PYAPI_GETTER(ModelGeometry, getColor)
        PYAPI_METHOD(ModelGeometry, clear)
        PYAPI_METHOD(ModelGeometry, addPrimitive)
        PYAPI_GETTER(ModelGeometry, getName)
        ;

    PYAPI_REF_BASE_CLASS(ModelLoader);

    PYAPI_REF_BASE_CLASS(ShaderManager)
        PYAPI_METHOD(SceneManager, setShaderMacroToFile)
        PYAPI_METHOD(SceneManager, setShaderMacroToString)
        PYAPI_METHOD(SceneManager, addProgram)
        PYAPI_METHOD(SceneManager, updateProgram)
        PYAPI_REF_GETTER(SceneManager, createProgramFromString)
        PYAPI_METHOD(SceneManager, reloadAndRecompileShaders)
        ;

    // SceneLayer
    PYAPI_REF_BASE_CLASS_WITH_CTOR(SceneLayer)
        .def_readonly("CameraDrawExplicitLayers", &SceneLayer::CameraDrawExplicitLayers)
        .def_readonly("Layer1", &SceneLayer::Layer1)
        .def_readonly("Layer2", &SceneLayer::Layer2)
        .def_readonly("Layer3", &SceneLayer::Layer3)
        .def_readonly("Layer4", &SceneLayer::Layer4)
        .def_readonly("Layer5", &SceneLayer::Layer5)
        .def_readonly("Layer6", &SceneLayer::Layer6)
        .def_readonly("Layer7", &SceneLayer::Layer7)
        .def_readonly("Layer8", &SceneLayer::Layer8)
        PYAPI_METHOD(SceneLayer, addLayer)
        PYAPI_METHOD(SceneLayer, removeLayer)
        PYAPI_METHOD(SceneLayer, setCamera)
        PYAPI_METHOD(SceneLayer, getId)
        PYAPI_METHOD(SceneLayer, setId)
        PYAPI_REF_GETTER(SceneLayer, getCamera)
        ;

    // LightingLayer
    PYAPI_REF_CLASS_WITH_CTOR(LightingLayer, SceneLayer)
        ;

    // CompositingLayer
    PYAPI_REF_CLASS_WITH_CTOR(CompositingLayer, SceneLayer)
        PYAPI_METHOD(CompositingLayer, reset)
        PYAPI_METHOD(CompositingLayer, loadCompositor)
        PYAPI_METHOD(CompositingLayer, isPassActive)
        PYAPI_METHOD(CompositingLayer, setPassActive)
        PYAPI_REF_GETTER(CompositingLayer, getUniform)
        ;

    // SceneManager
    void (SceneManager::*loadModelAsync1)(ModelInfo*, const String&) = &SceneManager::loadModelAsync;
    PYAPI_REF_CLASS(SceneManager, ShaderManager)
        PYAPI_REF_GETTER(SceneManager, getLightingLayer)
        PYAPI_REF_GETTER(SceneManager, getCompositingLayer)
        PYAPI_METHOD(SceneManager, addModel)
        PYAPI_METHOD(SceneManager, addModelAsset)
        PYAPI_METHOD(SceneManager, loadModel)
        .def("loadModelAsync", loadModelAsync1)
        PYAPI_METHOD(SceneManager, setBackgroundColor)
        PYAPI_METHOD(SceneManager, loadScene)
        PYAPI_METHOD(SceneManager, addLoader)
        PYAPI_METHOD(SceneManager, unload)
        PYAPI_METHOD(SceneManager, setSkyBox)
        PYAPI_METHOD(SceneManager, displayWand)
        PYAPI_METHOD(SceneManager, hideWand)
        PYAPI_METHOD(SceneManager, setWandEffect)
        PYAPI_METHOD(SceneManager, setWandSize)
        PYAPI_REF_GETTER(SceneManager, getGlobalUniforms)
        PYAPI_REF_GETTER(SceneManager, createTexture)
        // Physics
        PYAPI_GETTER(SceneManager, getGravity)
        PYAPI_METHOD(SceneManager, setGravity)
        PYAPI_METHOD(SceneManager, setPhysicsEnabled)
        PYAPI_METHOD(SceneManager, isPhysicsEnabled)
        PYAPI_METHOD(SceneManager, setColDetectionEnabled)
        PYAPI_METHOD(SceneManager, isColDetectionEnabled)
        ;

    // osg Texture2D
    class_<osg::Texture2D, boost::noncopyable, Ref<osg::Texture2D> >("Texture2D", no_init)
        ;

    // UniformType
    PYAPI_ENUM(Uniform::Type, UniformType)
        PYAPI_ENUM_VALUE(Uniform, Vector2f)
        PYAPI_ENUM_VALUE(Uniform, Vector3f)
        PYAPI_ENUM_VALUE(Uniform, Color)
        PYAPI_ENUM_VALUE(Uniform, Float)
        PYAPI_ENUM_VALUE(Uniform, Int);

    // Uniform
    PYAPI_REF_BASE_CLASS(Uniform)
        PYAPI_STATIC_REF_GETTER(Uniform, create)
        PYAPI_GETTER(Uniform, getType)
        PYAPI_METHOD(Uniform, setInt)
        PYAPI_METHOD(Uniform, getInt)
        PYAPI_METHOD(Uniform, setIntElement)
        PYAPI_METHOD(Uniform, getIntElement)
        PYAPI_METHOD(Uniform, setFloat)
        PYAPI_METHOD(Uniform, getFloat)
        PYAPI_METHOD(Uniform, setFloatElement)
        PYAPI_METHOD(Uniform, getFloatElement)
        PYAPI_METHOD(Uniform, setVector2f)
        PYAPI_GETTER(Uniform, getVector2f)
        PYAPI_METHOD(Uniform, setVector2fElement)
        PYAPI_GETTER(Uniform, getVector2fElement)
        PYAPI_METHOD(Uniform, setVector3f)
        PYAPI_GETTER(Uniform, getVector3f)
        PYAPI_METHOD(Uniform, setVector3fElement)
        PYAPI_GETTER(Uniform, getVector3fElement)
        PYAPI_METHOD(Uniform, setColor)
        PYAPI_GETTER(Uniform, getColor)
        PYAPI_METHOD(Uniform, setColorElement)
        PYAPI_GETTER(Uniform, getColorElement);

    // Uniforms
    PYAPI_REF_BASE_CLASS(Uniforms)
        PYAPI_REF_GETTER(Uniforms, attachUniform)
        PYAPI_REF_GETTER(Uniforms, addUniform)
        PYAPI_REF_GETTER(Uniforms, addUniformArray)
        PYAPI_REF_GETTER(Uniforms, getUniform)
        PYAPI_METHOD(Uniforms, removeAllUniforms);

    // Material
    PYAPI_REF_CLASS(Material, Uniforms)
        .def_readonly("CameraDrawExplicitMaterials", &Material::CameraDrawExplicitMaterials)
        PYAPI_STATIC_REF_GETTER(Material, create)
        PYAPI_METHOD(Material, setColor)
        .def("setTransparent", &Material::setTransparent, Material_setTransparent())
        PYAPI_METHOD(Material, isTransparent)
        PYAPI_METHOD(Material, reset)
        PYAPI_METHOD(Material, setProgram)
        PYAPI_METHOD(Material, parse)
        PYAPI_METHOD(Material, setAdditive)
        PYAPI_METHOD(Material, isAdditive)
        PYAPI_METHOD(Material, setDepthTestEnabled)
        PYAPI_METHOD(Material, isDepthTestEnabled)
        PYAPI_METHOD(Material, setDoubleFace)
        PYAPI_METHOD(Material, isDoubleFace)
        PYAPI_METHOD(Material, setWireframe)
        PYAPI_METHOD(Material, isWireframe)
        PYAPI_METHOD(Material, setPolygonOffset)
        PYAPI_METHOD(Material, setDiffuseTexture)
        PYAPI_GETTER(Material, getDiffuseTexture)
        PYAPI_METHOD(Material, setNormalTexture)
        PYAPI_GETTER(Material, getNormalTexture)
        PYAPI_GETTER(Material, setTexture)
        PYAPI_METHOD(Material, getShininess)
        PYAPI_METHOD(Material, setShininess)
        PYAPI_METHOD(Material, getGloss)
        PYAPI_METHOD(Material, setGloss)
        PYAPI_METHOD(Material, isLit)
        PYAPI_METHOD(Material, setLit)
        PYAPI_METHOD(Material, setAlpha)
        PYAPI_METHOD(Material, getAlpha)
        PYAPI_METHOD(Material, setCamera)
        PYAPI_METHOD(Material, setPointSprite)
        PYAPI_METHOD(Material, isPointSprite)
        PYAPI_METHOD(Material, setPointSize)
        PYAPI_METHOD(Material, getPointSize)
        PYAPI_REF_GETTER(Material, getCamera)
        ;

    // RigidBodyType
    PYAPI_ENUM(RigidBody::RigidBodyType, RigidBodyType)
        PYAPI_ENUM_VALUE(RigidBody, Box)
        PYAPI_ENUM_VALUE(RigidBody, Sphere)
        PYAPI_ENUM_VALUE(RigidBody, Cylinder)
        PYAPI_ENUM_VALUE(RigidBody, Plane)
        ;

    // RigidBody
    PYAPI_REF_BASE_CLASS(RigidBody)
        PYAPI_METHOD(RigidBody, isEnabled)
        PYAPI_METHOD(RigidBody, setEnabled)
        PYAPI_METHOD(RigidBody, initialize)
        PYAPI_METHOD(RigidBody, sync)
        PYAPI_METHOD(RigidBody, isUserControlled)
        PYAPI_METHOD(RigidBody, setUserControlled)
        PYAPI_METHOD(RigidBody, applyForce)
        PYAPI_METHOD(RigidBody, applyCentralForce)
        PYAPI_METHOD(RigidBody, applyImpulse)
        PYAPI_METHOD(RigidBody, applyCentralImpulse)
        PYAPI_METHOD(RigidBody, setLinearVelocity)
        PYAPI_METHOD(RigidBody, setAngularVelocity)
        PYAPI_METHOD(RigidBody, setFriction)
        PYAPI_METHOD(RigidBody, getFriction)
        PYAPI_METHOD(RigidBody, setRollingFriction)
        PYAPI_METHOD(RigidBody, getRollingFriction)
        ;

    // Entity
    PYAPI_REF_CLASS(Entity, SceneNode)
        PYAPI_METHOD(Entity, setLayer)
        PYAPI_REF_GETTER(Entity, getLayer)
        PYAPI_METHOD(Entity, castShadow)
        PYAPI_METHOD(Entity, doesCastShadow)
        PYAPI_METHOD(Entity, hasEffect)
        PYAPI_METHOD(Entity, setEffect)
        PYAPI_REF_GETTER(Entity, getMaterial)
        PYAPI_REF_GETTER(Entity, getMaterialByIndex)
        PYAPI_METHOD(Entity, getMaterialCount)
        PYAPI_METHOD(Entity, addMaterial)
        PYAPI_METHOD(Entity, removeMaterial)
        PYAPI_METHOD(Entity, clearMaterials)
        PYAPI_GETTER(Entity, listPieces)
        PYAPI_REF_GETTER(Entity, getPiece)
        PYAPI_METHOD(Entity, setCullingActive)
        PYAPI_METHOD(Entity, isCullingActive)
        PYAPI_REF_GETTER(Entity, getRigidBody)
        PYAPI_METHOD(Entity, setPointIntersectionEnabled)
        PYAPI_METHOD(Entity, isPointIntersectionEnabled)
        ;

    // Movie
	// ffmpeg-based movie object in omegalib
	// [Darren 10Jun14]
    PYAPI_REF_CLASS(Movie, Entity)
        PYAPI_STATIC_REF_GETTER(Movie, create)
        PYAPI_METHOD(Movie, rewind)
        PYAPI_METHOD(Movie, play)
        PYAPI_METHOD(Movie, pause)
        PYAPI_METHOD(Movie, stop)
        PYAPI_METHOD(Movie, setLooping)
        PYAPI_METHOD(Movie, getCurrentTime)
        PYAPI_METHOD(Movie, getLength)
        PYAPI_METHOD(Movie, getMovieWidth)
        PYAPI_METHOD(Movie, getMovieHeight)
        PYAPI_METHOD(Movie, seek)
        PYAPI_METHOD(Movie, setMask)
        PYAPI_METHOD(Movie, setMaskFromFile)
        PYAPI_METHOD(Movie, setMaskEnabled)
        PYAPI_METHOD(Movie, isLooping)
        PYAPI_METHOD(Movie, isPlaying)
        PYAPI_METHOD(Movie, isMaskEnabled)
        PYAPI_METHOD(Movie, getAspectRatio)
        PYAPI_GETTER(Movie, getPath)
        PYAPI_GETTER(Movie, getStreamStatus)
        PYAPI_GETTER(Movie, attachUniforms)
        ;
        
    // Volume
    // [Darren 26Nov14]
    PYAPI_REF_CLASS(Volume, Entity)
        PYAPI_STATIC_REF_GETTER(Volume, create)
        ;

    //ManipulatorController
    // [Max 28Jun16]
    PYAPI_REF_BASE_CLASS(ManipulatorController)
        PYAPI_STATIC_REF_GETTER(ManipulatorController, create)
        PYAPI_METHOD(ManipulatorController, onEvent)
        PYAPI_METHOD(ManipulatorController, setManipulator)
        PYAPI_METHOD(ManipulatorController, setEventAdapter)
        ;

    PYAPI_BASE_CLASS(AbstractOmegaManipulator)
        PYAPI_METHOD(AbstractOmegaManipulator, setHomeEye)
        PYAPI_METHOD(AbstractOmegaManipulator, setHome)
        PYAPI_METHOD(AbstractOmegaManipulator, _home)
        PYAPI_GETTER(AbstractOmegaManipulator, getCameraCenter)
        ;

    PYAPI_REF_CLASS(NodeTrackerManipulator, AbstractOmegaManipulator)
        PYAPI_STATIC_REF_GETTER(NodeTrackerManipulator, create)
        PYAPI_METHOD(NodeTrackerManipulator, setTrackedNode)
        ;

    PYAPI_REF_CLASS(OrbitManipulator, AbstractOmegaManipulator)
        PYAPI_STATIC_REF_GETTER(OrbitManipulator, create)
        ;

    PYAPI_REF_CLASS(TerrainManipulator, AbstractOmegaManipulator)
        PYAPI_STATIC_REF_GETTER(TerrainManipulator, create)
        PYAPI_METHOD(TerrainManipulator, setTerrainNode)
        ;


    // PYAPI_REF_CLASS(FirstPersonManipulator, AbstractOmegaManipulator)
    //     PYAPI_STATIC_REF_GETTER(FirstPersonManipulator, create)
    //     ;

    // ImageStatus
    PYAPI_ENUM(osg::ImageStream::StreamStatus, StreamStatus)
        PYAPI_ENUM_VALUE(osg::ImageStream, INVALID)
        PYAPI_ENUM_VALUE(osg::ImageStream, PLAYING)
        PYAPI_ENUM_VALUE(osg::ImageStream, PAUSED)
        PYAPI_ENUM_VALUE(osg::ImageStream, REWINDING)
    ;
        
    // SphereShape
    PYAPI_REF_CLASS(SphereShape, Entity)
        PYAPI_STATIC_REF_GETTER(SphereShape, create)
        ;

    // PlaneShape
    PYAPI_REF_CLASS(PlaneShape, Entity)
        PYAPI_STATIC_REF_GETTER(PlaneShape, create)
        ;

    // BoxShape
    PYAPI_REF_CLASS(BoxShape, Entity)
        PYAPI_STATIC_REF_GETTER(BoxShape, create)
        ;

    // CylinderShape
    PYAPI_REF_CLASS(CylinderShape, Entity)
        PYAPI_STATIC_REF_GETTER(CylinderShape, create)
        ;

    // Line
    class_<LineSet::Line, boost::noncopyable, Ref<LineSet::Line> >("Line", no_init)
        PYAPI_METHOD(LineSet::Line, setStart)
        PYAPI_METHOD(LineSet::Line, setEnd)
        PYAPI_METHOD(LineSet::Line, setThickness)
        PYAPI_METHOD(LineSet::Line, getThickness)
        ;

    // LineSet
    PYAPI_REF_CLASS(LineSet, Entity)
        PYAPI_STATIC_REF_GETTER(LineSet, create)
        PYAPI_REF_GETTER(LineSet, addLine)
        PYAPI_METHOD(LineSet, removeLine)
        ;

    // Text
    PYAPI_REF_CLASS(Text3D, Entity)
        PYAPI_STATIC_REF_GETTER(Text3D, create)
        PYAPI_METHOD(Text3D, setText)
        PYAPI_METHOD(Text3D, setFont)
        PYAPI_METHOD(Text3D, setColor)
        PYAPI_METHOD(Text3D, setFontSize)
        PYAPI_METHOD(Text3D, setFixedSize)
        PYAPI_METHOD(Text3D, setFontResolution)
        ;

    // StaticObject
    PYAPI_REF_CLASS(StaticObject, Entity)
        PYAPI_STATIC_REF_GETTER(StaticObject, create)
        ;

    // AnimatedObject
    PYAPI_REF_CLASS(AnimatedObject, Entity)
        .def("create", &AnimatedObject::create, PYAPI_RETURN_REF).staticmethod("create")
        .def("hasAnimations", &AnimatedObject::hasAnimations)
        .def("getNumAnimations", &AnimatedObject::getNumAnimations)
        .def("playAnimation", &AnimatedObject::playAnimation)
        .def("loopAnimation", &AnimatedObject::loopAnimation)
        .def("pauseAnimation", &AnimatedObject::pauseAnimation)
        .def("stopAllAnimations", &AnimatedObject::stopAllAnimations)
        .def("getCurAnimation", &AnimatedObject::getCurAnimation)
        .def("getAnimationLength", &AnimatedObject::getAnimationLength)
        .def("getAnimationStart", &AnimatedObject::getAnimationStart)
        .def("setAnimationStart", &AnimatedObject::setAnimationStart)
        // These use the new python API macros. Convert all declaractions to
        // use this
        PYAPI_METHOD(AnimatedObject, setCurrentModelIndex)
        PYAPI_METHOD(AnimatedObject, getCurrentModelIndex)
        PYAPI_METHOD(AnimatedObject, getNumModels)
        PYAPI_METHOD(AnimatedObject, setOnAnimationEndedScript)
        PYAPI_METHOD(AnimatedObject, getOnAnimationEndedScript)
        ;

    // LightType
    PYAPI_ENUM(Light::LightType, LightType)
        PYAPI_ENUM_VALUE(Light, Point)
        PYAPI_ENUM_VALUE(Light, Directional)
        PYAPI_ENUM_VALUE(Light, Spot)
        PYAPI_ENUM_VALUE(Light, Custom);

    // ShadowUpdateMode
    PYAPI_ENUM(Light::ShadowRefreshMode, ShadowRefreshMode)
        PYAPI_ENUM_VALUE(Light, OnFrame)
        PYAPI_ENUM_VALUE(Light, OnLightMove)
        PYAPI_ENUM_VALUE(Light, Manual);

    // ShadowMap
    PYAPI_REF_BASE_CLASS_WITH_CTOR(ShadowMap)
        PYAPI_METHOD(ShadowMap, setTextureSize)
        PYAPI_METHOD(ShadowMap, setSoft)
        PYAPI_METHOD(ShadowMap, isSoft)
        PYAPI_METHOD(ShadowMap, setSoftShadowParameters)
        PYAPI_METHOD(ShadowMap, setDirty)
        ;

    // ShadowMap
    //PYAPI_REF_CLASS_WITH_CTOR(SoftShadowMap, ShadowMap)
        ;

    // Light
    PYAPI_REF_CLASS(Light, SceneNode)
        .def("create", &Light::create, PYAPI_RETURN_REF).staticmethod("create")
        .def("setColor", &Light::setColor)
        .def("setAmbient", &Light::setAmbient)
        .def("setEnabled", &Light::setEnabled)
        .def("isEnabled", &Light::isEnabled)
        PYAPI_METHOD(Light, setLayer)
        PYAPI_REF_GETTER(Light, getLayer)
        PYAPI_METHOD(Light, setAttenuation)
        PYAPI_GETTER(Light, getAttenuation)
        PYAPI_METHOD(Light, getLightType)
        PYAPI_METHOD(Light, setLightType)
        PYAPI_METHOD(Light, getLightFunction)
        PYAPI_METHOD(Light, setLightFunction)
        PYAPI_METHOD(Light, setLightDirection)
        PYAPI_METHOD(Light, getLightDirection)
        PYAPI_METHOD(Light, getSpotCutoff)
        PYAPI_METHOD(Light, setSpotCutoff)
        PYAPI_METHOD(Light, getSpotExponent)
        PYAPI_METHOD(Light, setSpotExponent)
        PYAPI_METHOD(Light, setShadow)
        PYAPI_METHOD(Light, setShadowRefreshMode)
        PYAPI_REF_GETTER(Light, getShadow)
        ;

    // ModelInfo
    PYAPI_REF_BASE_CLASS_WITH_CTOR(ModelInfo)
        .def_readwrite("name", &ModelInfo::name)
        .def_readwrite("options", &ModelInfo::options)
        .def_readwrite("generateNormals", &ModelInfo::generateNormals)
        .def_readwrite("generateTangents", &ModelInfo::generateNormals)
        .def_readwrite("numFiles", &ModelInfo::numFiles)
        .def_readwrite("path", &ModelInfo::path)
        .def_readwrite("size", &ModelInfo::size)
        .def_readwrite("optimize", &ModelInfo::optimize)
        .def_readwrite("usePowerOfTwoTextures", &ModelInfo::usePowerOfTwoTextures)
        .def_readwrite("loaderOutput", &ModelInfo::loaderOutput)
        .def_readwrite("mapName", &ModelInfo::mapName)
        .def_readwrite("readerWriterOptions", &ModelInfo::readerWriterOptions)
        ;

    // SkyBox
    PYAPI_REF_BASE_CLASS_WITH_CTOR(Skybox)
        PYAPI_METHOD(Skybox, loadCubeMap)
        ;

    // Free Functions
    def("getSceneManager", getSceneManager, PYAPI_RETURN_REF);



    // Mouse Mapping of GUIEventAdapter

    PYAPI_ENUM(osgGA::GUIEventAdapter::MouseButtonMask, OsgMouseButtons)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, LEFT_MOUSE_BUTTON)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, MIDDLE_MOUSE_BUTTON)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, RIGHT_MOUSE_BUTTON)
        ;

    PYAPI_ENUM(osgGA::GUIEventAdapter::EventType, OsgEventType)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, NONE)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, PUSH)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, RELEASE)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, DOUBLECLICK)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, DRAG)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, MOVE)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEYDOWN)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEYUP)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, FRAME)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, RESIZE)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, SCROLL)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, PEN_PRESSURE)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, PEN_ORIENTATION)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, PEN_PROXIMITY_ENTER)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, PEN_PROXIMITY_LEAVE)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, CLOSE_WINDOW)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, QUIT_APPLICATION)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, USER)
           ;

    PYAPI_ENUM(osgGA::GUIEventAdapter::ScrollingMotion, OsgScrollingMotion)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, SCROLL_NONE)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, SCROLL_LEFT)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, SCROLL_RIGHT)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, SCROLL_UP)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, SCROLL_DOWN)
           PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, SCROLL_2D)
           ;


    PYAPI_ENUM(osgGA::GUIEventAdapter::KeySymbol, OsgKeySymbol)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Space)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_0)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_1)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_2)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_3)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_4)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_5)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_6)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_7)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_8)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_9)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_A)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_B)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_C)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_D)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_E)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_F)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_G)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_H)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_I)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_J)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_K)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_L)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_M)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_N)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_O)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_P)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Q)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_R)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_S)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_T)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_U)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_V)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_W)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_X)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Y)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Z)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_F35)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Home)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Left)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Up)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Right)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Down)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Prior)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Page_Up)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Next)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Page_Down)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_End)
        PYAPI_ENUM_VALUE(osgGA::GUIEventAdapter, KEY_Begin)
        ;


    PYAPI_REF_BASE_CLASS(ConfigurableOptions)
        PYAPI_PROPERTY(ConfigurableOptions, fixVerticalAxis)
        PYAPI_PROPERTY(ConfigurableOptions, flags)
        ;

    class_<EventAdapter, EventAdapterCallback, boost::noncopyable>("EventAdapter")
        PYAPI_METHOD(EventAdapterCallback, mapButton)
        PYAPI_METHOD(EventAdapterCallback, mapXY)
        PYAPI_METHOD(EventAdapterCallback, setInputRange)
        PYAPI_METHOD(EventAdapterCallback, mapScrollingMotion)
        PYAPI_METHOD(EventAdapterCallback, mapEventType)
        .def("getLastEvent", &EventAdapterCallback::getLastEvent, return_value_policy<reference_existing_object>())
        .def("getConfigOptions", &EventAdapterCallback::getConfigOptions, return_value_policy<reference_existing_object>())
        ;
        

}

