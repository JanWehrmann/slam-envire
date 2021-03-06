#include <Eigen/Geometry>

#include "envire/Core.hpp"
#include "envire/maps/Pointcloud.hpp"
#include "envire/operators/ScanMeshing.hpp"
#include "envire/operators/SimplifyPointcloud.hpp"
#include "envire/operators/MergePointcloud.hpp"

#include "boost/scoped_ptr.hpp"

#include <iostream>

using namespace envire;
using namespace std;
     
int main( int argc, char* argv[] )
{
    if( argc < 3 ) 
    {
	std::cout << "usage: env_simplify input output [cell_size]" << std::endl;
	exit(0);
    }
    double cell_size = 0.05;
    if( argc >=3 )
	cell_size = boost::lexical_cast<double>( argv[3] );
    std::cout << "using cell size: " << cell_size << std::endl;

    boost::scoped_ptr<Environment> env(Environment::unserialize( argv[1] ));
    
    // update scanmeshing ops to create normals for all cases 
    std::vector<envire::ScanMeshing*> ops = env->getItems<envire::ScanMeshing>();
    for(std::vector<envire::ScanMeshing*>::iterator it=ops.begin();it!=ops.end();it++)
    {
	(*it)->setMaxEdgeLength( 200.0 );
	std::cout << "set maxEdge length" << std::endl;
    }
    env->updateOperators();

    // get all available meshes
    std::vector<envire::Pointcloud*> meshes = env->getItems<envire::Pointcloud>();

    // create global pointcloud
    envire::MergePointcloud *merge = new envire::MergePointcloud();
    env->attachItem( merge );
    envire::Pointcloud *mpc = new envire::Pointcloud();
    env->attachItem( mpc );
    env->setFrameNode( mpc, env->getRootNode() );
    merge->addOutput( mpc );
    merge->setClearOutput( false );

    for(std::vector<envire::Pointcloud*>::iterator it=meshes.begin();it!=meshes.end();it++)
    {
	std::cout << "adding trimesh to merge " << (*it)->getUniqueId() << std::endl;
	merge->addInput( *it );
	merge->updateAll();

	env->detachItem( *it );
    }

    std::cout << "merged pointcloud with " << mpc->vertices.size() << " points" << std::endl;

    // and simplified pointcloud
    envire::Pointcloud *mpcs = new envire::Pointcloud();
    env->attachItem( mpcs );
    env->setFrameNode( mpcs, env->getRootNode() );

    envire::SimplifyPointcloud *simplify = new envire::SimplifyPointcloud();
    env->attachItem( simplify );
    simplify->addInput( mpc );
    simplify->addOutput( mpcs );
    simplify->setSimplifyCellSize( cell_size );

    simplify->updateAll();
    env->detachItem( mpc );
    env->detachItem( simplify );

    std::cout << "simplified pointcloud to " << mpcs->vertices.size() << " points" << std::endl;

    std::string path(argv[2]);
    env->serialize( path );

} 

