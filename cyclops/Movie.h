/**
Data Arena Movie object

[Darren 10Jun14]

*/
#ifndef __CY_MOVIE__
#define __CY_MOVIE__

#include "cyclops/Entity.h"
#include "cyclops/SceneManager.h"
#include <osgDB/ReadFile>
#include <osg/ImageStream>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/PositionAttitudeTransform>

namespace cyclops {
	using namespace omega;
	using namespace omegaOsg;

	class CY_API Movie: public Entity {
	public:
		
		static Movie* create(const String& filePath, float width, float height);

		
		Movie(SceneManager* scene, const String& filePath="", float width=1.0f, float height=1.0f);
		
		virtual void rewind();
		virtual void play();
		virtual void pause();
		virtual void stop();
		
		virtual void setLooping(bool loop);
		
		virtual double getCurrentTime();
		virtual void seek(double time);
		virtual osg::ImageStream::StreamStatus getStreamStatus();
		
		virtual int getMovieWidth();
		virtual int getMovieHeight();
		
		virtual double getLength();
		
		virtual void setMaskEnabled(bool enabled);
		virtual bool isMaskEnabled() { return maskEnabled; }
		virtual void setMask(osg::Image* image);
		virtual void setMaskFromFile(const String& filePath);
		
		virtual bool isPlaying();
		
		virtual float getAspectRatio() { return _aspectRatio; };
		const String& getPath() { return myFilePath; }
		
	protected:
		
		virtual void ffmpegPluginCheck();
 		static bool ffmpegPluginLoaded;
		
		// shader to use
		static const char *shaderSourceTex2D;
		
	private:
		
		float _aspectRatio;
		
		float myWidth;
		float myHeight;
		String myFilePath;
		osg::ref_ptr<osg::ImageStream> imagestream;
		
		osg::Image* imageMask;
		bool maskEnabled;
		
		osg::Texture2D* texture_mask;
		osg::StateSet* movieSet;
		osg::StateSet* stateset;
		
	};

}

#endif
