#ifndef __ENVIRE_LAYER__
#define __ENVIRE_LAYER__

#include "EnvironmentItem.hpp"
#include "Holder.hpp"

namespace envire
{
    class HolderBase;
    /** The layer is the base object that holds map data. It can be a cartesian
     * map (CartesianMap) or a non-cartesian one (AttributeList, TopologicalMap)
     */
    class Layer : public EnvironmentItem
    {
    protected:
        /** @todo explain immutability for layer */
        bool immutable;

        /** @todo explain dirty for a layer */
        bool dirty; 

        typedef std::map<std::string, HolderBase*> DataMap;

	/** associating key values with metadata stored in holder objects */ 
	DataMap data_map;

    public:
	static const std::string className;

	/** @brief custom copy constructor required because of metadata handling.
	 */
	Layer(const Layer& other);

	/** @brief custom assignment operator requried because of metadata.
	 */
	Layer& operator=(const Layer& other);

	explicit Layer(std::string const& id);
	virtual ~Layer();

	void serialize(Serialization& so);
        void unserialize(Serialization& so);

	virtual const std::string& getClassName() const {return className;};

        /** @return True if this layer cannot be changed by any means */
        bool isImmutable() const;

        /** Marks this frame as being immutable. This cannot be changed
         * afterwards, as some operators will depend on it.
         */
        void setImmutable();

        /** Unsets the dirty flag on this layer
         * @see isDirty
         */
        void resetDirty();

        /** Marks this layer as dirty
         * @see isDirty
         */
        void setDirty();

        /** In case this layer is auto-generated, this flag is true when the
         * layer sources have been updated but this layer has not yet been.
         */
        bool isDirty() const;

        /** Detach this layer from the operator that generates it, and returns
         * true if this operation was a success (not all operators support
         * this). After this method returned true, it is guaranteed that
         * isGenerated() returns false
         */
        bool detachFromOperator();

        /** True if this layer has been generated by an operator */
        bool isGenerated() const;
        
        /** Returns the operator that generated this layer, or raises
         * std::runtime_error if it is not a generated map
         */
        Operator* getGenerator() const;

	/** Recomputes this layer by applying the operator that has already
	 * generated this map. The actual operation will only be called if the
	 * dirty flag is set, so it is optimal to call it whenever an updated
	 * map is needed. After this call, it is guaranteed that isDirty()
	 * returns false.
         */
        void updateFromOperator();
	
	/** Layers can have hierarchical relationships. This function will add
	 * a child layer to this object.
	 * @param child - the child layer to add
	 */
	void addChild(Layer* child);

	/** @return the parent of this layer or NULL if the layer has no parent
	 */
	std::list<Layer*> getParents();

	/**
         * @return for a given path, it will return a suggestion for a filename 
         * to use when making this layer persistant
	 */
	const std::string getMapFileName( const std::string& path, const std::string& className ) const;

        /**
         * Like getMapFileName(), but allows to override the class name (i.e. not
         * use the one from the map's class directly). This is meant to be used
         * for backward compatibility, when map class names change.
         */
	const std::string getMapFileName( const std::string& className ) const;
        
        /** @return a suggestion for a filename to use when making this layer persistant
         */
        const std::string getMapFileName() const;

	/** will return true if an entry for metadata for the given key exists
	 */
	bool hasData(const std::string& type) const;

        /** Will return true if a metadata exists for the given key, and this
         * metadata is of the specified type
         */
        template<typename T>
        bool hasData(const std::string& type) const
        {
            DataMap::const_iterator it = data_map.find(type);
            if (it == data_map.end())
                return false;
            else return it->second->isOfType<T>();
        }

	/** For a given key, return the metadata associated with it. If the data
	 * does not exist, it will be created.
	 * Will throw a runtime error if the datatypes don't match.
	 */
	template <typename T>
	    T& getData(const std::string& type)
	{
	    if( !hasData( type ) )
	    {
		data_map[type] = new Holder<T>;
	    }

	    /*
	    if( typeid(*data_map[type]) != typeid(Holder<T>) )
	    {
		std::cerr 
		    << "type mismatch. type should be " 
		    << typeid(data_map[type]).name() 
		    << " but is " 
		    << typeid(Holder<T>).name()
		    << std::endl;
		throw std::runtime_error("data type mismatch.");
	    }
	    */

	    return data_map[type]->get<T>();
	};
	
	/** 
	* For a given key, return the metadata associated with it. 
	* If the data does not exist, a std::out_of_range is thrown.
	*/
	template <typename T>
	const T& getData(const std::string& type) const
	{
	    std::map <std::string, envire::HolderBase* >::const_iterator it = data_map.find(type);
	    if(it == data_map.end())
		throw std::runtime_error("No metadata with name " + type + " available ");
	    
	    return it->second->get<T>();
	    /*
	    if( typeid(*data_map[type]) != typeid(Holder<T>) )
	    {
		std::cerr 
		    << "type mismatch. type should be " 
		    << typeid(data_map[type]).name() 
		    << " but is " 
		    << typeid(Holder<T>).name()
		    << std::endl;
		throw std::runtime_error("data type mismatch.");
	    }
	    */
	};

	/** @brief remove metadata with the specified identifier
	 */
	void removeData(const std::string& type);

	/** @brief remove all metadata associated with this object
	 */
	void removeData();
    };
}

#endif
