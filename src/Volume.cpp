/**
Data Arena Volume object

[Darren 27Nov14]

*/

#include "cyclops/Volume.h"

// #include <osgDB/ReadFile>
#include <osg/ImageStream>

// #include <osg/Geode>
// #include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
// #include <osg/PositionAttitudeTransform>

// included from osgvolume example
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture3D>
#include <osg/Texture1D>
#include <osg/ImageSequence>
#include <osg/TexGen>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/PositionAttitudeTransform>
#include <osg/ClipNode>
#include <osg/AlphaFunc>
#include <osg/TexGenNode>
#include <osg/TexEnv>
#include <osg/TexEnvCombine>
#include <osg/Material>
#include <osg/PrimitiveSet>
#include <osg/Endian>
#include <osg/BlendFunc>
#include <osg/BlendEquation>
#include <osg/TransferFunction>
#include <osg/MatrixTransform>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgGA/EventVisitor>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/KeySwitchMatrixManipulator>

#include <osgUtil/CullVisitor>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/TabPlaneTrackballDragger>
#include <osgManipulator/TrackballDragger>

#include <osg/io_utils>

#include <algorithm>
#include <iostream>

#include <osg/ImageUtils>
#include <osgVolume/Volume>
#include <osgVolume/VolumeTile>
#include <osgVolume/RayTracedTechnique>
#include <osgVolume/FixedFunctionTechnique>

#include <osg/PositionAttitudeTransform>

using namespace cyclops;

enum ShadingModel
{
    Standard,
    LightShader,
    Isosurface,
    MaximumIntensityProjection
};

osg::Image* createTexture3D(osg::ImageList& imageList,
            unsigned int numComponentsDesired,
            int s_maximumTextureSize,
            int t_maximumTextureSize,
            int r_maximumTextureSize,
            bool resizeToPowerOfTwo)
{

    if (numComponentsDesired==0)
    {
        return osg::createImage3DWithAlpha(imageList,
                                        s_maximumTextureSize,
                                        t_maximumTextureSize,
                                        r_maximumTextureSize,
                                        resizeToPowerOfTwo);
    }
    else
    {
        GLenum desiredPixelFormat = 0;
        switch(numComponentsDesired)
        {
            case(1) : desiredPixelFormat = GL_LUMINANCE; break;
            case(2) : desiredPixelFormat = GL_LUMINANCE_ALPHA; break;
            case(3) : desiredPixelFormat = GL_RGB; break;
            case(4) : desiredPixelFormat = GL_RGBA; break;
        }

        return osg::createImage3D(imageList,
                                        desiredPixelFormat,
                                        s_maximumTextureSize,
                                        t_maximumTextureSize,
                                        r_maximumTextureSize,
                                        resizeToPowerOfTwo);
    }
}

osg::TransferFunction1D* readTransferFunctionFile(const std::string& filename, float colorScale=1.0f)
{
    std::string foundFile = osgDB::findDataFile(filename);
    if (foundFile.empty())
    {
        std::cout<<"Error: could not find transfer function file : "<<filename<<std::endl;
        return 0;
    }

    std::cout<<"Reading transfer function "<<filename<<std::endl;

    osg::TransferFunction1D::ColorMap colorMap;
    osgDB::ifstream fin(foundFile.c_str());
    while(fin)
    {
        float value, red, green, blue, alpha;
        fin >> value >> red >> green >> blue >> alpha;
        if (fin)
        {
            std::cout<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")"<<std::endl;
            colorMap[value] = osg::Vec4(red*colorScale,green*colorScale,blue*colorScale,alpha*colorScale);
        }
    }

    if (colorMap.empty())
    {
        std::cout<<"Error: No values read from transfer function file: "<<filename<<std::endl;
        return 0;
    }

    osg::TransferFunction1D* tf = new osg::TransferFunction1D;
    tf->assign(colorMap);

    return tf;
}


Volume* Volume::create(const String& filePath, const String& tfPath) {
	return new Volume(SceneManager::instance(), filePath, tfPath);
}

Volume::~Volume()
{
}

