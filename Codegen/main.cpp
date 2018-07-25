#include <iostream>
#include <fstream>
#include <algorithm>
#include <set>
#include <filesystem>

#include <Windows.h>

#include <clang-c\Index.h>

#include <unordered_map>

#include "stringUtils.h"
#include "fmt\format.h"

#include "json\json.hpp"
using json = nlohmann::json;

static const char *TEMPLATE_BEGIN = R"===(
#pragma once

#include "../dartHelpers.h"
)===";

// {0} - class name
// {1} - base class name
static const char *CLASS_TEMPLATE_WITH_BASE_BEGIN = R"===(
class AtumExt_{0} : public AtumExt_{1}
{{
public:
  {0} *objectAs{0} = nullptr;

public:
  void dart_ext_init()
  {{
    objectAs{0} = ({0}*)dart_ext_get_object();
  }}
)===";

static const char *CLASS_TEMPLATE_FINAL_BEGIN = R"===(
class AtumExt_{0} : public AtumExt_{1}
{{
public:
  {0} object;

public:
  virtual void* dart_ext_get_object() override
  {{
    return &object;
  }}
)===";

static const char *CLASS_TEMPLATE_BEGIN = R"===(
class AtumExt_{0} : public AtumExtClass
{{
public:
  {0} object;

public:
  virtual void* dart_ext_get_object() override
  {{
    return &object;
  }}
)===";

static const char *CLASS_TEMPLATE_ABSTRACT_BEGIN = R"===(
class AtumExt_{0} : public AtumExtClass
{{
public:
  {0} *objectAs{0} = nullptr;

public:
  void dart_ext_init()
  {{
    objectAs{0} = ({0}*)dart_ext_get_object();
  }}
)===";

static const char *CLASS_TEMPLATE_END = R"===(
};
)===";

// {0} - method name
// {1} - method params
// {2} - method args
// {3} - object access
// {4} - return type
// {5} - class name
static const char *METOD_TEMPLATE_DECL_NULL = R"===(
  /**
  * \returns null
  */
  Dart_Handle {0}({1});)===";

static const char *METOD_TEMPLATE_IMPL_NULL = R"===(
/**
* \returns null
*/
Dart_Handle AtumExt_{5}::{0}({1})
{{
  {3}{0}({2});
  return Dart_Null();
}}
)===";

static const char *METOD_TEMPLATE_DECL_VALUE = R"===(
  /**
  * \returns {4}
  */
  Dart_Handle {0}({1});)===";

static const char *METOD_TEMPLATE_IMPL_VALUE = R"===(
/**
* \returns {4}
*/
Dart_Handle AtumExt_{5}::{0}({1})
{{
  return nativeToDart({3}{0}({2}));
}}
)===";

static const char *METOD_TEMPLATE_DECL_GET_SET = R"===(
  /**
  * \returns {4}
  */
  Dart_Handle Get{0}();
  /**
  * \returns null
  */
  Dart_Handle Set{0}(const {4} value);)===";

static const char *METOD_TEMPLATE_IMPL_GET_SET = R"===(
/**
* \returns {4}
*/
Dart_Handle AtumExt_{5}::Get{0}()
{{
  return nativeToDart({3}{0}());
}}

/**
* \returns null
*/
Dart_Handle AtumExt_{5}::Set{0}(const {4} value)
{{
  {3}{0}() = value;
  return Dart_Null();
}}
)===";

// {0} - field name
// {1} - field access
// {2} - class name
static const char *METOD_TEMPLATE_DECL_GET_FIELD = R"===(
  Dart_Handle get_field_{0}();)===";

static const char *METOD_TEMPLATE_IMPL_GET_FIELD = R"===(
Dart_Handle AtumExt_{2}::get_field_{0}()
{{
  return nativeToDart({1}{0});
}}
)===";

// {0} - type name
// {1} - type name pure
static const char *FROM_VALUE_TEMPLATE = R"===(
template<>
inline Dart_Handle nativeToDart({0} value)
{{
  return Dart_Null();
}}
)===";

static const char *DART_TEMPLATE_BEGIN = R"===(
part of atum;
)===";

// {0} - class name
// {1} - base class name
// {2} - native methods
// {3} - dart methods
static const char *DART_CLASS_TEMPLATE = R"===(
class {0} extends {1} {{
{2}
{3}}}
)===";

