/**
Data Arena Movie object

[Darren 10Jun14]

*/

#include "cyclops/Movie.h"

#include <osgDB/ReadFile>
#include <osg/Geode>
#include <osg/Geometry>

using namespace cyclops;

bool Movie::ffmpegPluginLoaded = false;

const char *Movie::shaderSourceTex2D = {
	"uniform sampler2D movie_texture;\n"
	"uniform sampler2D mask_texture;\n"
	"uniform int mask_enabled;\n"
	"void main(void)\n"
	"{\n"
	"    vec4 texture_color = texture2D(movie_texture, gl_TexCoord[0].st); \n"
	"    gl_FragColor = texture_color;\n"
	"    if (mask_enabled) gl_FragColor[3] = texture2D(mask_texture, gl_TexCoord[0].st);\n"
	"}\n"
};

void Movie::ffmpegPluginCheck() {
	
 	if (!Movie::ffmpegPluginLoaded) {
		
		// force ffmpeg plugin
		std::string libName = osgDB::Registry::instance()->createLibraryNameForExtension("ffmpeg");
		osgDB::Registry::instance()->loadLibrary(libName);
 		Movie::ffmpegPluginLoaded = true;
 	}	
}

Movie* Movie::create(const String& filePath, float width, float height) {
	return new Movie(SceneManager::instance(), filePath, width, height);
}

Movie::~Movie()
{
}


Movie::Movie(SceneManager* scene, const String& filePath, float width, float height):
	Entity(scene),
	myWidth(width),
	myHeight(height),
	myFilePath(filePath) {

	imageMask = NULL;
	maskEnabled = false;
	
	String path;
		
	// if no model name found:
	if(filePath == "") {
		owarn("No movie specified!!");
		return;
	}
	
	if(!DataManager::findFile(filePath, path)) {
		ofwarn("!File not found: %1%", %filePath);
		return;
	}
	
	ffmpegPluginCheck();
		
	// The node containing the scene
	osg::Geode* geode = new osg::Geode();

	// statesets
	
	stateset = geode->getOrCreateStateSet();
	

	osg::Program* program = new osg::Program;
	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, Movie::shaderSourceTex2D ));
	stateset->addUniform(new osg::Uniform("movie_texture",0));
	stateset->addUniform(new osg::Uniform("mask_texture", 1));
	stateset->addUniform(new osg::Uniform("mask_enabled", (int) maskEnabled));
	stateset->setAttribute(program);

	imagestream = NULL;
	osg::ref_ptr<osg::Image> image = osgDB::readImageFile(path);
	imagestream = dynamic_cast<osg::ImageStream*>(image.get());
	
	if (!imagestream) {
		ofwarn("!Failed to load movie: %1% (unsupported file format or corrupted data)", %path);
		return;
	}
	
	imagestream->setLoopingMode(osg::ImageStream::NO_LOOPING);
	
	_aspectRatio = imagestream->s() * imagestream->getPixelAspectRatio() / imagestream->t();
	
	bool flip = imagestream->getOrigin()==osg::Image::TOP_LEFT;

	osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(osg::Vec3(0.0f, 0.0f, 0.0f),
		osg::Vec3(myWidth, 0.0f, 0.0f),
		osg::Vec3(0.0f, myHeight, 0.0f),
		0.0f, 
		flip ? 1.0f : 0.0f , 
		1.0f, 
		flip ? 0.0f : 1.0f);

	osg::Texture2D* texture = new osg::Texture2D();
	texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setResizeNonPowerOfTwoHint(false);

	texture->setImage(imagestream);
	
	movieSet = pictureQuad->getOrCreateStateSet();
	movieSet->setTextureAttributeAndModes(0,
		texture,
		osg::StateAttribute::ON);

	texture_mask = new osg::Texture2D();
	texture_mask->setImage(imageMask);
	texture_mask->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture_mask->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture_mask->setResizeNonPowerOfTwoHint(false);
	movieSet->setTextureAttributeAndModes(1,
		texture_mask);
	
	geode->addDrawable(pictureQuad);

	initialize(geode);

	getMaterial()->setColor(Color(1,1,1), Color(0,0,0));
	getMaterial()->setTransparent(true);
	
}

inline int Movie::getMovieWidth() { if (imagestream) { return imagestream->s(); } else { return 0; } }

inline int Movie::getMovieHeight() { if (imagestream) { return imagestream->t(); } else { return 0; } }


void Movie::rewind() {
	if (!imagestream) return;
	
	imagestream->rewind();
	
}

void Movie::play() {
	if (!imagestream) return;
	
	if (imagestream->getStatus() != osg::ImageStream::PLAYING) {
		std::cout<< imagestream << " Play" << std::endl;
		imagestream->play();
	}
	
}

void Movie::pause() {
	if (!imagestream) return;
	if (imagestream->getStatus() == osg::ImageStream::PLAYING) {
		std::cout<< imagestream << " Pause" << std::endl;
		imagestream->pause();
	}
	
}

void Movie::stop() {
	if (!imagestream) return;
	imagestream->rewind();
	imagestream->pause();
}

void Movie::setLooping(bool loop) {
	if (!imagestream) return;
	if (loop) {
		imagestream->setLoopingMode(osg::ImageStream::LOOPING);
	} else {
		imagestream->setLoopingMode(osg::ImageStream::NO_LOOPING);
	}
}

double Movie::getCurrentTime() {
	if (!imagestream) return -1;
	
	return imagestream->getCurrentTime();
}

void Movie::seek(double time) {
	if (!imagestream) return;
	std::cout<< imagestream.get() << " Seek: "<< time <<std::endl;
	
	if (imagestream->getStatus() == osg::ImageStream::INVALID) {
		omsg("stream status is INVALID");
	} else if (imagestream->getStatus() == osg::ImageStream::PLAYING) {
		omsg("stream status is PLAYING");
	} else if (imagestream->getStatus() == osg::ImageStream::PAUSED) {
		omsg("stream status is PAUSED");
	} else if (imagestream->getStatus() == osg::ImageStream::REWINDING) {
		omsg("stream status is REWINDING");
	}
	
	imagestream->seek(time);
}

void Movie::setMaskEnabled(bool enabled) {
	maskEnabled = enabled;
	stateset->getUniform("mask_enabled")->set((int) maskEnabled);
}

void Movie::setMask(osg::Image* image) {
	imageMask = image;
	
	if (imageMask) {
		texture_mask->setImage(imageMask);
		movieSet->setTextureAttributeAndModes(1, texture_mask);
		maskEnabled = true;
	}
}

void Movie::setMaskFromFile(const String& filePath) {
	setMask(osgDB::readImageFile(filePath));
}

bool Movie::isPlaying() {

	if (imagestream) { 
		return (imagestream->getStatus() == osg::ImageStream::PLAYING);
	} 
	
	return false;
}

osg::ImageStream::StreamStatus Movie::getStreamStatus() {
	
	if (imagestream) {
		return imagestream->getStatus();
	}
	
	return osg::ImageStream::INVALID;
}

double Movie:: getLength() {
	if (imagestream) {
		return imagestream->getLength();
	}
	
	return 0.0;
}
