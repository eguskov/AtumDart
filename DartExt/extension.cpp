#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <vector>
#include <algorithm>
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <array>
#include <utility>

#include <dart_api.h>
#include <dart_native_api.h>

#include <time.h>
#include <windows.h>

#include "Services/Render/Render.h"
#include "Services/Physics/Physics.h"
#include "Services/Controls/Controls.h"
#include "Services/Scene/SceneObject.h"
#include "Support/Timer.h"

#define DLL_EXPORT DART_EXPORT __declspec(dllexport)
#define PULL_DECL(n) struct Pull##n\
{\
  Pull##n()\
  {\
    const char* name = ((ClassFactorySceneObject*)&n::classFactory##n##decl)->GetName(); \
  }\
}; Pull##n pull##n;\

#include "SceneObjects/terrain.h"
#include "SceneObjects/tank.h"
#include "SceneObjects/PhysBox.h"

Dart_NativeFunction ResolveName(Dart_Handle name, int argc, bool* auto_setup_scope);

DLL_EXPORT Dart_Handle AtumExt_Init(Dart_Handle parent_library)
{
  if (Dart_IsError(parent_library))
  {
    return parent_library;
  }


  Dart_Handle cls = Dart_CreateNativeWrapperClass(parent_library, Dart_NewStringFromCString("NativeClass1"), 1);

  Dart_Handle result_code = Dart_SetNativeResolver(parent_library, ResolveName, NULL);
  return Dart_IsError(result_code) ? result_code : Dart_Null();
}

static inline Dart_Handle HandleError(Dart_Handle handle)
{
  if (Dart_IsError(handle))
  {
    Dart_PropagateError(handle);
  }
  return handle;
}

void SystemRand(Dart_NativeArguments arguments) {
  Dart_EnterScope();
  Dart_Handle result = HandleError(Dart_NewInteger(rand()));
  Dart_SetReturnValue(arguments, result);
  Dart_ExitScope();
}

void SystemSrand(Dart_NativeArguments arguments) {
  Dart_EnterScope();
  bool success = false;
  Dart_Handle seed_object = HandleError(Dart_GetNativeArgument(arguments, 0));
  if (Dart_IsInteger(seed_object)) {
    bool fits;
    HandleError(Dart_IntegerFitsIntoInt64(seed_object, &fits));
    if (fits) {
      int64_t seed;
      HandleError(Dart_IntegerToInt64(seed_object, &seed));
      srand(static_cast<unsigned>(seed));
      success = true;
    }
  }
  Dart_SetReturnValue(arguments, HandleError(Dart_NewBoolean(success)));
  Dart_ExitScope();
}

uint8_t* randomArray(int seed, int length)
{
  if (length <= 0 || length > 10000000)
  {
    return NULL;
  }

  uint8_t* values = reinterpret_cast<uint8_t*>(malloc(length));
  if (NULL == values)
  {
    return NULL;
  }

  srand(seed);
  for (int i = 0; i < length; ++i)
  {
    values[i] = rand() % 256;
  }
  return values;
}

void wrappedRandomArray(Dart_Port dest_port_id, Dart_CObject* message)
{
  Dart_Port reply_port_id = ILLEGAL_PORT;
  if (message->type == Dart_CObject_kArray && 3 == message->value.as_array.length)
  {
    // Use .as_array and .as_int32 to access the data in the Dart_CObject.
    Dart_CObject* param0 = message->value.as_array.values[0];
    Dart_CObject* param1 = message->value.as_array.values[1];
    Dart_CObject* param2 = message->value.as_array.values[2];
    if (param0->type == Dart_CObject_kInt32 &&
      param1->type == Dart_CObject_kInt32 &&
      param2->type == Dart_CObject_kSendPort) {
      int seed = param0->value.as_int32;
      int length = param1->value.as_int32;
      reply_port_id = param2->value.as_send_port.id;
      uint8_t* values = randomArray(seed, length);

      if (values != NULL) {
        Dart_CObject result;
        result.type = Dart_CObject_kTypedData;
        result.value.as_typed_data.type = Dart_TypedData_kUint8;
        result.value.as_typed_data.values = values;
        result.value.as_typed_data.length = length;
        Dart_PostCObject(reply_port_id, &result);
        free(values);
        // It is OK that result is destroyed when function exits.
        // Dart_PostCObject has copied its data.
        return;
      }
    }
  }
  Dart_CObject result;
  result.type = Dart_CObject_kNull;
  Dart_PostCObject(reply_port_id, &result);
}

void randomArrayServicePort(Dart_NativeArguments arguments)
{
  Dart_EnterScope();
  Dart_SetReturnValue(arguments, Dart_Null());
  Dart_Port service_port = Dart_NewNativePort("RandomArrayService", wrappedRandomArray, true);
  if (service_port != ILLEGAL_PORT)
  {
    Dart_Handle send_port = HandleError(Dart_NewSendPort(service_port));
    Dart_SetReturnValue(arguments, send_port);
  }
  Dart_ExitScope();
}

intptr_t getPeer(Dart_NativeArguments arguments)
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
  static std::vector<NativeFunction> functions;

  template <size_t N>
  static void push(const std::array<NativeFunction, N>& arr)
  {
    for (auto& f : arr)
      functions.push_back(f);
  }

  static Dart_NativeFunction find(const char* name)
  {
    for (auto& f : functions)
      if (!::strcmp(name, f.name))
        return f.function;
    return nullptr;
  }
};

