//============================================================================
// Copyright 2015 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//============================================================================

#include "VModelData.hpp"

#include "VFilter.hpp"
#include "ServerHandler.hpp"
#include "VAttribute.hpp"
#include "VNode.hpp"

#include <QMetaMethod>

//==========================================
//
// VModelServer
//
//==========================================

//It takes ownership of the filter

VModelServer::VModelServer(ServerHandler *server) :
   server_(server),
   filter_(0)
{
	//We has to observe the nodes of the server.
	server_->addNodeObserver(this);

	//We has to observe the nodes of the server.
	server_->addServerObserver(this);

}

VModelServer::~VModelServer()
{
	server_->removeNodeObserver(this);
	server_->removeServerObserver(this);

	if(filter_)
		delete filter_;
}

VNode* VModelServer::topLevelNode(int row) const
{
  return server_->vRoot()->childAt(row);
}

int VModelServer::indexOfTopLevelNode(const VNode* node) const
{
  return server_->vRoot()->indexOfChild(node);
}

int VModelServer::topLevelNodeNum() const
{
  return server_->vRoot()->numOfChildren();
}

int VModelServer::totalNodeNum() const
{
  return server_->vRoot()->totalNum();
}

void VModelServer::runFilter()
{
	if(filter_)
	{
		filter_->beginReset(server_);
		filter_->endReset();
	}
}

//==========================================
//
// VTreeServer
//
//==========================================

//It takes ownership of the filter

VTreeServer::VTreeServer(ServerHandler *server,NodeFilterDef* filterDef) :
   VModelServer(server)
{
	filter_=new TreeNodeFilter(filterDef);

	//We has to observe the nodes of the server.
	//server_->addNodeObserver(this);
}

VTreeServer::~VTreeServer()
{
}

int VTreeServer::checkAttributeUpdateDiff(VNode *node)
{
	//int last=node->cachedAttrNum();
	//int current=node->attrNum();
	//return current-last;
	return 0;
}

//--------------------------------------------------
// ServerObserver methods
//--------------------------------------------------

void VTreeServer::notifyDefsChanged(ServerHandler* server, const std::vector<ecf::Aspect::Type>& a)
{
	Q_EMIT dataChanged(this);
}

void VTreeServer::notifyBeginServerScan(ServerHandler* server,const VServerChange& change)
{
	Q_EMIT beginServerScan(this,change.suiteNum_);
}

void VTreeServer::notifyEndServerScan(ServerHandler* server)
{
	Q_EMIT endServerScan(this,server->vRoot()->numOfChildren());

	if(filter_)
	{
		/*if(filter_->update(node->node()))
		{
			Q_EMIT dataChanged(this);
		}*/
	}
}

void VTreeServer::notifyBeginServerClear(ServerHandler* server)
{
	Q_EMIT beginServerClear(this,-1);
}

void VTreeServer::notifyEndServerClear(ServerHandler* server)
{
	Q_EMIT endServerClear(this,-1);

	if(filter_)
	{
		/*if(filter_->update(node->node()))
		{
			Q_EMIT dataChanged(this);
		}*/
	}
}

void VTreeServer::notifyServerConnectState(ServerHandler* server)
{
	Q_EMIT rerender();
}

void VTreeServer::notifyServerActivityChanged(ServerHandler* server)
{
	Q_EMIT dataChanged(this);
}

//--------------------------------------------------
// NodeObserver methods
//--------------------------------------------------

