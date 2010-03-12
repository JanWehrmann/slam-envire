#ifndef __ENVIRE_POINTCLOUD_HPP__
#define __ENVIRE_POINTCLOUD_HPP__

#include "Core.hpp"
#include <Eigen/Core>

namespace envire {
    class Pointcloud : public CartesianMap 
    {
    public:
	/** definition of 3d points
	 */
	std::vector<Eigen::Vector3d> vertices;

    public:
	static const std::string className;

	EIGEN_MAKE_ALIGNED_OPERATOR_NEW

	Pointcloud();
	~Pointcloud();

	Pointcloud(Serialization& so);
	void serialize(Serialization& so);

	void writeMap(const std::string& path);
	void readMap(const std::string& path);

	const std::string& getClassName() const {return className;};

	Pointcloud* clone();
    };
}

#endif