static const char *DART_CLASS_ABSTRACT_TEMPLATE = R"===(
abstract class {0} extends {1} {{
{2}
{3}}}
)===";

static const char *DART_CLASS_FINAL_TEMPLATE = R"===(
class {0} extends {1} {{
{2}{3}
  _ctor() native "AtumExt_{0}::AtumExt_{0}";

  {0}() {{
    _ctor();
  }}
}}
)===";

struct Visitor
{
  static CXChildVisitResult visit(CXCursor c, CXCursor parent, CXClientData client_data)
  {
    Visitor *self = (Visitor*)client_data;
    self->process(c, parent);
    return CXChildVisit_Recurse;
  }

  class ScopeString
  {
  public:
    ScopeString(CXString&& s) : s(s) {};
    ~ScopeString() { clang_disposeString(s); };

    ScopeString(const ScopeString&) = delete;
    ScopeString& operator=(const ScopeString&) = delete;
    ScopeString(ScopeString&&) = delete;
    ScopeString& operator=(ScopeString&&) = delete;

    operator const char*() const { return str(); }
    const char* str() const { return clang_getCString(s); }

  private:
    CXString s;
  };

  struct String
  {
    String(CXString&& s) : str(clang_getCString(s)) { clang_disposeString(s); };

    String(const String&) = default;
    String& operator=(const String&) = default;
    String(String&&) = default;
    String& operator=(String&&) = default;

    operator const char*() const { return str.c_str(); }
    operator std::string() const { return str; }

    std::string str;
  };

  struct Parameter
  {
    std::string type;
    std::string typePure;
    std::string name;
  };

  struct Method
  {
    bool shouldIgnore = false;
    bool isVirtual = false;
    bool isPureVirtual = false;
    bool isReturnsValue = false;
    bool isReturnsRef = false;
    bool isOverridden = false;

    std::string name;
    std::string returnType;
    std::string returnTypePure;
    std::vector<Parameter> parameters;

    Method(const Method&) = delete;
    Method& operator=(const Method&) = delete;

    Method() = default;
    Method(Method&&) = default;
    Method& operator=(Method&&) = default;
  };

  struct Field
  {
    std::string type;
    std::string name;
  };

  struct Class
  {
    bool shouldIgnore = false;
    bool isAbstract = false;
    bool isFinal = true;

    std::string superName;
    std::string name;
    std::unordered_map<std::string, Method> methods;
    std::unordered_map<std::string, Field> fields;

    Class(const Class&) = delete;
    Class& operator=(const Class&) = delete;

    Class() = default;
    Class(Class&&) = default;
    Class& operator=(Class&&) = default;
  };

  std::unordered_map<std::string, Class> classes;
  std::set<std::string> valueTypes;

  struct Rules
  {
    std::set<std::string> deps;
    std::set<std::string> includes;
    std::set<std::string> includeClasses;
    std::set<std::string> excludeMethods;
    std::set<std::string> allowTypes;

    template <typename T>
    static inline void get_if_exists(const json& in, const char *key, T &out_value)
    {
      if (in.find(key) != in.end())
        out_value = in.at(key).get<std::remove_reference<decltype(out_value)>::type>();
    }

    void init(const json& in)
    {
      get_if_exists(in, "deps", deps);
      get_if_exists(in, "includes", includes);
      get_if_exists(in, "includeClasses", includeClasses);
      get_if_exists(in, "excludeMethods", excludeMethods);
      get_if_exists(in, "allowTypes", allowTypes);
    }

    bool shouldIgnoreClass(const std::string& class_name) const
    {
      return includeClasses.find(class_name) == includeClasses.end();
    }

    bool shouldIgnoreMethod(const std::string& method_name) const
    {
      return excludeMethods.find(method_name) != excludeMethods.end();
    }
  } rules;

  void readRules(const char *filename)
  {
    std::ifstream in;
    in.open(filename, std::ofstream::in);
    if (!in.is_open())
    {
      std::cerr << "Cannon open: " << filename << std::endl;
      return;
    }

    json j;
    in >> j;

    rules.init(j);
    return;
  }