void VTreeServer::notifyBeginNodeChange(const VNode* node, const std::vector<ecf::Aspect::Type>& aspect, const VNodeChange& change)
{
	if(node==NULL)
		return;

	bool runFilter=false;
	bool attrNumCh=(std::find(aspect.begin(),aspect.end(),ecf::Aspect::ADD_REMOVE_ATTR) != aspect.end());
	bool nodeNumCh=(std::find(aspect.begin(),aspect.end(),ecf::Aspect::ADD_REMOVE_NODE) != aspect.end());

	//-----------------------------------------------------------------------
	// The number of attributes changed but the number of nodes is the same!
	//-----------------------------------------------------------------------
	if(attrNumCh && !nodeNumCh)
	{
		if(change.cachedAttrNum_ != change.attrNum_)
		{
			Q_EMIT beginAddRemoveAttributes(this,node,
					change.attrNum_,change.cachedAttrNum_);
		}
	}

	//----------------------------------------------------------------------
	// The number of nodes changed but number of attributes is the same!
	//----------------------------------------------------------------------
	else if(!attrNumCh && nodeNumCh)
	{
		//Only a continuous block of nodes was added and the order of the nodes is the same
		if(change.nodeAddAt_ != -1)
		{
			int diff=change.nodeNum_-change.cachedNodeNum_;
			assert(diff >0);
			Q_EMIT beginAddRemoveNode(this,node,change.nodeAddAt_,diff);
		}
		//Only a continuous block of nodes was removed and the order of the nodes is the same
		else if(change.nodeRemoveAt_ != -1)
		{
			int diff=change.nodeNum_-change.cachedNodeNum_;
			assert(diff <0);
			Q_EMIT beginAddRemoveNode(this,node,change.nodeRemoveAt_,diff);
		}
		else
		{
			Q_EMIT resetBranch(this,node);
		}

		runFilter=true;
	}

	//---------------------------------------------------------------------
	// Both the number of nodes and the number of attributes changed!
	//---------------------------------------------------------------------
	else if(attrNumCh && nodeNumCh)
	{
		//This can never happen
		assert(0);
	}

	//---------------------------------------------------------------------
	// The number of attributes and nodes did not change
	//---------------------------------------------------------------------
	else
	{
		//Check the aspects
		for(std::vector<ecf::Aspect::Type>::const_iterator it=aspect.begin(); it != aspect.end(); ++it)
		{
			//Changes in the nodes
			if(*it == ecf::Aspect::STATE || *it == ecf::Aspect::DEFSTATUS ||
			   *it == ecf::Aspect::SUSPENDED)
			{
				Q_EMIT nodeChanged(this,node);
				runFilter=true;
			}

			//Changes might affect the icons
			else if (*it == ecf::Aspect::FLAG || *it == ecf::Aspect::SUBMITTABLE ||
					*it == ecf::Aspect::TODAY || *it == ecf::Aspect::TIME ||
					*it == ecf::Aspect::DAY || *it == ecf::Aspect::CRON || *it == ecf::Aspect::DATE)
			{
				Q_EMIT nodeChanged(this,node);
			}

			//Changes in the attributes
			if(*it == ecf::Aspect::METER ||  *it == ecf::Aspect::EVENT ||
			   *it == ecf::Aspect::LABEL || *it == ecf::Aspect::LIMIT ||
			   *it == ecf::Aspect::EXPR_TRIGGER || *it == ecf::Aspect::EXPR_COMPLETE ||
			   *it == ecf::Aspect::REPEAT || *it == ecf::Aspect::NODE_VARIABLE ||
			   *it == ecf::Aspect::LATE || *it == ecf::Aspect::TODAY || *it == ecf::Aspect::TIME ||
			   *it == ecf::Aspect::DAY || *it == ecf::Aspect::CRON || *it == ecf::Aspect::DATE )
			{
				Q_EMIT attributesChanged(this,node);
			}
		}
	}

	//We need to check if it changes the filter state
	if(runFilter && filter_)
	{
		/*if(filter_->update(node->node()))
		{
			Q_EMIT dataChanged(this);
		}*/
	}
}

