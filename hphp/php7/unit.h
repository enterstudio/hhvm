/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#ifndef incl_HPHP_PHP_UNIT_H
#define incl_HPHP_PHP_UNIT_H

#include "hphp/php7/bytecode.h"
#include "hphp/php7/cfg.h"
#include "hphp/runtime/base/attr.h"

#include <boost/variant.hpp>

#include <string>
#include <vector>

namespace HPHP { namespace php7 {

struct Function;
struct Class;
struct Region;
struct Unit;

struct Function {
  struct Param {
    std::string name;
    bool byRef;
  };

  explicit Function(Unit* parent, Class* cls = nullptr)
    : parent(parent)
    , definingClass(cls) {}

  bool returnsByReference() const {
    return attr & Attr::AttrReference;
  }

  std::string name;
  Attr attr{Attr::AttrNone};
  Unit* parent;
  Class* definingClass;
  CFG cfg;
  std::vector<Param> params;
  uint32_t startLineno;
  uint32_t endLineno;
};

struct Class {
  explicit Class(Unit* parent, uint32_t index)
    : parent(parent)
    , index(index) {}

  Function* makeMethod() {
    methods.emplace_back(std::make_unique<Function>(parent, this));
    return methods.back().get();
  }

  struct Property {
    std::string name;
    Attr attr;
    std::string initializer;
    CFG cfg;
  };

  Function* getConstructor(uint32_t lineno);
  void buildPropInit(uint32_t lineno);

  Unit* parent;
  std::string name;
  folly::Optional<std::string> parentName;
  std::vector<std::string> implements;
  std::vector<std::string> traits;
  uint32_t index;
  Attr attr{Attr::AttrNone};
  std::vector<std::unique_ptr<Function>> methods;
  std::vector<Property> properties;
};

struct Unit {
  explicit Unit()
    : pseudomain(std::make_unique<Function>(this)) {}

  Function* getPseudomain() const {
    return pseudomain.get();
  }

  Function* makeFunction() {
    functions.emplace_back(std::make_unique<Function>(this));
    return functions.back().get();
  }

  uint32_t nextClassId() {
    return classes.size();
  }

  Class* makeClass() {
    classes.emplace_back(std::make_unique<Class>(this, nextClassId()));
    return classes.back().get();
  }

  std::string name;
  std::unique_ptr<Function> pseudomain;
  std::vector<std::unique_ptr<Function>> functions;
  std::vector<std::unique_ptr<Class>> classes;
};

std::unique_ptr<Unit> makeFatalUnit(const std::string& filename,
                                    const std::string& msg);


}} // HPHP::php7

#endif // incl_HPHP_PHP_UNIT_H
