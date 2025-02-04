//============================================================================
// Copyright 2009- ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//============================================================================

#ifndef GENFILEPROVIDER_HPP
#define GENFILEPROVIDER_HPP

#include <string>
#include <vector>

#include "FetchTask.hpp"

class GenFileReceiver;
class VReply;

class GenFileProvider : public FetchQueueOwner {
public:
    GenFileProvider(GenFileReceiver*);
    ~GenFileProvider();
    GenFileProvider(const GenFileProvider&)            = delete;
    GenFileProvider& operator=(const GenFileProvider&) = delete;

    void clear() override;

    virtual void fetchFile(const std::string&);
    virtual void fetchFiles(const std::vector<std::string>&);
    const std::vector<std::string>& filesToFetch() const { return filesToFetch_; }
    VReply* theReply() const override { return reply_; }
    void fetchQueueSucceeded() override;
    void fetchQueueFinished(const std::string& filePath, VNode*) override;

protected:
    VReply* reply_{nullptr};
    GenFileReceiver* provider_{nullptr};
    std::vector<std::string> filesToFetch_;
};

class GenFileReceiver {
public:
    GenFileReceiver();
    virtual ~GenFileReceiver();
    GenFileReceiver(const GenFileReceiver&)            = delete;
    GenFileReceiver& operator=(const GenFileReceiver&) = delete;

    virtual void fileFetchFinished(VReply*)            = 0;
    virtual void fileFetchFailed(VReply*)              = 0;

protected:
    GenFileProvider* fetchManager_{nullptr};
};

#endif // GENFILEPROVIDER_HPP
