#include "EnvireVisualization.hpp"
#include <boost/bind.hpp>

#include "LaserScanVisualization.hpp"
#include "FrameNodeVisualization.hpp"
#include "TriMeshVisualization.hpp"
#include "PointcloudVisualization.hpp"
#include "ElevationGridVisualization.hpp"
#include "MLSVisualization.hpp"
#include "ImageRGB24Visualization.hpp"

#include "ItemManipulator.hpp"
#include <vizkit/PickHandler.hpp>

#include <QTreeWidget>
#include <QString>
#include <QAction>

using namespace vizkit;

EnvireVisualization::EnvireVisualization()
    : env( NULL )
{
    ownNode = new osg::Group();
    ownNode->setUpdateCallback( this );

    // setup eventlistener
    eventListener = boost::shared_ptr<EnvireEventListener>(
	    new EnvireEventListener( 
		boost::bind( &osg::Group::addChild, ownNode->asGroup(), _1 ),
		boost::bind( &osg::Group::removeChild, ownNode->asGroup(), _1 ) ) );

    // create and register visualizers
    visualizers.push_back( boost::shared_ptr<LaserScanVisualization>(new LaserScanVisualization() ) );
    visualizers.push_back( boost::shared_ptr<FrameNodeVisualization>(new FrameNodeVisualization() ) );
    visualizers.push_back( boost::shared_ptr<TriMeshVisualization>(new TriMeshVisualization() ) );
    visualizers.push_back( boost::shared_ptr<PointcloudVisualization>(new PointcloudVisualization() ) );
    visualizers.push_back( boost::shared_ptr<ElevationGridVisualization>(new ElevationGridVisualization() ) );
    visualizers.push_back( boost::shared_ptr<MLSVisualization>(new MLSVisualization() ) );
    visualizers.push_back( boost::shared_ptr<ImageRGB24Visualization>(new ImageRGB24Visualization() ) );

    // attach visualizers
    for(std::vector<boost::shared_ptr<EnvironmentItemVisualizer> >::iterator it = visualizers.begin(); it != visualizers.end(); it++)
    {
	eventListener->addVisualizer( (*it).get() );
    }
}

void EnvireVisualization::attachTreeWidget( QTreeWidget *treeWidget )
{
    //create context menu for TreeWidget
    treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);
    QAction* hideItem = new QAction(QString("hide item"), treeWidget);
    QAction* unhideItem = new QAction(QString("unhide item"), treeWidget);
    QAction* removeItem = new QAction(QString("remove item"), treeWidget);
    treeWidget->addAction(hideItem);
    treeWidget->addAction(unhideItem);
    treeWidget->addAction(removeItem);
    
    twl = boost::shared_ptr<TreeViewListener>(new TreeViewListener(treeWidget));
    ItemManipulator *im = new ItemManipulator(eventListener.get(), twl.get());
    QObject::connect(treeWidget, SIGNAL(itemActivated ( QTreeWidgetItem *, int )), im, SLOT(itemActivated ( QTreeWidgetItem *, int)));
    QObject::connect(treeWidget, SIGNAL(itemClicked ( QTreeWidgetItem *, int )), im, SLOT(itemClicked ( QTreeWidgetItem *, int)));
    QObject::connect(hideItem, SIGNAL(triggered()), im, SLOT(hideSelectedItems()));
    QObject::connect(unhideItem, SIGNAL(triggered()), im, SLOT(unhideSelectedItems()));
    QObject::connect(treeWidget, SIGNAL(itemChanged ( QTreeWidgetItem *, int )), im, SLOT(itemChanged ( QTreeWidgetItem *, int)));
    QObject::connect(removeItem, SIGNAL(triggered()), im, SLOT(removeSelectedItems()));
    //view->installEventFilter(im);
}

bool EnvireVisualization::isDirty() const
{
    // TODO fix this
    return true;
    //return eventListener->hasEvents();
}
    
void EnvireVisualization::operatorIntern( osg::Node* node, osg::NodeVisitor* nv )
{
    // this one is called from the osg thread
    // and will open the lock at each frame iteration cycle
    //
    // TODO fix this
    //eventListener->processEvents();
}

void EnvireVisualization::updateDataIntern( envire::Environment* const& data )
{
    // detach old root node
    if( env )
    {
	ownNode->asGroup()->removeChild( eventListener->getNodeForItem( env->getRootNode() ) );
    }

    env = data;
    env->addEventHandler( eventListener.get() );
    if( twl )
	env->addEventHandler( twl.get() );
}
