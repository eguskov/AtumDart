#pragma once

#include <iostream>
#include <atomic>
#include <vector>
#include <array>
#include <utility>

#include <dart_api.h>
#include <dart_native_api.h>

#include <Support/Matrix.h>
#include <Support/Vector.h>
#include <Support/Vector2.h>
#include <Support/Color.h>

static inline Dart_Handle HandleError(Dart_Handle handle)
{
  if (Dart_IsError(handle))
  {
    Dart_PropagateError(handle);
  }
  return handle;
}

template <typename T>
static Dart_Handle newObject(Dart_Handle type, T* instance)
{
  intptr_t peer = (intptr_t)instance;
  Dart_Handle object = HandleError(Dart_AllocateWithNativeFields(type, 1, &peer));
  Dart_NewWeakPersistentHandle(object, reinterpret_cast<void*>(instance), sizeof(*instance), releasePeer<T>);

  return HandleError(Dart_InvokeConstructor(object, Dart_Null(), 0, nullptr));
}

template <typename T>
static Dart_Handle newObjectRef(Dart_Handle type, T* instance)
{
  instance->retain();

  intptr_t peer = (intptr_t)instance;
  Dart_Handle object = HandleError(Dart_AllocateWithNativeFields(type, 1, &peer));
  Dart_NewWeakPersistentHandle(object, reinterpret_cast<void*>(instance), sizeof(*instance), releasePeer<T>);
  return HandleError(Dart_InvokeConstructor(object, Dart_Null(), 0, nullptr));
}

class RefCount
{
  std::atomic<int> refCount = 1;

public:
  virtual ~RefCount() {}

  virtual void retain()
  {
    ++refCount;
  }

  virtual void release()
  {
    std::cout << "RefCount[" << refCount << "]::release: " << this << std::endl;

    if (refCount > 1)
    {
      --refCount;
      return;
    }

    if (refCount > 0)
    {
      refCount = 0;
      delete this;
    }
  }
};

struct AtumExtClass : public RefCount
{
  virtual void* dart_ext_get_object() = 0;
};

enum class ObjectKind
{
  ClassPointer,
  Unknown
};

template<typename T>
struct get_kind {
private:

  static constexpr bool isClassPointer()
  {
    return std::is_pointer<T>::value && std::is_class<std::remove_pointer<T>::type>::value;
  }

  static constexpr ObjectKind detect()
  {
    return isClassPointer() ? ObjectKind::ClassPointer : ObjectKind::Unknown;
  }

public:
  static constexpr ObjectKind value = detect();
};

template<typename T, ObjectKind Kind>
struct dart_bridge {};

template<typename T>
struct dart_bridge<T, ObjectKind::Unknown>
{
  static inline void dartToNative(Dart_Handle argHandle, T& argValue)
  {
    static_assert(false, "dartToNative: Fuck, this shit! Missed implementation for type!");
  }

  static inline Dart_Handle nativeToDart(const T& value)
  {
    static_assert(false, "nativeToDart: Fuck, this shit! Missed implementation for type!");
    return Dart_Null();
  }
};

template<typename T>
struct dart_bridge<T, ObjectKind::ClassPointer>
{
  static inline void dartToNative(Dart_Handle argHandle, T& argValue)
  {
    argValue = nullptr;

    intptr_t peer = 0;
    HandleError(Dart_GetNativeInstanceField(argHandle, 0, &peer));
    if (peer)
      argValue = (T)((AtumExtClass*)peer)->dart_ext_get_object();
  }

  static inline Dart_Handle nativeToDart(const T& value)
  {
    return Dart_Null();
    // return newObjectRef(Dart_Handle type, value);
  }
};

template<typename T>
static inline void dartToNative(Dart_Handle argHandle, T& argValue)
{
  // For any unknown type
  dart_bridge<T, get_kind<T>::value>::dartToNative(argHandle, argValue);
}

template<typename T>
static inline Dart_Handle nativeToDart(const T& value)
{
  // For any unknown type
  return dart_bridge<T, get_kind<T>::value>::nativeToDart(value);
}

template<>
static inline void dartToNative(Dart_Handle argHandle, const char*& argValue)
{
  Dart_StringToCString(argHandle, &argValue);
}

template<>
static inline void dartToNative(Dart_Handle argHandle, Dart_Handle& argValue)
{
  argValue = argHandle;
}

template<>
static inline void dartToNative(Dart_Handle argHandle, int& argValue)
{
  int64_t v = 0;
  Dart_IntegerToInt64(argHandle, &v);
  argValue = (int)v;
}

template<>
static inline void dartToNative(Dart_Handle argHandle, float& argValue)
{
  double v = 0.0;
  Dart_DoubleValue(argHandle, &v);
  argValue = (float)v;
}

template<>
static inline void dartToNative(Dart_Handle argHandle, Vector& argValue)
{
  Dart_TypedData_Type dataTypeUnused;
  void* dataLocation;
  intptr_t dataLengthUnused;
  Dart_TypedDataAcquireData(argHandle, &dataTypeUnused, &dataLocation, &dataLengthUnused);

  ::memcpy(argValue.v, dataLocation, sizeof(Vector));

  Dart_TypedDataReleaseData(argHandle);
}

