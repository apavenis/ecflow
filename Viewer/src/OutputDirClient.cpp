//============================================================================
// Copyright 2014 ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//
//============================================================================

#include "OutputDirClient.hpp"

#include <QDebug>

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

OutputDirClient::OutputDirClient(const std::string& host,const std::string& portStr,QObject* parent) :
	OutputClient(host,portStr,parent)
{
}

VDir_ptr OutputDirClient::result() const
{
	return dir_;
}

void OutputDirClient::slotCheckTimeout()
{
	if(soc_->state() == QAbstractSocket::HostLookupState ||
	   soc_->state() == QAbstractSocket::ConnectingState)
	{
		soc_->abort();
		if(dir_)
			dir_.reset();

		Q_EMIT error("Timeout error");
	}
}

void OutputDirClient::slotConnected()
{
	qDebug() << "dir connected to " << soc_->peerName();

	soc_->write("list ",5);
	soc_->write(remoteFile_.c_str(),remoteFile_.size());
	soc_->write("\n",1);
}

void OutputDirClient::slotError(QAbstractSocket::SocketError err)
{
	qDebug() << "error" <<  soc_->errorString();

	switch(err)
	{
	case QAbstractSocket::RemoteHostClosedError:
		qDebug() << "remote host closed";

		if(dir_)
		{
			parseData();
		}

		Q_EMIT finished();

		if(dir_)
			dir_.reset();

		break;
	default:
		soc_->abort();
		if(dir_)
		{
			dir_.reset();
		}
		Q_EMIT error(soc_->errorString());
		break;
	}
}

void OutputDirClient::getDir(const std::string& name)
{
	connectToHost(host_,port_);

	boost::filesystem::path fp(name);
	std::string dirName=fp.parent_path().string();

	remoteFile_=name;
	dir_.reset();
	dir_=VDir_ptr(new VDir(dirName));

	//indicates the source of the files
	dir_->where(host_ + "@" + portStr_);

	connectToHost(host_,port_);
}

void OutputDirClient::slotRead()
{
	const qint64 size = 64*1024;
	char buf[size+1];
	quint64 len = 0;

	while((len = soc_->read(buf,size)) > 0)
	{
		data_.append(buf,len);
	}
}

void OutputDirClient::parseData()
{
	if(!dir_)
		return;

	QTextStream in(data_);
	while(!in.atEnd())
	{
		 int mode,uid,gid;
		 unsigned int size;
		 unsigned int atime,mtime,ctime;
		 QString name;

		 in >> mode >> uid >> gid >> size >> atime >> mtime >> ctime >> name;

		 if(!name.isEmpty())
		 {
			 boost::filesystem::path p(name.toStdString());
			 std::string fileDirName=p.parent_path().string();
			 std::string fileName=p.leaf().string();

			 //Adjust the path in the dir
			 if(dir_->path() !=  fileDirName)
			 {
				 dir_->path(fileDirName,false);
			 }

			 dir_->addItem(fileName,size,mtime);
		 }
	}
}