  void process(CXCursor c, CXCursor parent)
  {
    CXCursorKind kind = clang_getCursorKind(c);
    CXCursorKind parentKind = clang_getCursorKind(parent);

    String spelling = clang_getCursorSpelling(c);
    String kindSpelling = clang_getCursorKindSpelling(kind);

    if (kind == CXCursor_ClassDecl || kind == CXCursor_StructDecl)
    {
      Class cls;
      cls.name = std::move(spelling.str);
      if (parentKind == CXCursor_ClassDecl || kind == CXCursor_StructDecl)
      {
        cls.name = String(clang_getCursorSpelling(parent)).str + "::" + cls.name;
      }
      cls.shouldIgnore = rules.shouldIgnoreClass(cls.name);
      classes[cls.name] = std::move(cls);
    }
    else if (kind == CXCursor_FieldDecl)
    {
      if (clang_getCXXAccessSpecifier(c) != CX_CXXPublic)
        return;

      CXType resType = clang_getCursorType(c);
      if (resType.kind == CXType_Pointer)
        return;

      String className = clang_getCursorSpelling(parent);
      auto it = classes.find(className);
      if (it != classes.end())
      {
        Field f;
        f.name = std::move(spelling.str);
        f.type = std::move(String(clang_getTypeSpelling(resType)).str);
        it->second.fields[f.name] = std::move(f);
      }
    }
    else if (kind == CXCursor_CXXBaseSpecifier)
    {
      String className = clang_getCursorSpelling(parent);
      auto it = classes.find(className);
      if (it != classes.end())
      {
        it->second.superName = std::move(spelling.str);
        size_t pos = it->second.superName.find("class ");
        if (pos != std::string::npos)
          it->second.superName.replace(pos, ::strlen("class "), "");
      }
    }
    else if (kind == CXCursor_CXXMethod)
    {
      String className = clang_getCursorSpelling(parent);
      auto it = classes.find(className);
      if (it != classes.end())
      {
        CXType resType = clang_getCursorResultType(c);

        Method m;
        m.name = std::move(spelling.str);
        m.returnType = std::move(String(clang_getTypeSpelling(resType)).str);
        m.isReturnsValue = m.returnType != "void";
        m.isReturnsRef = resType.kind == CXType_LValueReference || resType.kind == CXType_RValueReference;
        m.isVirtual = clang_CXXMethod_isVirtual(c) != 0;
        m.isPureVirtual = clang_CXXMethod_isPureVirtual(c) != 0;
        m.shouldIgnore = rules.shouldIgnoreMethod(m.name);

        m.returnTypePure = m.returnType;
        strutils::replace(m.returnTypePure, "const ", "");
        strutils::replace(m.returnTypePure, " &", "");
        strutils::replace(m.returnTypePure, " *", "");

        CXCursor *overridden = nullptr;
        unsigned num_overridden = 0;
        clang_getOverriddenCursors(c, &overridden, &num_overridden);
        m.isOverridden = num_overridden != 0;
        clang_disposeOverriddenCursors(overridden);

        it->second.methods[m.name] = std::move(m);

        if (m.isPureVirtual)
          it->second.isAbstract = true;
      }
    }
    else if (kind == CXCursor_ParmDecl)
    {
      CXCursor classCursor = clang_getCursorSemanticParent(parent);
      String className = clang_getCursorSpelling(classCursor);
      auto classIt = classes.find(className);
      if (classIt != classes.end())
      {
        String methodName = clang_getCursorSpelling(parent);
        auto methodIt = classIt->second.methods.find(methodName);
        if (methodIt != classIt->second.methods.end())
        {
          // TODO: Eval default params
          CXEvalResult res = clang_Cursor_Evaluate(c);
          double r = -1.f;
          if (res)
          {
            CXEvalResultKind k = clang_EvalResult_getKind(res);
            if (k == CXEval_Float)
              r = clang_EvalResult_getAsDouble(res);
            clang_EvalResult_dispose(res);
          }

          Parameter p;
          p.name = std::move(spelling.str);
          p.type = std::move(String(clang_getTypeSpelling(clang_getCursorType(c))).str);

          p.typePure = p.type;
          strutils::replace(p.typePure, "const ", "");
          strutils::replace(p.typePure, " &", "");
          strutils::replace(p.typePure, " *", "");

          methodIt->second.parameters.push_back(std::move(p));
        }
      }
    }
  }

