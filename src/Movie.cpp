/**
Data Arena Movie object

[Darren 10Jun14]

*/

#include "cyclops/Movie.h"

#include <osgDB/ReadFile>
#include <osg/Geode>
#include <osg/Geometry>

using namespace cyclops;

Movie* Movie::create(const String& filePath, float width, float height) {
	return new Movie(SceneManager::instance(), filePath, width, height);
}

Movie::~Movie()
{
// 	ofmsg("ref count for stateset: %1%", %stateset->referenceCount());
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

	ofmsg("Loading image %1%", %filePath);
	// false means we dont try to check for file existence first
	osg::Texture2D* texture = getSceneManager()->getTexture(filePath, false); // put movie through scene manager
	imagestream = dynamic_cast<osg::ImageStream*>(texture->getImage());


	if (!imagestream) {
		ofwarn("!Failed to load movie: %1% (unsupported file format or corrupted data)", %filePath);
		return;
	}

	imagestream->rewind();
	imagestream->pause();
	imagestream->setLoopingMode(osg::ImageStream::NO_LOOPING);

	_aspectRatio = imagestream->s() * imagestream->getPixelAspectRatio() / imagestream->t();

	bool flip = imagestream->getOrigin()==osg::Image::TOP_LEFT;

	osg::ref_ptr<osg::Geometry> pictureQuad = osg::createTexturedQuadGeometry(osg::Vec3(0.0f, 0.0f, 0.0f),
		osg::Vec3(myWidth, 0.0f, 0.0f),
		osg::Vec3(0.0f, myHeight, 0.0f),
		0.0f,
		flip ? 1.0f : 0.0f ,
		1.0f,
		flip ? 0.0f : 1.0f);

	setEffect("movie");

	texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
	texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
	texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
	texture->setResizeNonPowerOfTwoHint(false);
	texture->setImage(0, imagestream.get());

 	stateset = getMaterial()->getStateSet();

	initialize(pictureQuad.get());

	getMaterial()->setColor(Color(1,1,1), Color(0,0,0));
	getMaterial()->setTransparent(true);

	attachUniforms();

}

void Movie::attachUniforms() {
	getMaterial()->addUniform("movie_texture", Uniform::Int)->setInt(0);
	getMaterial()->setTexture(myFilePath, 0, "movie_texture");

	getMaterial()->addUniform("mask_texture", Uniform::Int)->setInt(1);
	if (imageMask) {
		getMaterial()->setTexture(imageMask->getFileName(), 1, "mask_texture");
	}

	getMaterial()->addUniform("mask_enabled", Uniform::Int)->setInt((int) maskEnabled);
	setMaskEnabled(maskEnabled);

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

bool Movie::isLooping() {
	if (!imagestream) return false;
	return (imagestream->getLoopingMode() == osg::ImageStream::LOOPING);
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


// gstreamer seeks in ms, ffmpeg seeks in seconds
void Movie::seek(double time) {
	if (!imagestream) return;

// 	if (time < 0 || time >= imagestream->getLength()) {
// 		std::cout<< imagestream << " Invalid Seek: "<< time <<std::endl;
// 		return;
// 	}

	std::cout<< imagestream << " Seek: "<< time <<std::endl;

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

	if (getMaterial()->getUniform("mask_enabled")) {
		getMaterial()->getUniform("mask_enabled")->setInt((int) maskEnabled);
	}
}

void Movie::setMask(osg::Image* image) {
	if (image) {
		ofmsg("setting mask of image %1%", %myFilePath);
		getMaterial()->setTexture(myFilePath, 1, "mask_texture");

		setMaskEnabled(true);
	}
}

void Movie::setMaskFromFile(const String& filePath) {
	osg::Texture2D* texMask = getSceneManager()->getTexture(filePath);

	if (texMask) {
		imageMask = texMask->getImage();
		setMask(texMask->getImage());
	}
}

bool Movie::isPlaying() {
	if (!imagestream) return false;
	return (imagestream->getStatus() == osg::ImageStream::PLAYING);
}

osg::ImageStream::StreamStatus Movie::getStreamStatus() {
	if (!imagestream) return osg::ImageStream::INVALID;
	return imagestream->getStatus();
}

double Movie:: getLength() {
	if (!imagestream) return 0.0;;
	return imagestream->getLength();
}
