#ifndef __ENVIRE_ENVIRONMENT__
#define __ENVIRE_ENVIRONMENT__

#include <envire/core/EventSource.hpp>
#include <envire/core/Event.hpp>
#include <envire/core/Transform.hpp>
#include "EnvironmentItem.hpp"

namespace envire
{
    class Environment;
    class Serialization;
    class FrameNode;
    class Layer;
    class Operator;
    class CartesianMap;
    class EventHandler;
    class SynchronizationEventQueue;
    class Event;
    class SerializationFactory;
    
    /** The environment class manages EnvironmentItem objects and has ownership
     * of these.  all dependencies between the objects are handled in the
     * environment class, and convenience methods of the individual objects are
     * available to simplify usage.
     */
    class Environment 
    {
	friend class FileSerialization;
	friend class GraphViz;

	/** we track the last id given to an item, for assigning new id's.
	 */
        long last_id;
    public:
	static const std::string ITEM_NOT_ATTACHED;

    protected:
	typedef std::map<std::string, EnvironmentItem::Ptr > itemListType;
	typedef std::map<FrameNode*, FrameNode*> frameNodeTreeType;
	typedef std::multimap<Layer*, Layer*> layerTreeType;
	typedef std::multimap<Operator*, Layer*> operatorGraphType;
	typedef std::map<CartesianMap*, FrameNode*> cartesianMapGraphType;
	
	itemListType items;
	frameNodeTreeType frameNodeTree;
	layerTreeType layerTree;
	operatorGraphType operatorGraphInput;
	operatorGraphType operatorGraphOutput;
	cartesianMapGraphType cartesianMapGraph;

        // handler to keep track of all changes to synchronize this environment
        // with other environments
        SynchronizationEventQueue *synchronizationEventQueue;
	
	FrameNode* rootNode;
        std::string envPrefix;

	EventSource eventHandlers;
	void publishChilds(EventHandler* handler, FrameNode *parent);
	void detachChilds(FrameNode *parent, EventHandler* handler);

    public:
        Environment();
	virtual ~Environment();
        
	/** attaches an EnvironmentItem and puts it under the control of the Environment.
	 * Object ownership is passed to the environment in this way.
	 */
	void attachItem(EnvironmentItem* item);

        /** Attaches a cartesian map to this environment. If the map does not
         * yet have a frame node, and none is given in this call, it is
         * automatically attached to the root node
         */
        void attachItem(CartesianMap* item, FrameNode* node = 0);

	/** detaches an object from the environment. After this, the object is no longer owned by
	 * by the environment. All links to this object from other objects in the environment are
	 * removed.
	 *
	 * If the deep param is left to false, this may lead to orphans, since
	 * a FrameNode may end up without a parent. Setting deep to true will
	 * remove all child nodes, as well as associated maps.
	 */
	EnvironmentItem::Ptr detachItem(EnvironmentItem* item, bool deep = false );
	
	/**
	* This method will be called by any EnvironmentItem, which was
	* modified. Calling this method will invoke all listeners 
	* attached to the environment and call their itemModified
	* method.
	**/
	void itemModified(EnvironmentItem* item);
	
	EnvironmentItem::Ptr getItem( const std::string& uniqueId ) const
	{
            itemListType::const_iterator it = items.find(uniqueId);
            if (it == items.end())
                return 0;
            else
                return it->second;
        }

        /** Returns the only item of the given type.
         *
         * This is a convenience method to search for an item (usually, a map)
         * in environments where it is known to be the only item of that type
         *
         * Throws std::runtime_error if there is not exactly one match (i.e.
         * either more than one or none)
         */
	template <class T>
	boost::intrusive_ptr<T> getItem() const
	{
            boost::intrusive_ptr<T> result;
            for (itemListType::const_iterator it = items.begin(); it != items.end(); ++it)
            {
                boost::intrusive_ptr<T> map = boost::dynamic_pointer_cast<T>(it->second);
                if (map)
                {
                    if (result)
                        throw std::runtime_error("multiple maps in this environment are of the specified type");
                    else
                        result = map;
                }
            }
            if (!result)
                throw std::runtime_error("no maps in this environment are of the specified type");
            return result;
        }

