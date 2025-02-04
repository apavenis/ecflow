/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// Name        :
// Author      : Avi
// Revision    : $Revision: #10 $
//
// Copyright 2009- ECMWF.
// This software is licensed under the terms of the Apache Licence version 2.0
// which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
// In applying this licence, ECMWF does not waive the privileges and immunities
// granted to it by virtue of its status as an intergovernmental organisation
// nor does it submit to any jurisdiction.
//
// Description :
/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8

#include "PrintStyle.hpp"
// #include <iostream>

static PrintStyle::Type_t style_ = PrintStyle::NOTHING;

PrintStyle::Type_t PrintStyle::getStyle() {
    return style_;
}

void PrintStyle::setStyle(PrintStyle::Type_t f) {
    // std::cout << "PrintStyle::setStyle() BEFORE " << PrintStyle::to_string(style_) << " AFTER " <<
    // PrintStyle::to_string(f) << "   *********************\n";
    style_ = f;
}

bool PrintStyle::defsStyle() {
    if (getStyle() == PrintStyle::DEFS || getStyle() == PrintStyle::NOTHING) {
        return true;
    }
    return false;
}

bool PrintStyle::persist_style() {
    if (getStyle() == PrintStyle::MIGRATE || getStyle() == PrintStyle::NET) {
        return true;
    }
    return false;
}

bool PrintStyle::is_persist_style(PrintStyle::Type_t t) {
    if (t == PrintStyle::MIGRATE || t == PrintStyle::NET) {
        return true;
    }
    return false;
}

std::string PrintStyle::to_string() {
    return to_string(getStyle());
}

std::string PrintStyle::to_string(PrintStyle::Type_t t) {
    switch (t) {
        case PrintStyle::NOTHING:
            return "NOTHING";
            break;
        case PrintStyle::DEFS:
            return "DEFS";
            break;
        case PrintStyle::STATE:
            return "STATE";
            break;
        case PrintStyle::MIGRATE:
            return "MIGRATE";
            break;
        case PrintStyle::NET:
            return "NET";
            break;
    }
    return std::string();
}
