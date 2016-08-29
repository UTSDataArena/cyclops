#include "cyclops/CameraManipulator.h"
#include "cyclops/SceneManager.h"

#include <osg/MatrixTransform>
#include <osg/ComputeBoundsVisitor>
#include <iostream>


using namespace cyclops;

#define OOSG_VEC3(v) osg::Vec3(v[0], v[1], v[2])
#define OSGVEC3_OMEGA(v) Vector3f( v.x() , v.y(), v.z() )
#define P_OSGVEC(v)  std::cout << v.x() << " " << v.y() << " " << v.z() << std::endl




ManipulatorController::ManipulatorController(Camera *cam) : CameraController() {
    if (cam == NULL)
        myCamera = Engine::instance()->getDefaultCamera();
    else
        myCamera = cam;

    myCamera->setController(this);
}


void ManipulatorController::onEvent(Event* event)
{
    if (myManipulator==NULL)
        return;

    myManipulator->handle(event);
}



void ManipulatorController::update(const UpdateContext& context)
{
    if(!isEnabled() || myManipulator==NULL) return;

    myManipulator->updateOmegaCamera(myCamera);
}



void ManipulatorController::setManipulator(AbstractOmegaManipulator* manipulator, osg::Node* node){
    myManipulator = manipulator;
    if (node == NULL)
        myManipulator->_setNode(SceneManager::instance()->getOsgModule()->getRootNode() );
    else 
        myManipulator->_setNode(node);
    
    // default, can be overwritten by setEventAdapter
    MouseAdapter* adapter = new MouseAdapter;
    myManipulator->setEventAdapter(adapter);
}


void ManipulatorController::setEventAdapter(EventAdapter* adapter){
    myManipulator->setEventAdapter(adapter);
}


ManipulatorController* ManipulatorController::create(){
    return new ManipulatorController();
}




//=====================================


void AbstractOmegaManipulator::updateOmegaCamera(Camera *cam){
    std::cout << "update omega cam" << std::endl;
    osg::Vec3d eye, center, up;
    _getTransformation(eye, center, up);


    Vector3f oPosVec(eye.x(), eye.y(), eye.z());
    Vector3f oUpVec(up.x(), up.y(), up.z());
    Vector3f oCenterVec(center.x(), center.y(), center.z());

    
    //order is important here, setting lookat before position 
    // will result in choppy camera rotation
    cam->setPosition(oPosVec);
    cam->lookAt(oCenterVec, oUpVec);     
}


void AbstractOmegaManipulator::setHomeEye(const Vector3f& eye){
    osg::Vec3d unusedEye, center, up;
    _getHomePosition(unusedEye, center, up);

    osg::Vec3d convertedEye = OOSG_VEC3( eye );
    _setHomePosition( convertedEye, //OOSG_VEC3( omegaCam->getPosition() ),
                center,
                up);
    _home(0.0);
}


void AbstractOmegaManipulator::setHome(const Vector3f& eye, const Vector3f& center, const Vector3f& up){
    osg::Vec3d convertedEye = OOSG_VEC3( eye );
    osg::Vec3d convertedCenter = OOSG_VEC3( center );
    osg::Vec3d convertedUp = OOSG_VEC3( up );
    
    _setHomePosition( convertedEye, //OOSG_VEC3( omegaCam->getPosition() ),
        convertedCenter,
        convertedUp);

    _home(0.0);
}


Vector3f AbstractOmegaManipulator::getCameraCenter(){
    osg::Vec3d eye, center, up;
    _getTransformation(eye, center, up);

    Vector3f oCenterVec(center.x(), center.y(), center.z());


    return oCenterVec;
}


//=====================================


OrbitManipulator::OrbitManipulator(int flags) : inherited( flags )
{
    handler.set(this);
    setVerticalAxisFixed(false);
}

//=======================================================



NodeTrackerManipulator::NodeTrackerManipulator( int flags) : inherited( flags )
{
    handler.set(this);
    setVerticalAxisFixed(false);
}

