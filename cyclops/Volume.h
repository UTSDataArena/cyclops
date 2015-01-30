#ifndef __CY_VOLUME__
#define __CY_VOLUME__

#include "Entity.h"
#include "SceneManager.h"

namespace cyclops {
	using namespace omega;
	using namespace omegaOsg;

	class CY_API Volume: public Entity {
	public:

		static Volume* create(const String& filePath, const String& tfPath);

		Volume(SceneManager* scene, const String& filePath="", const String& tfPath="");
		~Volume();

	protected:

		float alphaFunc;
		String myFilePath;

		osg::Image* image;
		bool resizeToPowerOfTwo;

		osg::StateSet* stateset;

	};
}

#endif
