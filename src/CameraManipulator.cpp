#include "cyclops/CameraManipulator.h"

#include <osg/MatrixTransform>
#include <osg/ComputeBoundsVisitor>
#include <iostream>


using namespace cyclops;

#define OOSG_VEC3(v) osg::Vec3(v[0], v[1], v[2])

NodeTrackerManipulator::NodeTrackerManipulator( int flags ) : inherited( flags ) 
{
    setVerticalAxisFixed(false);
}


bool NodeTrackerManipulator::handle(Event* event)
{
    if(event->isProcessed()) return false;


    if(event->getServiceType() == Service::Pointer)
    {
        if(event->getType() == Event::Move)
        {
            if (event->isFlagSet(Event::Right))
                handleMouseDrag(event);
            else 
                handleMouseMove(event);
        }
    }

    return true;
}




/** Add the current mouse GUIEvent to internal stack.*/
void NodeTrackerManipulator::addMouseEvent( Event *event )
{
    if (lastEvent != NULL)
        delete lastEvent;

    lastEvent = currentEvent;
    currentEvent = new Event;

    omicronConnector::EventData ed;
    event->serialize(&ed);
    currentEvent->deserialize(&ed);

    // osg guieventdapaters of parent class
    _ga_t1 = _ga_t0;
    osgGA::GUIEventAdapter* osgNewEvent = new osgGA::GUIEventAdapter;

    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    Vector2i resolution = ds->getDisplayConfig().getCanvasRect().size();

    osgNewEvent->setInputRange(0, 0, static_cast<float>(resolution[0]), static_cast<float>(resolution[1]) );
    osgNewEvent->setX(currentEvent->getPosition().x());
    osgNewEvent->setY(currentEvent->getPosition().y());

    _ga_t0 = osgNewEvent;


}



/** Reset the internal GUIEvent stack.*/
void NodeTrackerManipulator::flushMouseEventStack()
{
    lastEvent = NULL;
    currentEvent = NULL;
}



bool NodeTrackerManipulator::handleMouse()
{
    // return if less then two events have been added
    if( currentEvent == NULL || lastEvent == NULL )
        return false;

    // get delta time
    double eventTimeDelta = currentEvent->getTimestamp() - lastEvent->getTimestamp();
    if( eventTimeDelta < 0. )
    {
        std::cout << "Manipulator warning: eventTimeDelta = " << eventTimeDelta << std::endl;
        eventTimeDelta = 0.;
    }

    DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
    Vector2i resolution = ds->getDisplayConfig().getCanvasRect().size();

    float curX = currentEvent->getPosition().x() / resolution[0];
    float curY = currentEvent->getPosition().y() / resolution[1];


    float lastX = lastEvent->getPosition().x() / resolution[0];
    float lastY = lastEvent->getPosition().y() / resolution[1];


    float myPointerAxisMultiplier = 100.0;

    float dx = (curX - lastX) * myPointerAxisMultiplier;
    float dy = -(curY - lastY) * myPointerAxisMultiplier;


    // return if there is no movement.
    if( dx == 0. && dy == 0. )
        return false;


    // call appropriate methods
    if( currentEvent->isFlagSet(Event::Right))
    {
        return performMovementLeftMouseButton( eventTimeDelta, dx, dy );
    }
    else if( currentEvent->isFlagSet(Event::Middle) )
    {
        return performMovementMiddleMouseButton( eventTimeDelta, dx, dy );
    }
    else if( currentEvent->isFlagSet(Event::Left) )
    {
        return performMovementRightMouseButton( eventTimeDelta, dx, dy );
    }
}


void NodeTrackerManipulator::handleMouseMove(Event *event)
{
    // std::cout << "handling mouse move" << std::endl;
}

void NodeTrackerManipulator::handleMouseDrag(Event* event){
    addMouseEvent(event);
    handleMouse();

    //     osg::Vec3d eye;
    // osg::Quat rotation;

    // getTransformation( eye, rotation );
    
    // std::cout << eye.x() << " " << eye.y() << " " << eye.z()    << std::endl;


}



class CopyCamCallback : public osg::NodeCallback
{
public:
    CopyCamCallback(NodeTrackerManipulator* manipulator, Camera* cam) : _manipulator(manipulator), _omegaCam(cam) {}
    
    virtual void operator()( osg::Node* node, osg::NodeVisitor* nv )
    {
        osg::Vec3d eye;
        osg::Vec3d center;
        osg::Vec3d up;

        /** Gets manipulator's focal center, eye position, and up vector.*/
        _manipulator->getTransformation(eye, center, up);

        Vector3f oPosVec(eye.x(), eye.y(), eye.z());
        Vector3f oUpVec(up.x(), up.y(), up.z());
        Vector3f oCenterVec(center.x(), center.y(), center.z());
        
        std::cout << eye.x() << " " << eye.y() << " " << eye.z() <<std::endl;
        std::cout << center.x() << " " << center.y() << " " << center.z() << std::endl << std::endl;
        _omegaCam->lookAt(oCenterVec, oUpVec);
        _omegaCam->setPosition(oPosVec);
        
    }
    
protected:
    osgGA::NodeTrackerManipulator* _manipulator;
    Camera* _omegaCam;
};



CameraManipulator::CameraManipulator() {
    myManipulator = new NodeTrackerManipulator;

    myManipulator->setTrackerMode( osgGA::NodeTrackerManipulator::NODE_CENTER_AND_ROTATION );
    myManipulator->setRotationMode( osgGA::NodeTrackerManipulator::TRACKBALL );


}

// CameraManipulator::setHomePosition(Vector3f pos, )



void CameraManipulator::onEvent(Event* event)
{
    myManipulator->handle(event);
}


// void CameraManipulator::setHomePosition(float px, float py, float pz, )

void CameraManipulator::setTrackedNode(Entity* entity) {
    myTrackedNode = entity->getOsgNode();
    myManipulator->setTrackNode( myTrackedNode.get() );

    Camera* omegaCam = Engine::instance()->getDefaultCamera();


    // myManipulator->setHomePosition( OOSG_VEC3( omegaCam->getPosition() ) , osg::Vec3(), osg::Z_AXIS );


    osg::BoundingSphere boundingSphere;
    osg::ComputeBoundsVisitor cbVisitor;
    myTrackedNode->accept(cbVisitor);
    osg::BoundingBox &bb = cbVisitor.getBoundingBox();

    if (bb.valid()) boundingSphere.expandBy(bb);
    else boundingSphere = myTrackedNode->getBound();



    myManipulator->setHomePosition( boundingSphere.center() + osg::Vec3(0, 20.0, 0), //OOSG_VEC3( omegaCam->getPosition() ),
                    boundingSphere.center(),
                    osg::Z_AXIS);
    

    osg::ref_ptr<CopyCamCallback> copyCam = new CopyCamCallback(myManipulator, omegaCam);
    entity->getOsgNode()->addUpdateCallback(copyCam);
}


CameraManipulator* CameraManipulator::create(){
    return new CameraManipulator();
}