void NodeTrackerManipulator::setModes(std::string trackerMode, std::string rotationMode)
{
    if (trackerMode.compare("NODE_CENTER_AND_ROTATION") == 0)
        setTrackerMode( osgGA::NodeTrackerManipulator::NODE_CENTER_AND_ROTATION );
    else if (trackerMode.compare("NODE_CENTER") == 0)
        setTrackerMode( osgGA::NodeTrackerManipulator::NODE_CENTER);
    else if (trackerMode.compare("NODE_CENTER_AND_AZIM") == 0)
        setTrackerMode(osgGA::NodeTrackerManipulator::NODE_CENTER_AND_AZIM);
    else
        ofwarn("[NodeTrackerManipulator::setModes] tracker mode: %1% not supported!", %trackerMode);


    if (rotationMode.compare("ELEVATION_AZIM")==0)
        setRotationMode( osgGA::NodeTrackerManipulator::ELEVATION_AZIM);
    else if (rotationMode.compare("TRACKBALL")==0)
        setRotationMode(osgGA::NodeTrackerManipulator::TRACKBALL);
    else
        ofwarn("[NodeTrackerManipulator::setModes] rotation mode: %1% not supported!", %rotationMode);
}


void NodeTrackerManipulator::setTrackedNode(Entity* entity) {
    setTrackNode( entity->getOsgNode() );


    osg::BoundingSphere boundingSphere;
    osg::ComputeBoundsVisitor cbVisitor;
    entity->getOsgNode()->accept(cbVisitor);
    osg::BoundingBox &bb = cbVisitor.getBoundingBox();

    if (bb.valid()) boundingSphere.expandBy(bb);
    else boundingSphere = entity->getOsgNode()->getBound();



    setHomePosition( boundingSphere.center() + osg::Vec3(0, 50.0, 0),
                    boundingSphere.center(),
                    osg::Z_AXIS);

    home(0.0);
}




void NodeTrackerManipulator::updateOmegaCamera(Camera *cam){
    osg::Vec3d eye, unused_center, up;

    // call same method, that osg internally uses for its camera updates
    osg::Matrixd invMatrix = getInverseMatrix();
    invMatrix.getLookAt(eye, unused_center, up);

    osg::NodePath nodePath;
    //get the path from the tracked node to the top level element
    getTrackNodePath().getNodePath(nodePath);
    //compute a transform from the tracked node space to world space
    osg::Matrixd localToWorld = osg::computeLocalToWorld(nodePath, true);
    // apply this transformation to the cameramanipulator center
    // this is normally the tracked node center, but might be panned
    osg::Vec3d worldCenter = _center * localToWorld;

    omega::Vector3f oCenterVec = OSGVEC3_OMEGA(worldCenter);
    // set the lookat of omega camera
    //order is important here, setting lookat before position 
    // will result in choppy camera rotation
    cam->setPosition( OSGVEC3_OMEGA(eye) );
    cam->lookAt(oCenterVec, OSGVEC3_OMEGA(up) );   
}



//=======================================================



TerrainManipulator::TerrainManipulator(int flags) : inherited( flags )
{
    handler.set(this);
    setVerticalAxisFixed(false);
    // setRotationMode(ELEVATION_AZIM);
}


void TerrainManipulator::setTerrainNode(Entity *entity) {
    setNode( entity->getOsgNode() );
    home(0.0);
}




//=======================================================


// FirstPersonManipulator::FirstPersonManipulator(int flags) :  inherited( flags | SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT)
// {
//     setVerticalAxisFixed(true);
//     handler.set(this);
// }


// bool FirstPersonManipulator::handleMouseWheel( Event *event)
// {
//     int wheel = event->getExtraDataInt(0);



//     if(_flags & SET_CENTER_ON_WHEEL_FORWARD_MOVEMENT )
//     {

//         if( ((wheel < 0 && _wheelMovement > 0.)) ||
//             ((wheel > 0  && _wheelMovement < 0.)) )
//         {
//             // stop thrown animation
//             _thrown = false;

//                 // center by mouse intersection (no animation)
//                 handler.setCenterByMousePointerIntersection( event );
//         }
//     }


//     // mouse scroll up event
//     if (wheel > 0)
//     {
//         // perform zoom
//         // move forward
//         moveForward( isAnimating() ? dynamic_cast< FirstPersonAnimationData* >( _animationData.get() )->_targetRot : _rotation,
//                  -_wheelMovement * (getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize : 1. ));
//         return true;
//     } else {
//         // move backward
//         moveForward( _wheelMovement * (getRelativeFlag( _wheelMovementFlagIndex ) ? _modelSize : 1. ));
//         _thrown = false;
//         return true;
//     }
// }