  void postProcess()
  {
    for (auto& p : classes)
    {
      if (!p.second.superName.empty())
      {
        auto superIt = classes.find(p.second.superName);
        if (superIt != classes.end())
          superIt->second.isFinal = false;
      }
    }
  }

  void saveDartTo(const char *filename)
  {
    std::ofstream out;
    out.open(filename, std::ofstream::out);
    if (!out.is_open())
    {
      std::cerr << "Cannon open: " << filename << std::endl;
      return;
    }

    out << DART_TEMPLATE_BEGIN;

    std::unordered_map<std::string, std::string> typesRemap = {
      { "void", "" },
      { "float", "double" },
      { "const char *", "String" },
      { "Color", "Float32List" },
      { "Color &", "Float32List" },
      { "Vector &", "Float32List" },
      { "Matrix &", "Float32List" }
    };

    std::unordered_map<std::string, std::string> typesDartRemap = {
      { "float", "double" },
      { "const char *", "String" },
      { "Color", "Vector4" },
      { "Color &", "Vector4" },
      { "Vector &", "Vector3" },
      { "Matrix &", "Matrix4" }
    };

    for (auto& dep : rules.deps)
    {
      typesRemap.insert(std::pair<std::string, std::string>(dep + " *", dep));
      typesDartRemap.insert(std::pair<std::string, std::string>(dep + " *", dep));
    }

    static const std::unordered_map<std::string, std::string> dartCtorRemap = {
      { "Matrix4", "new Matrix4.fromFloat32List({})" },
      { "Vector3", "new Vector3.fromFloat32List({})" },
      { "Vector4", "new Vector4.fromFloat32List({})" }
    };

    static const std::unordered_map<std::string, std::string> dartGetRemap = {
      { "Matrix4", "{0}.storage" },
      { "Vector3", "{0}.storage" },
      { "Vector4", "{0}.storage" }
    };

    static const std::set<std::string> primitiveDartTypes = {
      "bool",
      "int",
      "double"
    };

    for (auto& p : classes)
    {
      const auto& cls = p.second;
      if (cls.shouldIgnore)
        continue;

      std::string nativeMethods;
      std::string dartMethods;
      for (auto& m : cls.methods)
      {
        const auto& method = m.second;
        if (method.shouldIgnore)
          continue;

        bool addComma = false;
        std::string defParams;
        std::string params;
        std::string args;
        for (auto& param : method.parameters)
        {
          if (addComma)
          {
            defParams += ", ";
            params += ", ";
            args += ", ";
          }

          auto remapIt = typesRemap.find(param.type);
          std::string dartType = remapIt == typesRemap.end() ? param.type : remapIt->second;

          addComma = true;
          defParams += fmt::format("{}", dartType);
          params += fmt::format("{} {}", dartType, param.name);
          args += fmt::format("{}", param.name);
        }

        auto remapIt = typesRemap.find(method.returnType);
        std::string dartNativeType = remapIt == typesRemap.end() ? method.returnType : remapIt->second;

        auto remapDartIt = typesDartRemap.find(method.returnType);
        std::string dartType = remapDartIt == typesDartRemap.end() ? method.returnType : remapDartIt->second;

        auto dartCtorIt = dartCtorRemap.find(dartType);
        std::string dartCtor = dartCtorIt == dartCtorRemap.end() ? "{}" : dartCtorIt->second;

        auto dartGetIt = dartGetRemap.find(dartType);
        std::string dartGet = dartGetIt == dartGetRemap.end() ? "{}" : dartGetIt->second;

        if (method.isReturnsRef)
        {
          nativeMethods += fmt::format("  {2} _Get{0}() native \"AtumExt_{1}::Get{0}\";\n", method.name, cls.name, dartNativeType);
          nativeMethods += fmt::format("  _Set{0}({2}) native \"AtumExt_{1}::Set{0}\";\n", method.name, cls.name, dartNativeType);

          dartMethods += fmt::format("  {1} get {0} => {2};\n", method.name, dartType, fmt::format(dartCtor, fmt::format("_Get{}()", method.name)));
          dartMethods += fmt::format("  set {0}({1} v) => _Set{0}({2});\n", method.name, dartType, fmt::format(dartGet, "v"));
        }
        else if (dartNativeType.empty())
        {
          bool isSetMethod = false;
          if (method.name.find("Set") == 0 || method.name.find("set") == 0)
          {
            isSetMethod = true;
            std::string name = method.name;
            strutils::replace(name, "Set", "");
            strutils::replace(name, "set", "");
            dartMethods += fmt::format("  set {0}({1} v) => _Set{0}({2});\n", name, defParams, fmt::format(dartGet, "v"));
          }

          std::string prefix;
          if (method.isOverridden && cls.isFinal)
          {
            prefix = "  @override\n";
          }

          if (method.isPureVirtual || (method.isVirtual && cls.isAbstract))
            nativeMethods += fmt::format("  {0}({1});\n", method.name, defParams);
          else if (!method.isReturnsValue && !isSetMethod)
            nativeMethods += fmt::format("{3}  {0}({2}) native \"AtumExt_{1}::{0}\";\n", method.name, cls.name, params, prefix);
          else
            nativeMethods += fmt::format("  _{0}({2}) native \"AtumExt_{1}::{0}\";\n", method.name, cls.name, defParams);
        }
        else
        {
          bool isGetMethod = false;
          if (method.name.find("Get") == 0 || method.name.find("get") == 0)
          {
            isGetMethod = true;
            std::string name = method.name;
            strutils::replace(name, "Get", "");
            strutils::replace(name, "get", "");
            dartMethods += fmt::format("  {1} get {0} => {2};\n", name, dartType, fmt::format(dartCtor, fmt::format("_{}()", method.name)));
          }

          if (method.isPureVirtual)
            nativeMethods += fmt::format("  {1} {0}();\n", method.name, dartNativeType);
          else if ((!method.isReturnsValue || primitiveDartTypes.find(dartNativeType) != primitiveDartTypes.end()) && !isGetMethod)
            nativeMethods += fmt::format("  {2} {0}() native \"AtumExt_{1}::{0}\";\n", method.name, cls.name, dartNativeType);
          else
            nativeMethods += fmt::format("  {2} _{0}() native \"AtumExt_{1}::{0}\";\n", method.name, cls.name, dartNativeType);
        }
      }

      for (auto& f : cls.fields)
      {
        const auto& field = f.second;

        auto remapIt = typesRemap.find(field.type);
        std::string dartNativeType = remapIt == typesRemap.end() ? field.type : remapIt->second;

        auto remapDartIt = typesDartRemap.find(field.type);
        std::string dartType = remapDartIt == typesDartRemap.end() ? field.type : remapDartIt->second;

        auto dartCtorIt = dartCtorRemap.find(dartType);
        std::string dartCtor = dartCtorIt == dartCtorRemap.end() ? "{}" : dartCtorIt->second;

        nativeMethods += fmt::format("  {2} _get_field_{0}() native \"AtumExt_{1}::get_field_{0}\";\n", field.name, cls.name, dartNativeType);
        dartMethods += fmt::format("  {1} get {0} => {2};\n", field.name, dartType, fmt::format(dartCtor, fmt::format("_get_field_{}()", field.name)));
      }

      out << fmt::format(cls.isFinal ? DART_CLASS_FINAL_TEMPLATE : cls.isAbstract ? DART_CLASS_ABSTRACT_TEMPLATE : DART_CLASS_TEMPLATE,
        cls.name,
        cls.superName.empty() || cls.superName == "Object" ? "_Native" : cls.superName,
        nativeMethods,
        dartMethods) << std::endl;
    }
  }