	template <class T>
	boost::intrusive_ptr<T> getItem( const std::string& uniqueId ) const
	{
            itemListType::const_iterator it = items.find(uniqueId);
            if (it == items.end())
                return 0;
            else
                return boost::dynamic_pointer_cast<T>(it->second);
        }

	void addChild(FrameNode* parent, FrameNode* child);
	void addChild(Layer* parent, Layer* child);

	void removeChild(FrameNode* parent, FrameNode* child);
	void removeChild(Layer* parent, Layer* child);

	FrameNode* getParent(FrameNode* node);
	std::list<Layer*> getParents(Layer* layer);

	FrameNode* getRootNode();
	std::list<FrameNode*> getChildren(FrameNode* parent);
	std::list<Layer*> getChildren(Layer* parent);
	std::list<const Layer*> getChildren(const Layer* parent) const;

	void setFrameNode(CartesianMap* map, FrameNode* node);
	void detachFrameNode(CartesianMap* map, FrameNode* node);
	
	FrameNode* getFrameNode(CartesianMap* map);
	std::list<CartesianMap*> getMaps(FrameNode* node);
	
	bool addInput(Operator* op, Layer* input);
	bool addOutput(Operator* op, Layer* output);

	bool removeInput(Operator* op, Layer* input);
	bool removeOutput(Operator* op, Layer* output);

	bool removeInputs(Operator* op);
	bool removeOutputs(Operator* op);

	std::list<Layer*> getInputs(Operator* op);
        
        /** Returns the only layer that is an input of type LayerT for the given
         * operator
         *
         * LayerT must be a pointer-to-map type:
         * 
         * <code>
         * MLSGrid* mls = env->getInput< MLSGrid* >(mls_slope_operator);
         * </code>
         *
         * If multiple matching layers are found, an exception is raised
         */
        template<typename LayerT>
        LayerT getInput(Operator* op)
        {
            std::list<Layer*> inputs = getInputs(op);
            LayerT result = 0;
            for (std::list<Layer*>::iterator it = inputs.begin(); it != inputs.end(); ++it)
            {
                LayerT layer = dynamic_cast<LayerT>(*it);
                if (layer)
                {
                    if (result)
                        throw std::runtime_error("more than one input layer with the required type found");
                    else
                        result = layer;
                }
            }
            if (!result)
                throw std::runtime_error("cannot find an input layer with the required type");
            return result;
        }

	std::list<Layer*> getOutputs(Operator* op);

        /** Returns the only layer that is an output of type T for the given
         * operator
         *
         * LayerT must be a pointer-to-map type:
         * 
         * <code>
         * Grid<double>* slopes = env->getOutput< Grid<double>* >(mls_slope_operator);
         * </code>
         *
         * If multiple matching layers are found, an exception is raised
         */
        template<typename LayerT>
        LayerT getOutput(Operator* op)
        {
            std::list<Layer*> outputs = getOutputs(op);
            LayerT result = 0;
            for (std::list<Layer*>::iterator it = outputs.begin(); it != outputs.end(); ++it)
            {
                LayerT layer = dynamic_cast<LayerT>(*it);
                if (layer)
                {
                    if (result)
                        throw std::runtime_error("more than one output layer with the required type found");
                    else
                        result = layer;
                }
            }
            if (!result)
                throw std::runtime_error("cannot find an output layer with the required type");
            return result;
        }

        /** Returns the operator that has \c output in its output, or NULL if
         * none exist.
         *
         * Note that a layer can be output of a single operator only
         */
	Operator* getGenerator(Layer* output);

        /** Returns the layers that are generated from \c input
         *
         * In practice, it returns the output layers of all the operators for
         * which \c input is listed as input
         */
        std::list<Layer*> getLayersGeneratedFrom(Layer* input);