void VTreeServer::notifyEndNodeChange(const VNode* node, const std::vector<ecf::Aspect::Type>& aspect, const VNodeChange& change)
{
	if(node==NULL)
		return;

	bool runFilter=false;
	bool attrNumCh=(std::find(aspect.begin(),aspect.end(),ecf::Aspect::ADD_REMOVE_ATTR) != aspect.end());
	bool nodeNumCh=(std::find(aspect.begin(),aspect.end(),ecf::Aspect::ADD_REMOVE_NODE) != aspect.end());

	//-----------------------------------------------------------------------
	// The number of attributes changed but the number of nodes is the same!
	//-----------------------------------------------------------------------
	if(attrNumCh && !nodeNumCh)
	{
		if(change.cachedAttrNum_ != change.attrNum_)
		{
			Q_EMIT endAddRemoveAttributes(this,node,
					change.attrNum_,change.cachedAttrNum_);
		}
	}

	//----------------------------------------------------------------------
	// The number of nodes changed but number of attributes is the same!
	//----------------------------------------------------------------------
	else if(!attrNumCh && nodeNumCh)
	{
		//Only one node was added and the order of the nodes is the same
		if(change.nodeAddAt_ != -1)
		{
			Q_EMIT endAddRemoveNode(this,node,true);
		}
		else if(change.nodeRemoveAt_ != -1)
		{
			Q_EMIT endAddRemoveNode(this,node,false);
		}
		else
		{
			//Q_EMIT resetBranch(this,node);
		}
	}

}

void VTreeServer::notifyBeginNodeClear(const VNode* node)
{
	Q_EMIT beginNodeClear(this,node);
}

void VTreeServer::notifyEndNodeClear(const VNode* node)
{
	Q_EMIT endNodeClear();
}

void VTreeServer::notifyBeginNodeScan(const VNode* node,const VNodeChange& change)
{
	Q_EMIT beginNodeScan(this,node,change.nodeNum_);
}

void VTreeServer::notifyEndNodeScan(const VNode* node)
{
	Q_EMIT endNodeScan(this,node);
}

//==========================================
//
// VTableServer
//
//==========================================

//It takes ownership of the filter

VTableServer::VTableServer(ServerHandler *server,NodeFilterDef* filterDef) :
	VModelServer(server)

{
	filter_=new TableNodeFilter(filterDef);

	//We has to observe the nodes of the server.
	//server_->addNodeObserver(this);
}

VTableServer::~VTableServer()
{
}

//--------------------------------------------------
// ServerObserver methods
//--------------------------------------------------


void VTableServer::notifyBeginServerScan(ServerHandler* server,const VServerChange& change)
{
	//At this point we do not know how many nodes we will have in the filter!
}

void VTableServer::notifyEndServerScan(ServerHandler* server)
{
	//Update the filter using the current server state
	filter_->beginReset(server);

	int realCount=filter_->realMatchCount();

	//The filter pretends it is empty
	//Tell the model to add realCount number of rows!!
	Q_EMIT beginServerScan(this,realCount);

	filter_->endReset();

	Q_EMIT endServerScan(this,realCount);
}

void VTableServer::notifyBeginServerClear(ServerHandler* server)
{
	int n=filter_->matchCount();
	Q_EMIT beginServerClear(this,n);
}

void VTableServer::notifyEndServerClear(ServerHandler* server)
{
	int n=filter_->matchCount();
	filter_->clear();
	Q_EMIT endServerClear(this,n);
}

void VTableServer::notifyServerConnectState(ServerHandler* server)
{
	Q_EMIT rerender();
}

void VTableServer::notifyServerActivityChanged(ServerHandler* server)
{
	Q_EMIT dataChanged(this);
}

void VTableServer::notifyBeginNodeChange(const VNode* node, const std::vector<ecf::Aspect::Type>& types,const VNodeChange&)
{
	int n=filter_->matchCount();
	Q_EMIT beginServerClear(this,n);
	filter_->clear();
	Q_EMIT endServerClear(this,n);
}