  static std::string removeModifiers(const std::string& str)
  {
    std::string result(str);
    strutils::replace(result, "const ", "");
    strutils::replace(result, " &", "");
    strutils::replace(result, " *", "");
    return std::move(result);
  }

  void saveCPPTo(const char *filename)
  {
    std::ofstream cppOut;
    std::ofstream hOut;

    cppOut.open(std::string(filename) + ".gen.cpp", std::ofstream::out);
    if (!cppOut.is_open())
    {
      std::cerr << "Cannon open: " << filename << ".gen.cpp" << std::endl;
      return;
    }

    hOut.open(std::string(filename) + ".gen.h", std::ofstream::out);
    if (!hOut.is_open())
    {
      std::cerr << "Cannon open: " << filename << ".gen.h" << std::endl;
      return;
    }

    hOut << TEMPLATE_BEGIN << std::endl;
    cppOut << fmt::format("#include \"{}.gen.h\"", std::experimental::filesystem::path(filename).filename().generic_string()) << std::endl;

    for (auto& inc : rules.includes)
      hOut << fmt::format("#include <{}>", inc) << std::endl;

    for (auto& dep : rules.deps)
      hOut << fmt::format("#include \"{}.gen.h\"", dep) << std::endl;

    std::string templateBody;
    for (auto& p : classes)
    {
      const auto& cls = p.second;
      if (cls.shouldIgnore)
        continue;

      std::string head;
      std::string access;

      if (cls.isAbstract && (cls.superName.empty() || cls.superName == "Object"))
      {
        head = fmt::format(CLASS_TEMPLATE_ABSTRACT_BEGIN, p.first);
        access = fmt::format("objectAs{0}->", p.first);
      }
      else if (cls.superName.empty())
      {
        head = fmt::format(CLASS_TEMPLATE_BEGIN, p.first);
        access = "object.";
      }
      else
      {
        head = fmt::format(cls.isFinal ? CLASS_TEMPLATE_FINAL_BEGIN : CLASS_TEMPLATE_WITH_BASE_BEGIN, p.first, cls.superName);
        access = cls.isFinal ? "object." : fmt::format("objectAs{0}->", p.first);
      }

      std::string body;
      std::string bodyImpl;
      std::string def = fmt::format(cls.isAbstract ? "NATIVE_METHOD_LIST_NO_CTOR(AtumExt_{}," : "NATIVE_METHOD_LIST(AtumExt_{},", cls.name);

      bool addMethodComma = false;
      for (auto& m : cls.methods)
      {
        const auto& method = m.second;
        if (method.shouldIgnore)
          continue;

        if (method.isReturnsValue)
          valueTypes.insert(method.returnTypePure);

        bool addComma = false;
        std::string defParams;
        std::string params;
        std::string args;
        for (auto& param : method.parameters)
        {
          if (addComma)
          {
            defParams += ", ";
            params += ", ";
            args += ", ";
          }

          addComma = true;
          defParams += fmt::format("{}", param.type);
          params += fmt::format("{} {}", param.type, param.name);
          args += fmt::format("{}", param.name);

          // valueTypes.insert(param.typePure);
        }

        if (method.isReturnsValue)
        {
          body += fmt::format(method.isReturnsRef ? METOD_TEMPLATE_DECL_GET_SET : METOD_TEMPLATE_DECL_VALUE, method.name, params, args, access, method.returnType, cls.name);
          bodyImpl += fmt::format(method.isReturnsRef ? METOD_TEMPLATE_IMPL_GET_SET : METOD_TEMPLATE_IMPL_VALUE, method.name, params, args, access, method.returnType, cls.name);
        }
        else
        {
          body += fmt::format(METOD_TEMPLATE_DECL_NULL, method.name, params, args, access);
          bodyImpl += fmt::format(METOD_TEMPLATE_IMPL_NULL, method.name, params, args, access, "", cls.name);
        }

        if (addMethodComma)
        {
          def += ",";
        }

        addMethodComma = true;

        if (method.isReturnsRef)
        {
          def += fmt::format("\n  NATIVE_METHOD(AtumExt_{0}, Get{1}),", cls.name, method.name);

          std::string typeWithoutModifiers = method.returnType;
          if (size_t pos = typeWithoutModifiers.find("const "))
          {
            if (pos != std::string::npos)
              typeWithoutModifiers.replace(pos, ::strlen("const "), "");
          }
          if (size_t pos = typeWithoutModifiers.find(" &"))
          {
            if (pos != std::string::npos)
              typeWithoutModifiers.replace(pos, ::strlen(" &"), "");
          }

          def += fmt::format("\n  NATIVE_METHOD(AtumExt_{0}, Set{1}, {2})", cls.name, method.name, typeWithoutModifiers);
        }
        else if (defParams.empty())
          def += fmt::format("\n  NATIVE_METHOD(AtumExt_{}, {})", cls.name, method.name);
        else
          def += fmt::format("\n  NATIVE_METHOD(AtumExt_{}, {}, {})", cls.name, method.name, defParams);
      }

      for (auto& f : cls.fields)
      {
        const auto& field = f.second;

        if (field.type.find("std::vector") != std::string::npos)
          continue;

        std::string access = "object.";

        if (cls.isAbstract && (cls.superName.empty() || cls.superName == "Object"))
          access = fmt::format("objectAs{0}->", p.first);
        else if (cls.superName.empty())
          access = "object.";
        else
          access = cls.isFinal ? "object." : fmt::format("objectAs{0}->", p.first);

        valueTypes.insert(field.type);

        body += fmt::format(METOD_TEMPLATE_DECL_GET_FIELD, field.name, access);
        bodyImpl += fmt::format(METOD_TEMPLATE_IMPL_GET_FIELD, field.name, access, cls.name);

        if (addMethodComma)
        {
          def += ",";
        }

        addMethodComma = true;

        def += fmt::format("\n  NATIVE_METHOD(AtumExt_{}, get_field_{})", cls.name, field.name);
      }

      def += ");";

      // templateBody += fmt::format("{}{}{}\n{}\n", head, body, CLASS_TEMPLATE_END, def);
      templateBody += fmt::format("{}\n{}\n", bodyImpl, def);
      hOut << fmt::format("{}{}{}", head, body, CLASS_TEMPLATE_END);
    }

    static const std::set<std::string> knownTypes = {
      "bool",
      "int",
      "float",
      "double",
      "const char *",
      "Matrix",
      "Matrix &",
      "const Matrix &",
      "Color",
      "Color &",
      "const Color &",
      "Vector",
      "Vector &",
      "const Vector &",
      "Vector2",
      "Vector2 &",
      "const Vector2 &",
    };

    std::string fromValueFuncs;
    for (auto &type : valueTypes)
    {
      if (knownTypes.find(type) != knownTypes.end())
        continue;

      if (rules.deps.find(type) != rules.deps.end())
        continue;

      /*if (type.find("std::vector") != std::string::npos)
        continue;*/

      std::string typePure = removeModifiers(type);

      if (type.find("*") != std::string::npos)
        fromValueFuncs += fmt::format(FROM_VALUE_TEMPLATE, type + " const &", typePure);
      else if (type.find("&") != std::string::npos)
        fromValueFuncs += fmt::format(FROM_VALUE_TEMPLATE, type, typePure);
      else
        fromValueFuncs += fmt::format(FROM_VALUE_TEMPLATE, "const " + type + " &", typePure);
    }
    cppOut << fromValueFuncs << templateBody << std::endl;
  }
};

