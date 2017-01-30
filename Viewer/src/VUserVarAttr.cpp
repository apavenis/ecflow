//============================================================================
// Copyright 2017 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//
//============================================================================

#include "VUserVarAttr.hpp"
#include "VAttributeType.hpp"
#include "VNode.hpp"

#include "NodeAttr.hpp"

//================================
// VUserVarAttrType
//================================

class VUserVarAttrType : public VAttributeType
{
public:
    explicit VUserVarAttrType();
    QString toolTip(QStringList d) const;
    void encode(const Variable&,QStringList&) const;

private:
    enum DataIndex {TypeIndex=0,NameIndex=1,ValueIndex=2};
};


VUserVarAttrType::VUserVarAttrType() : VAttributeType("var")
{
    dataCount_=3;
    searchKeyToData_["var_name"]=NameIndex;
    searchKeyToData_["var_value"]=ValueIndex;
    searchKeyToData_["var_type"]=TypeIndex;
    searchKeyToData_["name"]=NameIndex;
    scanProc_=VUserVarAttr::scan;
}

QString VUserVarAttrType::toolTip(QStringList d) const
{
   return QString();
}

void VUserVarAttrType::encode(const Variable& v,QStringList& data) const
{
    data << qName_ <<
            QString::fromStdString(v.name()) <<
            QString::fromStdString(v.theValue());
}

static VUserVarAttrType atype;

//=====================================================
//
// VUserVarAttr
//
//=====================================================

VUserVarAttr::VUserVarAttr(VNode *parent,const Variable& v, int index) : VAttribute(parent,index)
{
    //name_=v.name();
}

VAttributeType* VUserVarAttr::type() const
{
    return &atype;
}

QStringList VUserVarAttr::data() const
{
    QStringList s;

    //Node
    if(parent_->isServer() == 0)
    {
        if(parent_->node_)
        {
            const std::vector<Variable>& v=parent_->node_->variables();
            atype.encode(v[index_],s);
        }
    }
    //Server
    else
    {
        std::vector<Variable> v;
        parent_->variables(v);
        atype.encode(v[index_],s);
    }

    return s;
}

std::string VUserVarAttr::strName() const
{
    //Node
    if(parent_->isServer() == 0)
    {
        if(parent_->node_)
        {
            const std::vector<Variable>& v=parent_->node_->variables();
            return v[index_].name();
        }
    }
    //Server
    else
    {
        std::vector<Variable> v;
        parent_->variables(v);
        return v[index_].name();
    }
    return std::string();
}

void VUserVarAttr::scan(VNode* vnode,std::vector<VAttribute*>& vec)
{
    //Node
    if(vnode->isServer() == 0)
    {
        if(vnode->node_)
        {
            const std::vector<Variable>& v=vnode->node_->variables();
            int n=v.size();
            for(size_t i=0; i < n; i++)
            {
                vec.push_back(new VUserVarAttr(vnode,v[i],i));
            }
        }
    }
    //Server
    else
    {
        std::vector<Variable> v;
        vnode->variables(v);
        int n=v.size();
        for(size_t i=0; i < n; i++)
        {
            vec.push_back(new VUserVarAttr(vnode,v[i],i));
        }
    }
}

int VUserVarAttr::totalNum(VNode* vnode)
{
    //Node
    if(vnode->isServer() == 0)
    {
        if(vnode->node_)
        {
            return vnode->node_->variables().size();
        }
    }
    //Server
    else
    {
        std::vector<Variable> v;
        vnode->variables(v);
        return v.size();
    }
    return 0;
}