decltype(NativeLookup::functions) NativeLookup::functions;

#define STR(a) #a
#define CONCAT(a, b) STR(a) STR(::) STR(b)
#define NATIVE_METHOD(cls, method) NativeFunction(CONCAT(cls, method), nativeCall<cls, &cls::method>)
#define NATIVE_METHOD_WITH_ARGS(cls, method, ...) NativeFunction(CONCAT(cls, method), nativeCall<cls, __VA_ARGS__, &cls::method>)
#define NATIVE_FUNC(cls, func) NativeFunction(CONCAT(cls, func), cls::func)
#define NATIVE_CTOR(cls) NativeFunction(CONCAT(cls, cls), newInstance<cls>)
#define NATIVE_METHOD_LIST(cls, ...) \
  static std::array<NativeFunction, argsCount(__VA_ARGS__)> native##cls = { { __VA_ARGS__ } }; \
  struct native##cls##_pushToNativeLookup { native##cls##_pushToNativeLookup() { NativeLookup::push(native##cls); }} native##cls##_push;

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

template<typename T>
static inline void nativeArgumentValue(Dart_Handle argHandle, T& argValue)
{
}

template<>
static inline void nativeArgumentValue(Dart_Handle argHandle, const char*& argValue)
{
  Dart_StringToCString(argHandle, &argValue);
}

template<>
static inline void nativeArgumentValue(Dart_Handle argHandle, Dart_Handle& argValue)
{
  argValue = argHandle;
}

template<>
static inline void nativeArgumentValue(Dart_Handle argHandle, int& argValue)
{
  int64_t v = 0;
  Dart_IntegerToInt64(argHandle, &v);
  argValue = v;
}

template<typename T>
static inline typename T::type nativeArgument(Dart_NativeArguments arguments)
{
  size_t i = typename T::index;

  Dart_Handle handle = Dart_GetNativeArgument(arguments, typename T::index);
  T::type value;
  nativeArgumentValue(handle, value);
  return value;
}

template <typename T, size_t Idx>
struct _Arg
{
  typedef T type;
  static const size_t index = Idx;
};

template<typename T, typename Method, typename... Args>
void nativeCallInner(Dart_NativeArguments arguments, const Method& method)
{
  Dart_EnterScope();
  Dart_SetReturnValue(arguments, Dart_Null());
  if (intptr_t peer = getPeer(arguments))
  {
    Dart_Handle ret = (((T*)peer)->*method)(nativeArgument<Args>(arguments)...);
    Dart_SetReturnValue(arguments, HandleError(ret));
  }
  Dart_ExitScope();
}

template<typename T, typename Arg1, Dart_Handle(T::*Method)(Arg1 arg)>
void nativeCall(Dart_NativeArguments arguments)
{
  nativeCallInner<T, decltype(Method), _Arg<Arg1, 1>>(arguments, Method);
}