Volume::Volume(SceneManager* scene, const String& filePath, const String& tfPath):
	Entity(scene),
	resizeToPowerOfTwo(false),
	myFilePath(filePath) {

	String path;

	// if no model name found:
	if(filePath == "") {
		owarn("No path specified!!");
		return;
	}

	osg::ImageList imageList;
	osgDB::DirectoryContents contents = osgDB::getSortedDirectoryContents(filePath);

	ofmsg("number of files: %1%", %contents.size());

	for (unsigned int i = 0; i < contents.size(); ++i)
	{

		ofmsg("Looking at %1%", %contents[i]);

		osg::Image *image = osgDB::readImageFile( filePath + contents[i] );

		if(image)
		{
			ofmsg("Reading %1%", %image->getFileName());
			OSG_NOTICE<<"Read osg::Image FileName::"<<image->getFileName()<<", pixelFormat=0x"<<std::hex<<image->getPixelFormat()<<std::dec<<", s="<<image->s()<<", t="<<image->t()<<", r="<<image->r()<<std::endl;
			imageList.push_back(image);
		}
	}


	if(!DataManager::findFile(filePath, path)) {
		ofwarn("!File not found: %1%", %filePath);
		return;
	}

	// transfer function stuff
	bool useShader = false;
    osg::ref_ptr<osg::TransferFunction1D> transferFunction;

	if (tfPath != "") {
		transferFunction = readTransferFunctionFile(tfPath, 1.0f/255.0f);
	} else {
		transferFunction = new osg::TransferFunction1D;
		transferFunction->setColor(0.0, osg::Vec4(1.0,0.0,0.0,0.0));
		transferFunction->setColor(0.5, osg::Vec4(1.0,1.0,0.0,0.5));
		transferFunction->setColor(1.0, osg::Vec4(0.0,0.0,1.0,1.0));
		transferFunction->assign(transferFunction->getColorMap());
	}

	osgVolume::TransferFunctionProperty* tfp = new osgVolume::TransferFunctionProperty(transferFunction.get());

// 	osg::Image* image_3d = createTexture3D(imageList, 0, 2048, 2048, 2048, resizeToPowerOfTwo);
	osg::Image* image_3d = osg::createImage3DWithAlpha(imageList, 2048, 2048, 2048, resizeToPowerOfTwo);

	if (image_3d == NULL) {
		owarn("Image not created!");
		return;
	}

	int image_s = image_3d->s();
	int image_t = image_3d->t();
	int image_r = image_3d->r();

    osg::ref_ptr<osgVolume::ImageDetails> details = dynamic_cast<osgVolume::ImageDetails*>(image_3d->getUserData());
    osg::ref_ptr<osg::RefMatrix> matrix = details ? details->getMatrix() : dynamic_cast<osg::RefMatrix*>(image_3d->getUserData());

	float maxDimSize = 0.0f;

	float xSize = static_cast<float>(image_s);
	float ySize = static_cast<float>(image_t);
	float zSize = static_cast<float>(image_r);

    if (!matrix)
    {

		maxDimSize = xSize >= ySize ? xSize : ySize;
		maxDimSize = maxDimSize >= zSize ? maxDimSize : zSize;

        matrix = new osg::RefMatrix(xSize, 0.0,   0.0,   0.0,
                                    0.0,   ySize, 0.0,   0.0,
                                    0.0,   0.0,   zSize, 0.0,
                                    0.0,   0.0,   0.0,   1.0);
    }

	float xMultiplier = 1.0f;
	float yMultiplier = 1.0f;
	float zMultiplier = 2.5f;

    if (xMultiplier!=1.0 || yMultiplier!=1.0 || zMultiplier!=1.0)
    {
        matrix->postMultScale(osg::Vec3d(fabs(xMultiplier), fabs(yMultiplier), fabs(zMultiplier)));
    }

	if (!useShader) {
		image_3d = osgVolume::applyTransferFunction(image_3d, transferFunction.get());
	}


	osg::ref_ptr<osgVolume::Volume> volume = new osgVolume::Volume;
    osg::ref_ptr<osgVolume::VolumeTile> tile = new osgVolume::VolumeTile;
    volume->addChild(tile.get());

    osg::ref_ptr<osgVolume::ImageLayer> layer = new osgVolume::ImageLayer(image_3d);

    if (details)
    {
        layer->setTexelOffset(details->getTexelOffset());
        layer->setTexelScale(details->getTexelScale());
    }

	// rescale input to be between 0-1.0
	layer->rescaleToZeroToOneRange();

	layer->setLocator(new osgVolume::Locator(*matrix));
	tile->setLocator(new osgVolume::Locator(*matrix));

    tile->setLayer(layer.get());

	// applying transfer function
	ShadingModel shadingModel = Standard;
	float alphaFunc = 0.02f;
	double sampleDensityWhenMoving = 0.0;

    if (useShader)
    {

        osgVolume::SwitchProperty* sp = new osgVolume::SwitchProperty;
        sp->setActiveProperty(0);

        osgVolume::AlphaFuncProperty* ap = new osgVolume::AlphaFuncProperty(alphaFunc);
        osgVolume::SampleDensityProperty* sd = new osgVolume::SampleDensityProperty(0.005);
        osgVolume::SampleDensityWhenMovingProperty* sdwm = sampleDensityWhenMoving!=0.0 ? new osgVolume::SampleDensityWhenMovingProperty(sampleDensityWhenMoving) : 0;
        osgVolume::TransparencyProperty* tp = new osgVolume::TransparencyProperty(1.0);
        osgVolume::TransferFunctionProperty* tfp = transferFunction.valid() ? new osgVolume::TransferFunctionProperty(transferFunction.get()) : 0;

        {
            // Standard
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            if (sdwm) cp->addProperty(sdwm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // Light
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::LightingProperty);
            if (sdwm) cp->addProperty(sdwm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // Isosurface
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::IsoSurfaceProperty(alphaFunc));
            if (sdwm) cp->addProperty(sdwm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // MaximumIntensityProjection
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::MaximumIntensityProjectionProperty);
            if (sdwm) cp->addProperty(sdwm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        switch(shadingModel)
        {
            case(Standard):                     sp->setActiveProperty(0); break;
			case(LightShader):                  sp->setActiveProperty(1); break;
            case(Isosurface):                   sp->setActiveProperty(2); break;
            case(MaximumIntensityProjection):   sp->setActiveProperty(3); break;
        }
        layer->addProperty(sp);

		tile->setVolumeTechnique(new osgVolume::RayTracedTechnique);
    }
    else
    {
        layer->addProperty(new osgVolume::AlphaFuncProperty(alphaFunc));
        tile->setVolumeTechnique(new osgVolume::FixedFunctionTechnique);
    }

    if (volume.valid()) {
		osg::ref_ptr<osg::Node> loadedModel = volume.get();
		osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();
		pat->setScale(osg::Vec3(1.0f/maxDimSize, 1.0f/maxDimSize, 1.0f/maxDimSize));
 		pat->setPosition(osg::Vec3((xSize / maxDimSize) * -.5f,
								   (ySize / maxDimSize) * -.5f,
 								   -(zSize / maxDimSize)));
		pat->addChild(loadedModel.get());

		osg::ref_ptr<osg::Node> node = pat;


		initialize(node);


	} else {
		omsg("Volume not valid!");
	}

}