template<>
static inline void dartToNative(Dart_Handle argHandle, Vector2& argValue)
{
  Dart_TypedData_Type dataTypeUnused;
  void* dataLocation;
  intptr_t dataLengthUnused;
  Dart_TypedDataAcquireData(argHandle, &dataTypeUnused, &dataLocation, &dataLengthUnused);

  ::memcpy(argValue.v, dataLocation, sizeof(Vector2));

  Dart_TypedDataReleaseData(argHandle);
}

template<>
static inline void dartToNative(Dart_Handle argHandle, Color& argValue)
{
  Dart_TypedData_Type dataTypeUnused;
  void* dataLocation;
  intptr_t dataLengthUnused;
  Dart_TypedDataAcquireData(argHandle, &dataTypeUnused, &dataLocation, &dataLengthUnused);

  ::memcpy(&argValue.r, dataLocation, sizeof(Color));

  Dart_TypedDataReleaseData(argHandle);
}

template<>
static inline void dartToNative(Dart_Handle argHandle, Matrix& argValue)
{
  Dart_TypedData_Type data_type_unused;
  void* data_location;
  intptr_t data_length_unused;
  Dart_TypedDataAcquireData(argHandle, &data_type_unused, &data_location, &data_length_unused);

  ::memcpy(argValue.matrix, data_location, sizeof(float) * 16);

  Dart_TypedDataReleaseData(argHandle);
}

template<typename T>
static inline T dartToNative(Dart_NativeArguments arguments, size_t index)
{
  Dart_Handle handle = Dart_GetNativeArgument(arguments, (int)index);
  T value;
  dartToNative(handle, value);
  return value;
}

template<>
static inline Dart_Handle nativeToDart(const bool& value)
{
  return Dart_NewBoolean(value);
}

template<>
static inline Dart_Handle nativeToDart(const int& value)
{
  return Dart_NewInteger(value);
}

template<>
static inline Dart_Handle nativeToDart(const float& value)
{
  double val = value;
  return Dart_NewDouble(val);
}

template<>
static inline Dart_Handle nativeToDart(const char * const & value)
{
  return Dart_NewStringFromCString(value);
}

template<>
static inline Dart_Handle nativeToDart(const Vector& value)
{
  Dart_Handle list = Dart_NewTypedData(Dart_TypedData_kFloat32, 3);

  Dart_TypedData_Type dataTypeUnused;
  void* dataLocation;
  intptr_t dataLengthUnused;
  Dart_TypedDataAcquireData(list, &dataTypeUnused, &dataLocation, &dataLengthUnused);

  ::memcpy(dataLocation, value.v, sizeof(value));

  Dart_TypedDataReleaseData(list);

  return HandleError(list);
}

template<>
static inline Dart_Handle nativeToDart(const Vector2& value)
{
  Dart_Handle list = Dart_NewTypedData(Dart_TypedData_kFloat32, 2);

  Dart_TypedData_Type dataTypeUnused;
  void* dataLocation;
  intptr_t dataLengthUnused;
  Dart_TypedDataAcquireData(list, &dataTypeUnused, &dataLocation, &dataLengthUnused);

  ::memcpy(dataLocation, value.v, sizeof(value));

  Dart_TypedDataReleaseData(list);

  return HandleError(list);
}

template<>
static inline Dart_Handle nativeToDart(const Color& value)
{
  Dart_Handle list = Dart_NewTypedData(Dart_TypedData_kFloat32, 4);

  Dart_TypedData_Type dataTypeUnused;
  void* dataLocation;
  intptr_t dataLengthUnused;
  Dart_TypedDataAcquireData(list, &dataTypeUnused, &dataLocation, &dataLengthUnused);

  ::memcpy(dataLocation, &value.r, sizeof(value));

  Dart_TypedDataReleaseData(list);

  return HandleError(list);
}

template<>
static inline Dart_Handle nativeToDart(const Matrix& value)
{
  Dart_Handle list = Dart_NewTypedData(Dart_TypedData_kFloat32, 16);

  Dart_TypedData_Type dataTypeUnused;
  void* dataLocation;
  intptr_t dataLengthUnused;
  Dart_TypedDataAcquireData(list, &dataTypeUnused, &dataLocation, &dataLengthUnused);

  ::memcpy(dataLocation, value.matrix, sizeof(value));

  Dart_TypedDataReleaseData(list);

  return HandleError(list);
}

static intptr_t getPeer(Dart_NativeArguments arguments)
{
  intptr_t peer = 0;
  Dart_Handle dartThis = HandleError(Dart_GetNativeArgument(arguments, 0));
  HandleError(Dart_GetNativeInstanceField(dartThis, 0, &peer));
  return peer;
}

template <typename ... T>
constexpr size_t argsCount(const T& ...) { return sizeof...(T); }

struct NativeFunction
{
  const char* name;
  Dart_NativeFunction function;