template<typename T, typename Arg1, typename Arg2, Dart_Handle(T::*Method)(Arg1 arg1, Arg2 arg2)>
void nativeCall(Dart_NativeArguments arguments)
{
  nativeCallInner<T, decltype(Method), _Arg<Arg1, 1>, _Arg<Arg2, 2>>(arguments, Method);
}

template <typename T>
static void releasePeer(void* isolate_callback_data, Dart_WeakPersistentHandle handle, void* peer)
{
  ((T*)peer)->release();
}

template <typename T>
void newInstance(Dart_NativeArguments arguments)
{
  Dart_EnterScope();

  Dart_SetReturnValue(arguments, Dart_Null());

  intptr_t peer = 0;
  Dart_Handle dartThis = HandleError(Dart_GetNativeArgument(arguments, 0));
  Dart_GetNativeInstanceField(dartThis, 0, &peer);

  if (!peer)
  {
    T *instance = new T;
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

template <typename T>
Dart_Handle newObject(Dart_Handle type, T* instance)
{
  intptr_t peer = (intptr_t)instance;
  Dart_Handle object = HandleError(Dart_AllocateWithNativeFields(type, 1, &peer));
  Dart_NewWeakPersistentHandle(object, reinterpret_cast<void*>(instance), sizeof(*instance), releasePeer<T>);

  return HandleError(Dart_InvokeConstructor(object, Dart_Null(), 0, nullptr));
}

template <typename T>
Dart_Handle newObjectRef(Dart_Handle type, T* instance)
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

class AtumSceneObject : public RefCount
{
public:
  SceneObject* object = nullptr;

  Dart_Handle cast(Dart_Handle type)
  {
    return newObjectRef(type, this);
  }

  Dart_Handle getTrans()
  {
    Matrix& m = object->Trans();

    Dart_Handle h = Dart_RootLibrary();
    Dart_Handle hm = HandleError(Dart_GetClass(h, Dart_NewStringFromCString("Matrix4")));

    Dart_Handle list = Dart_NewTypedData(Dart_TypedData_kFloat32, 16);

    Dart_TypedData_Type data_type_unused;
    void* data_location;
    intptr_t data_length_unused;
    Dart_TypedDataAcquireData(list, &data_type_unused, &data_location, &data_length_unused);

    ::memcpy(data_location, m.matrix, sizeof(float) * 16);
    Dart_TypedDataReleaseData(list);

    Dart_Handle matObject = HandleError(Dart_Allocate(hm));
    return HandleError(Dart_InvokeConstructor(matObject, Dart_NewStringFromCString("fromFloat32List"), 1, &list));
  }

  Dart_Handle setTrans(Dart_Handle mat4)
  {
    Dart_TypedData_Type data_type_unused;
    void* data_location;
    intptr_t data_length_unused;
    Dart_TypedDataAcquireData(mat4, &data_type_unused, &data_location, &data_length_unused);

    Matrix& m = object->Trans();

    ::memcpy(m.matrix, data_location, sizeof(float) * 16);
    Dart_TypedDataReleaseData(mat4);

    return Dart_Null();
  }

  Dart_Handle getName()
  {
    return Dart_NewStringFromCString(object->GetName());
  }

  Dart_Handle getClassName()
  {
    return Dart_NewStringFromCString(object->GetClassName());
  }
};

NATIVE_METHOD_LIST(AtumSceneObject,
  NATIVE_CTOR(AtumSceneObject),
  NATIVE_METHOD_WITH_ARGS(AtumSceneObject, cast, Dart_Handle),
  NATIVE_METHOD(AtumSceneObject, getTrans),
  NATIVE_METHOD_WITH_ARGS(AtumSceneObject, setTrans, Dart_Handle),
  NATIVE_METHOD(AtumSceneObject, getName),
  NATIVE_METHOD(AtumSceneObject, getClassName));

class AtumTank : public AtumSceneObject
{
public:
  Dart_Handle getAngles()
  {
    if (!object->Playing())
      return Dart_Null();

    ((Tank*)object)->angles;

    Dart_Handle h = Dart_RootLibrary();
    Dart_Handle hm = HandleError(Dart_GetClass(h, Dart_NewStringFromCString("Vector3")));

    Dart_Handle list = Dart_NewTypedData(Dart_TypedData_kFloat32, 3);

    Dart_TypedData_Type data_type_unused;
    void* data_location;
    intptr_t data_length_unused;
    Dart_TypedDataAcquireData(list, &data_type_unused, &data_location, &data_length_unused);

    ::memcpy(data_location, ((Tank*)object)->angles.v, sizeof(Vector));
    Dart_TypedDataReleaseData(list);

    Dart_Handle matObject = HandleError(Dart_Allocate(hm));
    return HandleError(Dart_InvokeConstructor(matObject, Dart_NewStringFromCString("fromFloat32List"), 1, &list));
  }

  Dart_Handle setAngles(Dart_Handle vec3)
  {
    if (!object->Playing())
      return Dart_Null();

    Dart_TypedData_Type data_type_unused;
    void* data_location;
    intptr_t data_length_unused;
    Dart_TypedDataAcquireData(vec3, &data_type_unused, &data_location, &data_length_unused);

    ::memcpy(((Tank*)object)->angles.v, data_location, sizeof(Vector));

    Dart_TypedDataReleaseData(vec3);
  }

  Dart_Handle move(Dart_Handle vec3)
  {
    if (!object->Playing())
      return Dart_Null();

    Dart_TypedData_Type data_type_unused;
    void* data_location;
    intptr_t data_length_unused;
    Dart_TypedDataAcquireData(vec3, &data_type_unused, &data_location, &data_length_unused);

    Vector dir;
    ::memcpy(dir.v, data_location, sizeof(Vector));
    ((Tank*)object)->controller->Move(dir);

    Dart_TypedDataReleaseData(vec3);

    return Dart_Null();
  }
};

NATIVE_METHOD_LIST(AtumTank,
  NATIVE_CTOR(AtumTank),
  NATIVE_METHOD(AtumTank, getAngles),
  NATIVE_METHOD_WITH_ARGS(AtumTank, setAngles, Dart_Handle),
  NATIVE_METHOD_WITH_ARGS(AtumTank, move, Dart_Handle));

class AtumScene : public RefCount
{
public:
  Scene* scene = nullptr;

  ~AtumScene()
  {
    delete scene;
  }

  Dart_Handle addObject(Dart_Handle scene_object_type, const char* name)
  {
    AtumSceneObject* object = new AtumSceneObject;
    object->object = scene->AddObject(name);
    return newObject(scene_object_type, object);
  }

  Dart_Handle play()
  {
    scene->Play();
    return Dart_Null();
  }

  Dart_Handle load(const char* name)
  {
    scene->Load(name);
    return Dart_Null();
  }

  Dart_Handle getObject(Dart_Handle scene_object_type, int index)
  {
    SceneObject* sceneObject = scene->GetObj(index);
    AtumSceneObject* object = nullptr;

    if (!::strcmp(sceneObject->GetClassName(), "Tank"))
      object = new AtumTank;
    else
      object = new AtumSceneObject;

    object->object = sceneObject;
    return newObject(scene_object_type, object);
  }

  Dart_Handle getObjectsCount()
  {
    return Dart_NewInteger(scene->GetObjectsCount());
  }
};

NATIVE_METHOD_LIST(AtumScene,
  NATIVE_CTOR(AtumScene),
  NATIVE_METHOD_WITH_ARGS(AtumScene, addObject, Dart_Handle, const char*),
  NATIVE_METHOD_WITH_ARGS(AtumScene, load, const char*),
  NATIVE_METHOD_WITH_ARGS(AtumScene, getObject, Dart_Handle, int),
  NATIVE_METHOD(AtumScene, getObjectsCount),
  NATIVE_METHOD(AtumScene, play));

class AtumCore : public Object, public RefCount
{
  EUIWindow* mainWnd;

  Scene* currentScene = nullptr;
  TaskExecutor::SingleTaskPool* renderTaskPool;

  class Listener : public EUIWidget::Listener
  {
    AtumCore& owner;

  public:
    Listener(AtumCore& owner) : owner(owner) {}

    virtual void OnMouseMove(EUIWidget* sender, int mx, int my)
    {
      if (sender == owner.mainWnd)
      {
        // controls.OverrideMousePos(mx, my);
      }
    }

    virtual void OnLeftMouseDown(EUIWidget* sender, int mx, int my) {}
    virtual void OnLeftMouseUp(EUIWidget* sender, int mx, int my) {}
    virtual void OnRightMouseUp(EUIWidget* sender, int mx, int my) {}
    virtual void OnMenuItem(EUIWidget* sender, int id) {}
    virtual void OnUpdate(EUIWidget* sender) {}
    virtual void OnListBoxChange(EUIWidget* sender, int index) {}
    virtual void OnEditBoxChange(EUIWidget* sender) {}
    virtual void OnResize(EUIWidget* sender) {}
    virtual void OnWinClose(EUIWidget* sender) {}
  } listener;

  std::thread* uiThread = nullptr;

  struct UIMessage
  {
    uint32_t wndMessage = 0;
    union
    {
      struct
      {
        int width;
        int height;
      } asResize;

      struct
      {
        int x;
        int y;
      } asMousePos;

    } value;
  };

  std::mutex uiEventsLock;
  std::queue<UIMessage> uiEvents;

  std::atomic<bool> ready = false;
  std::atomic<bool> done = false;

public:
  AtumCore() : listener(*this) {}

  static LRESULT CALLBACK wndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
  {
    AtumCore* core = nullptr;

    LONG_PTR corePtr = ::GetWindowLongPtr(hwnd, GWLP_USERDATA);
    if (corePtr)
      core = (AtumCore*)corePtr;

    if (msg == WM_SIZE)
    {
      UIMessage m;
      m.wndMessage = msg;
      m.value.asResize.width = LOWORD(lParam);
      m.value.asResize.height = HIWORD(lParam);

      std::unique_lock<std::mutex> lock(core->uiEventsLock);
      core->uiEvents.push(m);
    }
    else if (msg == WM_MOUSEMOVE)
    {
      UIMessage m;
      m.wndMessage = msg;
      m.value.asMousePos.x = LOWORD(lParam);
      m.value.asMousePos.y = HIWORD(lParam);

      std::unique_lock<std::mutex> lock(core->uiEventsLock);
      core->uiEvents.push(m);
    }
    else if (msg == WM_QUIT || msg == WM_DESTROY || msg == WM_CLOSE)
    {
      UIMessage m;
      m.wndMessage = msg;

      std::unique_lock<std::mutex> lock(core->uiEventsLock);
      core->uiEvents.push(m);
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
  }

  static void handleUIThread(AtumCore* core)
  {
    core->mainWnd = new EUIWindow("Editor", false, true, 30, 30, 800, 600);
    // core->mainWnd->SetListener(&listener, 0);

    {
      UIMessage m;
      m.wndMessage = WM_CREATE;

      std::unique_lock<std::mutex> lock(core->uiEventsLock);
      core->uiEvents.push(m);
    }

    HWND hwnd = *((HWND*)core->mainWnd->GetNative());

    ::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&AtumCore::wndProc);
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)core);

    core->mainWnd->Show(true);
    core->mainWnd->SetFocused();
    // mainWnd->Maximaze();

    while (true)
    {
      MSG msg;

      while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }

  void release()
  {
    done = true;
    Sleep(500);
  }

  Dart_Handle init()
  {
    PULL_DECL(Terrain);
    PULL_DECL(PhysBox);
    PULL_DECL(Tank);

    std::cout << __FUNCTION__ << std::endl;

    EUI::Init("settings/EUI/theme.dat");

    uiThread = new std::thread(handleUIThread, this);

    UIMessage m;
    while (!nextUIMessage(m) || m.wndMessage != WM_CREATE)
    {
      Sleep(50);
    }

    if (m.wndMessage == WM_CREATE)
    {
      controls.Init(mainWnd->GetNative(), "settings/controls/hardware_pc", "settings/controls/user_pc");
      render.Init("DX11", mainWnd->GetWidth(), mainWnd->GetHeight(), mainWnd->GetNative());

      render.AddExecutedLevelPool(1);

      renderTaskPool = render.AddTaskPool();
      renderTaskPool->AddTask(1, this, (Object::Delegate)&AtumCore::draw);

      physics.Init();
      //scene.Init();

      //scene.Load("Media/Scene.scn");
      //scene.Play();

      ClassFactorySceneObject* decl = ClassFactorySceneObject::First();
      while (decl)
      {
        std::cout << decl->GetName() << std::endl;
        decl = decl->Next();
      }

      ready = true;
    }

    return Dart_Null();
  }

  bool nextUIMessage(UIMessage& m)
  {
    std::unique_lock<std::mutex> lock(uiEventsLock);

    if (uiEvents.empty())
      return false;

    m = uiEvents.front();
    uiEvents.pop();

    return true;
  }

  Dart_Handle update()
  {
    if (!ready || done)
      return Dart_NewDouble(done ? -1.f : 0.f);

    UIMessage m;
    while (nextUIMessage(m))
    {
      if (m.wndMessage == WM_SIZE)
      {
        render.GetDevice()->SetVideoMode(m.value.asResize.width, m.value.asResize.height, mainWnd->GetNative());
      }
      else if (m.wndMessage == WM_MOUSEMOVE)
      {
        controls.OverrideMousePos(m.value.asMousePos.x, m.value.asMousePos.y);
      }
      else if (m.wndMessage == WM_QUIT || m.wndMessage == WM_DESTROY || m.wndMessage == WM_CLOSE)
      {
        done = true;
        return Dart_NewDouble(-1.f);
      }
    }

    float dt = Timer::CountDeltaTime();

    physics.Update(dt);
    if (currentScene)
      currentScene->Execute(dt);
    render.Execute(dt);
    controls.Update(dt);

    physics.Fetch();

    return Dart_NewDouble(dt);
  }

  Dart_Handle addScene(Dart_Handle scene_type)
  {
    AtumScene* scene = new AtumScene;

    scene->scene = new Scene;
    scene->scene->Init();

    currentScene = scene->scene;

    return newObject(scene_type, scene);
  }

  Dart_Handle controlsGetAlias(const char* name)
  {
    return Dart_NewInteger(controls.GetAlias(name));
  }

  Dart_Handle controlsIsDebugKeyActive(const char* name)
  {
    return Dart_NewBoolean(controls.DebugKeyPressed(name, Controls::Active));
  }

  Dart_Handle controlsIsDebugKeyPressed(const char* name)
  {
    return Dart_NewBoolean(controls.DebugKeyPressed(name, Controls::Activated));
  }

  void draw(float dt)
  {
    if (!ready)
      return;

    //std::cout << __FUNCTION__ << " dt= " << dt << std::endl;

    render.GetDevice()->Clear(true, COLOR_GRAY, true, 1.0f);

    render.ExecutePool(0, dt);
    render.ExecutePool(1000, dt);

    render.GetDevice()->Present();
  }

  static void servicePortHandler(Dart_Port dest_port_id, Dart_CObject* message)
  {
    Dart_Port reply_port_id = ILLEGAL_PORT;
    if (message->type == Dart_CObject_kArray && 3 == message->value.as_array.length)
    {
      // Use .as_array and .as_int32 to access the data in the Dart_CObject.
      Dart_CObject* param0 = message->value.as_array.values[0];
      Dart_CObject* param1 = message->value.as_array.values[1];
      Dart_CObject* param2 = message->value.as_array.values[2];
      if (param0->type == Dart_CObject_kInt32 &&
        param1->type == Dart_CObject_kInt64 &&
        param2->type == Dart_CObject_kSendPort) {
        int command = param0->value.as_int32;
        uint64_t peer = param1->value.as_int64;
        reply_port_id = param2->value.as_send_port.id;

        bool success = false;

        AtumCore* core = (AtumCore*)peer;
        if (command == 1)
        {
          success = true;
          core->init();
        }
        else if (command == 2)
        {
          success = true;
          core->update();
        }

        Dart_CObject result;
        result.type = Dart_CObject_kBool;
        result.value.as_bool = success;
        Dart_PostCObject(reply_port_id, &result);

        // uint8_t* values = randomArray(seed, length);

        /*if (values != NULL) {
          Dart_CObject result;
          result.type = Dart_CObject_kTypedData;
          result.value.as_typed_data.type = Dart_TypedData_kUint8;
          result.value.as_typed_data.values = values;
          result.value.as_typed_data.length = length;
          Dart_PostCObject(reply_port_id, &result);
          free(values);
          return;
        }*/
      }
    }

    Dart_CObject result;
    result.type = Dart_CObject_kNull;
    Dart_PostCObject(reply_port_id, &result);
  }

  static void servicePort(Dart_NativeArguments arguments)
  {
    Dart_EnterScope();
    Dart_SetReturnValue(arguments, Dart_Null());
    Dart_Port service_port = Dart_NewNativePort("AtumService", servicePortHandler, true);
    if (service_port != ILLEGAL_PORT)
      Dart_SetReturnValue(arguments, HandleError(Dart_NewSendPort(service_port)));
    Dart_ExitScope();
  }
};

NATIVE_METHOD_LIST(AtumCore,
  NATIVE_CTOR(AtumCore),
  NATIVE_METHOD(AtumCore, init),
  NATIVE_METHOD(AtumCore, update),
  NATIVE_METHOD_WITH_ARGS(AtumCore, addScene, Dart_Handle),
  NATIVE_METHOD_WITH_ARGS(AtumCore, controlsGetAlias, const char*),
  NATIVE_METHOD_WITH_ARGS(AtumCore, controlsIsDebugKeyActive, const char*),
  NATIVE_METHOD_WITH_ARGS(AtumCore, controlsIsDebugKeyPressed, const char*),
  NATIVE_FUNC(AtumCore, servicePort));

void wrappedTestDoSomethingAsync(Dart_Port dest_port_id, Dart_CObject* message)
{
  Dart_Port reply_port_id = ILLEGAL_PORT;
  if (message->type == Dart_CObject_kArray && 3 == message->value.as_array.length)
  {
    // Use .as_array and .as_int32 to access the data in the Dart_CObject.
    Dart_CObject* param0 = message->value.as_array.values[0];
    Dart_CObject* param1 = message->value.as_array.values[1];
    Dart_CObject* param2 = message->value.as_array.values[2];
    if (param0->type == Dart_CObject_kInt32 &&
      param1->type == Dart_CObject_kInt32 &&
      param2->type == Dart_CObject_kSendPort) {
      int seed = param0->value.as_int32;
      int length = param1->value.as_int32;
      reply_port_id = param2->value.as_send_port.id;
      uint8_t* values = randomArray(seed, length);

      if (values != NULL) {
        Dart_CObject result;
        result.type = Dart_CObject_kTypedData;
        result.value.as_typed_data.type = Dart_TypedData_kUint8;
        result.value.as_typed_data.values = values;
        result.value.as_typed_data.length = length;
        Dart_PostCObject(reply_port_id, &result);
        free(values);
        // It is OK that result is destroyed when function exits.
        // Dart_PostCObject has copied its data.
        return;
      }
    }
  }
  Dart_CObject result;
  result.type = Dart_CObject_kNull;
  Dart_PostCObject(reply_port_id, &result);
}

struct FunctionLookup {
  const char* name;
  Dart_NativeFunction function;
};

FunctionLookup function_list[] =
{
  { "SystemRand", SystemRand },
  { "SystemSrand", SystemSrand },
  { "RandomArray_ServicePort", randomArrayServicePort },
  { NULL, NULL }
};

FunctionLookup no_scope_function_list[] =
{
  { "NoScopeSystemRand", SystemRand },
  { NULL, NULL }
};

Dart_NativeFunction ResolveName(Dart_Handle name, int argc, bool* auto_setup_scope)
{
  if (!Dart_IsString(name))
  {
    return NULL;
  }

  Dart_NativeFunction result = NULL;
  if (auto_setup_scope == NULL)
  {
    return NULL;
  }

  Dart_EnterScope();

  const char* cname;
  HandleError(Dart_StringToCString(name, &cname));

  for (int i = 0; function_list[i].name != NULL; ++i)
  {
    if (strcmp(function_list[i].name, cname) == 0)
    {
      *auto_setup_scope = true;
      result = function_list[i].function;
      break;
    }
  }

  if (!result)
    result = NativeLookup::find(cname);

  if (result != NULL)
  {
    Dart_ExitScope();
    return result;
  }

  for (int i = 0; no_scope_function_list[i].name != NULL; ++i)
  {
    if (strcmp(no_scope_function_list[i].name, cname) == 0)
    {
      *auto_setup_scope = false;
      result = no_scope_function_list[i].function;
      break;
    }
  }

  Dart_ExitScope();

  return result;
}