void VTableServer::notifyEndNodeChange(const VNode* node, const std::vector<ecf::Aspect::Type>& types,const VNodeChange&)
{
	//Update the filter using the current server state
	filter_->beginReset(server_);
	int realCount=filter_->matchCount();

	//Tell the model to add realCount number of rows!!
	Q_EMIT beginServerScan(this,realCount);

	filter_->endReset();

	Q_EMIT endServerScan(this,realCount);

}

//==========================================
//
// VModelData
//
//==========================================

VModelData::VModelData(ServerFilter* serverFilter,NodeFilterDef *filterDef,ModelType modelType) :
		modelType_(modelType),
		serverFilter_(serverFilter),
		filterDef_(filterDef)
{
	connect(filterDef_,SIGNAL(changed()),
			this,SLOT(slotFilterDefChanged()));

	init();
}


VModelData::~VModelData()
{
	clear();
}

//Completely clear the data and rebuild everything with a new
//ServerFilter.
void VModelData::reset(ServerFilter* serverFilter)
{
	clear();
	serverFilter_=serverFilter;

	init();
}

void VModelData::init()
{
	serverFilter_->addObserver(this);

	for(unsigned int i=0; i < serverFilter_->items().size(); i++)
	{
		if(ServerHandler *server=serverFilter_->items().at(i)->serverHandler())
		{
			add(server);
		}
	}
}

void VModelData::clear()
{
	if(serverFilter_)
		serverFilter_->removeObserver(this);

	for(int i=0; i < servers_.size(); i++)
	{
		delete servers_.at(i);
	}

	servers_.clear();
}

void VModelData::add(ServerHandler *server)
{
	VModelServer* d=NULL;

	switch(modelType_)
	{
	case TreeModel:
		d=new VTreeServer(server,filterDef_);
		break;
	case TableModel:
		d=new VTableServer(server,filterDef_);
		break;
	default:
		return; //TODO: give an error message!!
	}

	//We need to relay all the possible signals from VModelServer

	//Loop over the methods of the VModelServer
	for(int i = 0; i < d->staticMetaObject.methodCount(); i++)
	{
		//Get the method signature
		const char* sg=d->staticMetaObject.method(i).signature();

		//Check if it is a signal
		if(d->staticMetaObject.indexOfSignal(sg) != -1)
		{
			//Check if it is a signal in the current class as well
			int idx=staticMetaObject.indexOfSignal(sg);
			if(idx != -1)
			{
				//We simply relay this signal
				connect(d,d->staticMetaObject.method(i),
						this,staticMetaObject.method(idx));
			}
		}
	}

	servers_.push_back(d);
}

VModelServer* VModelData::server(int n) const
{
	return (n >=0 && n < servers_.size())?servers_.at(n):0;
}

ServerHandler* VModelData::realServer(int n) const
{
	return (n >=0 && n < servers_.size())?servers_.at(n)->server_:0;
}


int VModelData::indexOfServer(void* idPointer) const
{
	for(unsigned int i=0; i < servers_.size(); i++)
		if(servers_.at(i) == idPointer)
			return i;

	return -1;
}

int VModelData::indexOfServer(ServerHandler* s) const
{
	for(unsigned int i=0; i < servers_.size(); i++)
		if(servers_.at(i)->server_ == s)
			return i;
	return -1;
}

//This has to be very fast!!!
bool VModelData::isFiltered(VNode *node) const
{
	return true;

	//TODO: make it work again

	/*ServerHandler* server=node->server();
	int id=indexOfServer(server);
	if(id != -1 && servers_.at(id)->filter_)
	{
		return servers_.at(id)->filter_->isFiltered(node);
	}

	return true;*/
}

bool VModelData::identifyInFilter(VModelServer* server,int& start,int& count,VNode** node)
{
	start=-1;
	count=-1;
	*node=NULL;

	int totalRow=0;
	if(server)
	{
		*node=server->filter_->realMatchAt(0);
		if(*node)
		{
			count=server->filter_->realMatchCount();
			start=0;
			for(unsigned int i=0; i < servers_.size(); i++)
			{
				if(servers_.at(i) == server)
				{
					return true;
				}
				start+=servers_.at(i)->filter_->matchCount();
			}
		}
	}

	return false;
}