template<typename T, bool HasMethod>
struct Test
{
  static void testFunc(const T &in) {}
};

template<typename T>
struct Test<T, true>
{
  static void testFunc(const T &in) {}
};

int main(int argc, char* argv[])
{
  int a = 0;
  Test<int, true>::testFunc(a);

  if (argc < 5)
  {
    std::cerr << "Usage: " << argv[0] << " <rules.json> <input.cpp> <output.cpp> <output.dart>" << std::endl;
    return -1;
  }

  char path[MAX_PATH];
  ::GetCurrentDirectory(MAX_PATH, path);

  const char *clangArgs[] = {
    "-xc++",
    "-w",
    "-D_DEBUG",
    "-Wno-invalid-token-paste",
    "-ID:/Projects/Atum/Atum",
    "-ID:/Projects/Atum/Libraries/EUI/include",
    "-ID:/Projects/Atum/Libraries/PhysX/Include"
  };
  const int clangArgsCount = _countof(clangArgs);

  CXIndex index = clang_createIndex(1, 1);
  CXTranslationUnit unit = clang_parseTranslationUnit(
    index,
    argv[2],
    clangArgs, clangArgsCount,
    nullptr, 0,
    CXTranslationUnit_None);
  if (unit == nullptr)
  {
    std::cerr << "Unable to parse translation unit. Quitting." << std::endl;
    return -1;
  }

  Visitor visitor;

  visitor.readRules(argv[1]);

  CXCursor cursor = clang_getTranslationUnitCursor(unit);
  clang_visitChildren(
    cursor,
    Visitor::visit,
    &visitor);

  clang_disposeTranslationUnit(unit);
  clang_disposeIndex(index);

  visitor.postProcess();

  visitor.saveCPPTo(argv[3]);
  visitor.saveDartTo(argv[4]);

  return 0;
}