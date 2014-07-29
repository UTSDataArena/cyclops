/**
Data Arena Movie object

[Darren 10Jun14]

*/

#include "cyclops/Movie.h"

using namespace cyclops;

///////////////////////////////////////////////////////////////////////////////
osg::Geometry* Movie::myCreateTexturedQuadGeometry(const osg::Vec3& pos,float width,float height, osg::Image* image, bool xyPlane, bool option_flip)
{
	bool flip = image->getOrigin()==osg::Image::TOP_LEFT;
	if (option_flip) flip = !flip;

	osg::ref_ptr<osg::Geometry> pictureQuad = osg::createTexturedQuadGeometry(pos,
		osg::Vec3(width,0.0f,0.0f),
		xyPlane ? osg::Vec3(0.0f,height,0.0f) : osg::Vec3(0.0f,0.0f,height),
		0.0f, flip ? 1.0f : 0.0f , 1.0f, flip ? 0.0f : 1.0f);

	osg::Texture2D* texture = new osg::Texture2D();
	texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setResizeNonPowerOfTwoHint(false);

	texture->setImage(image);
	
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
	
	
// 		movieSet->setTextureAttributeAndModes(0,
// 			texture);
	

	return pictureQuad.release();
}


Movie* Movie::create(const String& filePath, float width, float height) {
	return new Movie(SceneManager::instance(), filePath, width, height);
}

Movie::Movie(SceneManager* scene, const String& filePath, float width, float height):
	Entity(scene),
	myWidth(width),
	myHeight(height),
	myFilePath(filePath) {

	imageMask = NULL;
	maskEnabled = false;
		
	static bool ffmpegPluginLoaded = false;		
		
	if (!ffmpegPluginLoaded) {
		
		// force ffmpeg plugin
		std::string libName = osgDB::Registry::instance()->createLibraryNameForExtension("ffmpeg");
		osgDB::Registry::instance()->loadLibrary(libName);
		ffmpegPluginLoaded = true;
	}
		
		
	// The node containing the scene
	osg::ref_ptr<osg::Node> node = new osg::Node();
	
	osg::ref_ptr<osg::Geode> geode = new osg::Geode();

	// if no model name found:
	if(filePath == "")
	{
		
		owarn("No movie specified!!");
		
		return;

	}
	
	stateset = geode->getOrCreateStateSet();
	// stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	
	// shader to use
	static const char *shaderSourceTex2D = {
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

	osg::Program* program = new osg::Program;

	program->addShader(new osg::Shader(osg::Shader::FRAGMENT, shaderSourceTex2D ));

	stateset->addUniform(new osg::Uniform("movie_texture",0));
	stateset->addUniform(new osg::Uniform("mask_texture", 1));
	stateset->addUniform(new osg::Uniform("mask_enabled", (int) maskEnabled));

	stateset->setAttribute(program);

	imagestream = NULL;
		
	String path;
	
	if(DataManager::findFile(filePath, path)) {
		
// 		ofmsg("reading %1%", %path);
		osg::ref_ptr<osg::Image> image = osgDB::readImageFile(path);
		imagestream = dynamic_cast<osg::ImageStream*>(image.get());
		
		if (imagestream) {
// 			ofmsg("i have an image stream with %1%", %imagestream->libraryName());
// 			ofmsg("i have %1% frames", %(imagestream->getLength() * imagestream->getFrameRate()));
			
			imagestream->setLoopingMode(osg::ImageStream::LOOPING);
// 			imagestream->play();
			
// 			ofmsg("stream status is %1%", %imagestream->getStatus());
			
			float width = imagestream->s() * imagestream->getPixelAspectRatio();
			float height = imagestream->t();
			
			_aspectRatio = width / height;
			
			// osg::Drawable* drawable = myCreateTexturedQuadGeometry(osg::Vec3(-myWidth / 2.0f, -myHeight / 2.0f, 0.0f), myWidth, myHeight, imagestream.get(), true, false);
			osg::Drawable* drawable = myCreateTexturedQuadGeometry(osg::Vec3(0.0f, 0.0f, 0.0f), myWidth, myHeight, imagestream.get(), true, false);
			geode->addDrawable(drawable);
			
		} else {
			ofwarn("!Failed to load movie: %1% (unsupported file format or corrupted data)", %path);
			return;
		}
		
		float r = geode->getBound().radius() * 2;
		float scale = 1 / r;
		
		/*
		osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();
		pat->setPosition(osg::Vec3(-(myWidth / 2.0), -(myHeight / 2.0), 0));
		pat->setScale(osg::Vec3(scale, scale, scale));
		pat->addChild(geode);
		*/
		node = geode;
		
		setCullingActive(false);
		initialize(node);

		getMaterial()->setColor(Color(1,1,1), Color(0,0,0));
		getMaterial()->setTransparent(true);
	}
	else
	{
		ofwarn("!File not found: %1%", %filePath);
		return;
	}

	
	
}

inline int Movie::getMovieWidth() { if (imagestream) { return imagestream->s(); } else { return 0; } }

inline int Movie::getMovieHeight() { if (imagestream) { return imagestream->t(); } else { return 0; } }


void Movie::rewind() {
	if (!imagestream) return;
	
	imagestream->rewind();
// 	imagestream->play();
	
}

void Movie::play() {
	if (!imagestream) return;
	
	if (imagestream->getStatus() != osg::ImageStream::PLAYING)
	{
		std::cout<< imagestream.get() << " Play" << std::endl;
		imagestream->play();
	}
	
}

void Movie::pause() {
	if (!imagestream) return;
	if (imagestream->getStatus() == osg::ImageStream::PLAYING)
	{
		std::cout<< imagestream.get() << " Pause" << std::endl;
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