//This has to be very fast!!!
int VModelData::posInFilter(VModelServer* server,const VNode *node) const
{
	int totalRow=0;
	if(server)
	{
		for(unsigned int i=0; i < servers_.size(); i++)
		{
			NodeFilter *filter=servers_.at(i)->filter_;
			if(servers_.at(i) == server)
			{
				int pos=filter->matchPos(node);
				if(pos != -1)
				{
					totalRow+=filter->matchPos(node);
					return totalRow;
				}
				else
					return -1;
			}
			else
			{
				totalRow+=filter->matchCount();
			}
		}
	}

	return -1;
}

//This has to be very fast!!!
int VModelData::posInFilter(const VNode *node) const
{
	int serverIdx=indexOfServer(node->server());
	if(serverIdx != -1)
	{
		return posInFilter(servers_.at(serverIdx),node);
	}

	return -1;
}


int VModelData::numOfNodes(int index) const
{
	if(VModelServer *d=server(index))
	{
		return d->totalNodeNum();
	}
	return 0;
}

VNode* VModelData::getNodeFromFilter(int totalRow)
{
	int cnt=0;

	if(totalRow < 0)
		return NULL;

	for(unsigned int i=0; i < servers_.size(); i++)
	{
		NodeFilter *filter=servers_.at(i)->filter_;
		int pos=totalRow-cnt;
		if(pos < filter->matchCount())
		{
			return filter->matchAt(pos);
		}
		cnt+=filter->matchCount();
	}

	return NULL;
}

//Return the number of filtered nodes for the given server
int VModelData::numOfFiltered(int index) const
{
	if(VModelServer *d=server(index))
	{
		return d->filter_->matchCount();

	}
	return 0;
}


void VModelData::runFilter(bool broadcast)
{
	for(unsigned int i=0; i < servers_.size(); i++)
	{
		servers_.at(i)->runFilter();
	}

	if(broadcast)
	{
		Q_EMIT filterChanged();
	}
}

VNode* VModelData::topLevelNode(void* ptrToServer,int row)
{
	for(unsigned int i=0; i < servers_.size(); i++)
	{
		if(servers_.at(i) == ptrToServer)
		{
			return servers_.at(i)->topLevelNode(row);
		}
	}
	return NULL;
}

bool VModelData::identifyTopLevelNode(const VNode* node,VModelServer** server,int& row)
{
	for(unsigned int i=0; i < servers_.size(); i++)
	{
		row=servers_.at(i)->indexOfTopLevelNode(node);
		if(row != -1)
		{
			*server=servers_.at(i);
			return true;
		}
	}

	return false;
}

//ServerFilter observer methods

void VModelData::notifyServerFilterAdded(ServerItem* item)
{
	if(!item)
		return;

	if(ServerHandler *server=item->serverHandler())
	{
		//Notifies the model that a change will happen
		Q_EMIT serverAddBegin(count());

		add(server);

		//Notifies the model that the change has finished
		Q_EMIT serverAddEnd();
		return;
	}
}

void VModelData::notifyServerFilterRemoved(ServerItem* item)
{
	if(!item)
		return;

	int i=0;
	for(std::vector<VModelServer*>::iterator it=servers_.begin(); it!= servers_.end(); ++it)
	{
		if((*it)->server_ == item->serverHandler())
		{
			//Notifies the model that a change will happen
			Q_EMIT serverRemoveBegin(i);

			delete *it;
			servers_.erase(it);

			//Notifies the model that the change has finished
			Q_EMIT serverRemoveEnd();
			return;
		}
		i++;
	}
}

void VModelData::notifyServerFilterChanged(ServerItem* item)
{
	//Q_EMIT dataChanged();
}

void VModelData::slotFilterDefChanged()
{
	runFilter(true);
}
