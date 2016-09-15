//============================================================================
// Copyright 2016 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//============================================================================

#include "TriggerCollector.hpp"

#include "UserMessage.hpp"
#include "VItem.hpp"
#include "VItemPathParser.hpp"
#include "VNode.hpp"

#define _UI_TRIGGERCOLLECTOR_DEBUG

class TriggerListItem
{
public:
    TriggerListItem(VItem* t,VItem* dep,TriggerCollector::Mode mode) :
        t_(t), dep_(dep), mode_(mode) {}

    ~TriggerListItem() {if(t_->isAttribute()) delete t_;}

    VItem* t_; //trigger or triggered
    VItem* dep_;
    TriggerCollector::Mode mode_;
};

TriggerListCollector::~TriggerListCollector()
{
    for(size_t i=0; i < items_.size(); i++)
    {
        delete items_[i];
    }
}


void TriggerListCollector::add(VItem* t, VItem* dep,Mode mode,VItem* t1)
{
    TriggerListItem *item=new TriggerListItem(t,dep,mode) ;
    items_.push_back(item);

    if(dep)
    {
        UserMessage::debug(" dep=" + dep->typeName() + " " +  dep->strName());
        UserMessage::debug("    =" + item->dep_->typeName() + " " +  item->dep_->strName());
    }


#if 0
    // Title
    if(title_)
    {
            int n = fprintf(f_,"\n%s:\n",t_) - 2;
            while(n--) fputc('-',f_);
            fputc('\n',f_);
            t_ = 0;
    }

    p_.observe(&n);
    fprintf(f_,"%s {%s}",n.type_name(), n.full_name().c_str());
    if(p) {
        fprintf(f_," through ");
        p_.observe(p);

        switch(mode)
        {
            case trigger_lister::parent:  fprintf(f_,"parent "); break;
            case trigger_lister::child:   fprintf(f_,"child ");  break;
        }

        fprintf(f_,"%s {%s}",p->type_name(),p->full_name().c_str());
    }
    fputc('\n',f_);
#endif
}

QString TriggerListCollector::text()
{
    QString s="<table width=\'100%\'>";
    //s+="<tr><th width=\'100\'>Type</th><th>Path</th></tr>";

    bool firstDirectTrigger=true;

    for(unsigned int i=0; i < items_.size(); i++)
    {
        VItem *t=items_[i]->t_;
        VItem *d=items_[i]->dep_;
        if(!t)
            continue;

        if(!d)
        {
            if(firstDirectTrigger)
            {
                s+="<tr><td class=\'main\' colspan=\'2\'>Triggers directly triggering the selected node</td></tr>";
                firstDirectTrigger=false;
            }

            QString type=QString::fromStdString(t->typeName());
            QString path=QString::fromStdString(t->fullPath());
            QString anchor=QString::fromStdString(VItemPathParser::encode(t->fullPath(),t->typeName()));

            s+="<tr>";
            s+="<td width=\'100\'>" + type + "</td>";
            //s+="<td>" + t->name() +"</td>";
            //s+="<td><a style=\'text-decoration:none;\' href=\'aa\'>" + QString::fromStdString(t->fullPath()) +"</a>";
            s+="<td><a href=\'" + anchor  + "\'>" + path +"</a>";
        }
    }

    QString prevH;

    for(unsigned int i=0; i < items_.size(); i++)
    {
        VItem *t=items_[i]->t_;
        VItem *d=items_[i]->dep_;
        if(!t || !d)
            continue;

        QString h;

        if(items_[i]->mode_== Parent)
           h+="Through parent";
        else
           h+="Through child";

        QString type=QString::fromStdString(d->typeName());
        QString path=QString::fromStdString(d->fullPath());
        QString anchor=QString::fromStdString(VItemPathParser::encode(d->fullPath(),d->typeName()));


        h+="  " + type;
        h+=" <a class=\'chp\' href=\'" + anchor + "\'>" + path +"</a>";

        if(h != prevH)
        {
            s+="<tr><td class=\'child\' colspan=\'2\'>" + h + "</td></tr>";
            prevH=h;
        }

        type=QString::fromStdString(t->typeName());
        path=QString::fromStdString(t->fullPath());
        anchor=QString::fromStdString(VItemPathParser::encode(t->fullPath(),t->typeName()));

        s+="<tr>";
        s+="<td width=\'100\'>" + type + "</td>";
        s+="<td><a href=\'" +  anchor + "\'>" + path +"</a>";
     }

    return s;

#if 0
    QString s="<table>";
    s+="<tr><th colspan=\'2\'>" + QString::fromStdString(title_) + "</th></tr>";
    for(unsigned int i=0; i < items_.size(); i++)
    {

         VItem *t=items_[i]->t_;

         if(!t)
             continue;

         s+="<tr>";
         s+="<td>" + QString::fromStdString(t->typeName()) + "</td>";
         //s+="<td>" + t->name() +"</td>";
         //s+="<td><a style=\'text-decoration:none;\' href=\'aa\'>" + QString::fromStdString(t->fullPath()) +"</a>";
         s+="<td><a href=\'aa\'>" + QString::fromStdString(t->fullPath()) +"</a>";

         VItem *d=items_[i]->dep_;
         if(d)
         {

             UserMessage::debug(" dep=" + d->typeName() + " " +  d->strName());

            // s+="<td>";
             //s+="<td>" + QString::fromStdString(d->typeName());
             //s+=" " + d->name();

             if(items_[i]->mode_== Parent)
                s+="  through parent";
             else
                s+="  through child";

             s+="  " + QString::fromStdString(d->typeName());
             s+=" <a href=\'aa\'>" + QString::fromStdString(d->fullPath()) +"</a></td>";
         }

         s+="</tr>";
    }

    s+="</table>";
    return s;
#endif

}


void TriggerChildCollector::add(VItem* t, VItem*,Mode mode,VItem *t1)
{
    if(!t->isAncestor(node_))
    {
        // child is a kid of n whose trigger_panel is outside its subtree
        collector_->add(t,child_,TriggerCollector::Child,t1);
    }
}

void TriggerParentCollector::add(VItem* t, VItem*,Mode mode,VItem *t1)
{
    collector_->add(t,parent_,TriggerCollector::Parent,t1);
}


void TriggeredCollector::add(VItem* trigger, VItem*,Mode mode,VItem*)
{
    if(trigger->isNode())
        trigger->isNode()->addTriggeredData(node_);

    //else if(trigger->isAttribute())
    //    trigger->parent()->addTriggeredData(node_,trigger);

}