  constexpr NativeFunction() : name(nullptr), function(nullptr) {}
  constexpr NativeFunction(const char* n, const Dart_NativeFunction& f) : name(n), function(f) {}
};

struct NativeLookup
{
  static std::vector<NativeFunction> *functions;

  template <size_t N>
  static void push(const std::array<NativeFunction, N>& arr)
  {
    if (!functions)
      functions = new std::vector<NativeFunction>;
    for (auto& f : arr)
      functions->push_back(f);
  }

  static Dart_NativeFunction find(const char* name)
  {
    for (auto& f : *functions)
      if (!::strcmp(name, f.name))
        return f.function;
    return nullptr;
  }
};

#define STR(a) #a
#define CONCAT(a, b) STR(a) STR(::) STR(b)
#define NATIVE_METHOD(cls, method, ...) NativeFunction(CONCAT(cls, method), nativeCall<cls, decltype(&cls::method), &cls::method, __VA_ARGS__>)
#define NATIVE_FUNCON(cls, func) NativeFunction(CONCAT(cls, func), cls::func)
#define NATIVE_CTOR(cls) NativeFunction(CONCAT(cls, cls), newInstance<cls>)

#define NATIVE_METHOD_LIST(cls, ...) \
  static std::array<NativeFunction, argsCount(__VA_ARGS__) + 1> native##cls = { { NATIVE_CTOR(cls), __VA_ARGS__ } }; \
  struct native##cls##_pushToNativeLookup { native##cls##_pushToNativeLookup() { NativeLookup::push(native##cls); }} native##cls##_push;

#define NATIVE_METHOD_LIST_NO_CTOR(cls, ...) \
  static std::array<NativeFunction, argsCount(__VA_ARGS__)> native##cls = { { __VA_ARGS__ } }; \
  struct native##cls##_pushToNativeLookup { native##cls##_pushToNativeLookup() { NativeLookup::push(native##cls); }} native##cls##_push;

template <typename PeerT, typename Method, typename... Args, size_t... I>
static inline void nativeCallImpl(Dart_NativeArguments arguments, Method method, std::index_sequence<I...>)
{
  Dart_EnterScope();
  Dart_SetReturnValue(arguments, Dart_Null());
  if (intptr_t peer = getPeer(arguments))
  {
    Dart_Handle ret = (((PeerT*)peer)->*method)(dartToNative<Args>(arguments, I + 1)...);
    Dart_SetReturnValue(arguments, HandleError(ret));
  }
  Dart_ExitScope();
}

template<typename T, typename Method, Method method, typename... Args>
void nativeCall(Dart_NativeArguments arguments)
{
  nativeCallImpl<T, Method, Args...>(arguments, method, std::make_index_sequence<sizeof...(Args)>{});
}

template<typename T, Dart_Handle(T::*Method)()>
void nativeCall(Dart_NativeArguments arguments)
{
  Dart_EnterScope();
  Dart_SetReturnValue(arguments, Dart_Null());
  if (intptr_t peer = getPeer(arguments))
  {
    Dart_Handle ret = (((T*)peer)->*Method)();
    Dart_SetReturnValue(arguments, HandleError(ret));
  }
  Dart_ExitScope();
}

template <typename T>
static void releasePeer(void* isolate_callback_data, Dart_WeakPersistentHandle handle, void* peer)
{
  ((T*)peer)->release();
}

template<typename T>
struct has_dart_ext_init
{
private:
  template <class, class> class checker;

  template <typename C>
  static std::true_type detect(checker<C, decltype(&C::dart_ext_init)> *);

  template <typename C>
  static std::false_type detect(...);

  typedef decltype(detect<T>(nullptr)) type;

public:
  static const bool value = std::is_same<std::true_type, decltype(detect<T>(nullptr))>::value;
};

template <typename T, bool HasDartExtInit>
struct DartInstance {};

template <typename T>
struct DartInstance<T, true>
{
  static void init(T *instance)
  {
    instance->dart_ext_init();
  }
};

template <typename T>
struct DartInstance<T, false>
{
  static void init(T *instance) {}
};

template <typename T>
inline void newInstance(Dart_NativeArguments arguments)
{
  Dart_EnterScope();

  Dart_SetReturnValue(arguments, Dart_Null());

  intptr_t peer = 0;
  Dart_Handle dartThis = HandleError(Dart_GetNativeArgument(arguments, 0));
  Dart_GetNativeInstanceField(dartThis, 0, &peer);

  if (!peer)
  {
    T *instance = new T;
    DartInstance<T, has_dart_ext_init<T>::value>::init(instance);

    std::cout << "createPeer: " << instance << std::endl;

    Dart_NewWeakPersistentHandle(dartThis, reinterpret_cast<void*>(instance), sizeof(*instance), releasePeer<T>);
    HandleError(Dart_SetNativeInstanceField(dartThis, 0, (intptr_t)instance));
  }
  else
  {
    std::cout << "reusePeer: " << (T*)peer << std::endl;
  }

  Dart_ExitScope();
}