        /** Returns the layers of type T that are generated from \c input
         *
         * T must be a pointer on a map:
         *
         * <code>
         * env->getGeneratedFrom<Grid<double>*>(mls);
         * </code>
         */
        template<typename T>
        std::list<T> getGeneratedFrom(Layer* input)
        {
            std::list<Layer*> all = getLayersGeneratedFrom(input);
            std::list<T> result;
            for (std::list<Layer*>::const_iterator it = all.begin();
                    it != all.end(); ++it)
            {
                T as_T = dynamic_cast<T>(*it);
                if (as_T) result.push_back(as_T);
            }
            return result;
        }

	void updateOperators();

        /** Serializes this environment to the given directory */
        void serialize(std::string const& path);

        /** Loads the environment from the given directory and returns it */
        static Environment* unserialize(std::string const& path);

	/**
	 * Adds an eventHandler that gets called whenever there
	 * are modifications to the evironment.
	 * On registration, all content of the environment is put to the
	 * handler as if it was being generated.
	 */
	void addEventHandler(EventHandler *handler);

	/**
	 * Remove the given eventHandler from the environment.
	 * The handler will receive events as if the Environment was destroyed.
	 */
	void removeEventHandler(EventHandler *handler);

	/**
	 * will pass the @param event to the registered event handlers
	 */
	void handle( const Event& event );

	/**
	 * returns all items of a particular type
	 */
	template <class T>
	    std::vector<T*> getItems()
	{
	    std::vector<T*> result;
	    for(itemListType::iterator it=items.begin();it != items.end(); ++it )
	    {
		// TODO this is not very efficient...
		T* item = dynamic_cast<T*>( it->second.get() );
		if( item )
		    result.push_back( item );
	    }
	    return result;
	}

        //convenience function to create EnvironmentItems which are automatically attached
        //to the environment 
        template<class T>
        T* create()
        {
            T* p = new T;
            attachItem(p);
            return p;
        }

        /** 
	 * Returns the transformation from the frame represented by @a from to
         * the frame represented by @a to. This always defines an unique
         * transformation, as the frames are sorted in a tree.
	 *
	 * relativeTransform( child, child->getParent() ) is equivalent to
	 * child->getTransform().
         */
	Transform relativeTransform(const FrameNode* from, const FrameNode* to);

        /** 
	 * @overload
         *
         * Returns the relative transformation between the frames of two
         * cartesian maps
         */
	Transform relativeTransform(const CartesianMap* from, const CartesianMap* to);

	/** @return a new transform object, that specifies the transformation
	 * from the @param from frame to the @param to frame, and will take care of
	 * uncertainty (linearised) on the way.
	 */
	TransformWithUncertainty relativeTransformWithUncertainty(const FrameNode* from, const FrameNode* to);

        /** 
	 * @overload
         *
         * Returns the relative transformation, including uncertainty, between
         * the frames of two cartesian maps, 
         */
	TransformWithUncertainty relativeTransformWithUncertainty(const CartesianMap* from, const CartesianMap* to);
        
        /** Sets the prefix for ID generation for this environment
         *
         * The prefix is normalized to start and end with the '/' separation
         * marker. The prefix is used as a sort of namespace, so that
	 * ids can be kept unique between different environments.
         *
         * The default prefix is /
         */
        void setEnvironmentPrefix(std::string envPrefix);
        
        /** Returns the prefix for ID generation on this environment
         *
         * The default prefix is /
         */
        std::string getEnvironmentPrefix() const { return envPrefix; }

        /** Apply a set of serialized modifications to this environment
         */
        void applyEvents(std::vector<BinaryEvent> const& events);

        /** Pulls all binary events from the environment 
         *  (serialiaze all items as binary events)
         *  If all is set to false only the modifications since the last pull
         *  are regarded
         */
        void pullEvents(std::vector<BinaryEvent> &events,bool all = false);
    };
}

#endif