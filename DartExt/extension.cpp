#define _WINSOCKAPI_
#include <windows.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <algorithm>
#include <thread>
#include <queue>
#include <mutex>

#include "dartHelpers.h"

// #undef NATIVE_METHOD_LIST
// #define NATIVE_METHOD_LIST(...)

#include "Services/Render/Render.h"
#include "Services/Physics/Physics.h"
#include "Services/Controls/Controls.h"
#include "Services/Scene/SceneObject.h"
#include "Support/Timer.h"

#include "SceneObjects/terrain.h"
#include "SceneObjects/tank.h"
#include "SceneObjects/PhysBox.h"

#define DLL_EXPORT DART_EXPORT __declspec(dllexport)
#define PULL_DECL(n) struct Pull##n\
{\
  Pull##n()\
  {\
    const char* name = ((ClassFactorySceneObject*)&n::classFactory##n##decl)->GetName(); \
    n::meta_data.Init(); \
    OutputDebugString(name); \
    OutputDebugString("\n"); \
  }\
};\
Pull##n pull##n;\

#include "gen/Scene.gen.h"

decltype(NativeLookup::functions) NativeLookup::functions = nullptr;

Dart_NativeFunction ResolveName(Dart_Handle name, int argc, bool* auto_setup_scope);

DLL_EXPORT Dart_Handle AtumExt_Init(Dart_Handle parent_library)
{
  if (Dart_IsError(parent_library))
  {
    return parent_library;
  }

  PULL_DECL(Terrain);
  PULL_DECL(PhysBox);
  PULL_DECL(Tank);

  Dart_Handle cls = Dart_CreateNativeWrapperClass(parent_library, Dart_NewStringFromCString("NativeClass1"), 1);

  Dart_Handle result_code = Dart_SetNativeResolver(parent_library, ResolveName, NULL);
  return Dart_IsError(result_code) ? result_code : Dart_Null();
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

class AtumScene : public RefCount
{
public:
  Scene* scene = nullptr;

  AtumScene() {}
  ~AtumScene()
  {
    delete scene;
  }

  //@ export start
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

  static Dart_Handle getCorrectInstance(Dart_Handle scene_object_type, SceneObject* scene_object)
  {
    /*AtumSceneObject* object = nullptr;
    if (!::strcmp(scene_object->GetClassName(), "Tank"))
      object = new AtumTank;
    else
      object = new AtumSceneObject;
    object->object = scene_object;
    return newObject(scene_object_type, object);*/
    return Dart_Null();
  }

  Dart_Handle addObject(Dart_Handle scene_object_type, const char* name)
  {
    return getCorrectInstance(scene_object_type, scene->AddObject(name));
  }

  Dart_Handle getObject(Dart_Handle scene_object_type, int index)
  {
    return getCorrectInstance(scene_object_type, scene->GetObj(index));
  }

  Dart_Handle getObjectsCount()
  {
    return nativeToDart(scene->GetObjectsCount());
  }
  //@ export end
};

NATIVE_METHOD_LIST(AtumScene,
  NATIVE_METHOD(AtumScene, addObject, Dart_Handle, const char*),
  NATIVE_METHOD(AtumScene, load, const char*),
  NATIVE_METHOD(AtumScene, getObject, Dart_Handle, int),
  NATIVE_METHOD(AtumScene, getObjectsCount),
  NATIVE_METHOD(AtumScene, play));

class AtumPhysScene : public RefCount
{
public:
  PhysScene* scene = nullptr;

  AtumPhysScene() {}
  ~AtumPhysScene()
  {
    if (scene)
      physics.DestroyScene(scene);
  }

  Dart_Handle rayCast(Dart_Handle desc)
  {
    return Dart_Null();
  }

  Dart_Handle createController(Dart_Handle desc)
  {
    /*PhysControllerDesc cdesc;
    cdesc.height = 1.0f;
    cdesc.radius = 1.0f;
    cdesc.pos = transform.Pos();
    cdesc.slopeLimit = cosf(RADIAN * 60.0f);

    controller = pscene->CreateController(cdesc);*/

    return Dart_Null();
  }

  Dart_Handle createHeightmap(Dart_Handle desc)
  {
    /*PhysHeightmapDesc hdesc;
    hdesc.width = terrain->hwidth;
    hdesc.height = terrain->hheight;
    hdesc.hmap = terrain->hmap;
    hdesc.scale = Vector2(terrain->scaleh, terrain->scalev);

    hm = pscene->CreateHeightmap(hdesc);*/

    return Dart_Null();
  }

  Dart_Handle createBox(Dart_Handle desc)
  {
    /*obj = pscene->CreateBox(Vector(boxes[i].box->sizeX * 0.5f, boxes[i].box->sizeY * 0.5f, boxes[i].box->sizeZ * 0.5f),
      boxes[i].box->Trans(), boxes[i].box->isStatic);*/

    return Dart_Null();
  }
};

NATIVE_METHOD_LIST(AtumPhysScene,
  NATIVE_METHOD(AtumPhysScene, rayCast, Dart_Handle));

class AtumCore : public Object, public RefCount
{
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

  struct SharedData
  {
    EUIWindow* mainWnd;

    std::vector<Scene*> scenes;
    std::vector<PhysScene*> physScenes;
    TaskExecutor::SingleTaskPool* renderTaskPool;

    std::thread* uiThread = nullptr;

    std::mutex critSec;
    std::mutex uiCritSec;
    std::queue<UIMessage> uiEvents;

    std::atomic<bool> ready = false;
    std::atomic<bool> done = false;
  };

  static SharedData sharedData;

public:
  AtumCore()
  {
  }

  virtual ~AtumCore()
  {
    sharedData.done = true;
    Sleep(500);
  }

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

      std::unique_lock<std::mutex> lock(core->sharedData.uiCritSec);
      core->sharedData.uiEvents.push(m);
    }
    else if (msg == WM_MOUSEMOVE)
    {
      UIMessage m;
      m.wndMessage = msg;
      m.value.asMousePos.x = LOWORD(lParam);
      m.value.asMousePos.y = HIWORD(lParam);

      std::unique_lock<std::mutex> lock(core->sharedData.uiCritSec);
      core->sharedData.uiEvents.push(m);
    }
    else if (msg == WM_QUIT || msg == WM_DESTROY || msg == WM_CLOSE)
    {
      UIMessage m;
      m.wndMessage = msg;

      std::unique_lock<std::mutex> lock(core->sharedData.uiCritSec);
      core->sharedData.uiEvents.push(m);
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
  }

  static void handleUIThread(AtumCore* core)
  {
    core->sharedData.mainWnd = new EUIWindow("Editor", false, true, 30, 30, 800, 600);
    // core->mainWnd->SetListener(&listener, 0);

    {
      UIMessage m;
      m.wndMessage = WM_CREATE;

      std::unique_lock<std::mutex> lock(core->sharedData.uiCritSec);
      core->sharedData.uiEvents.push(m);
    }

    HWND hwnd = *((HWND*)core->sharedData.mainWnd->GetNative());

    ::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)&AtumCore::wndProc);
    ::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)core);

    core->sharedData.mainWnd->Show(true);
    core->sharedData.mainWnd->SetFocused();
    // mainWnd->Maximaze();

    while (!core->sharedData.done)
    {
      MSG msg;

      while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE))
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }

  Dart_Handle init()
  {
    if (sharedData.uiThread)
      return Dart_Null();

    std::cout << __FUNCTION__ << std::endl;

    EUI::Init("settings/EUI/theme.dat");

    sharedData.uiThread = new std::thread(handleUIThread, this);

    UIMessage m;
    while (!nextUIMessage(m) || m.wndMessage != WM_CREATE)
    {
      Sleep(50);
    }

    if (m.wndMessage == WM_CREATE)
    {
      controls.Init("settings/controls/hardware_pc", true);
      render.Init("DX11", (int)sharedData.mainWnd->GetWidth(), (int)sharedData.mainWnd->GetHeight(), sharedData.mainWnd->GetNative());

      render.AddExecutedLevelPool(1);

      sharedData.renderTaskPool = render.AddTaskPool();
      sharedData.renderTaskPool->AddTask(1, this, (Object::Delegate)&AtumCore::draw);

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

      sharedData.ready = true;
    }

    return Dart_Null();
  }

  bool nextUIMessage(UIMessage& m)
  {
    std::unique_lock<std::mutex> lock(sharedData.uiCritSec);

    if (sharedData.uiEvents.empty())
      return false;

    m = sharedData.uiEvents.front();
    sharedData.uiEvents.pop();

    return true;
  }

  Dart_Handle update()
  {
    if (!sharedData.ready || sharedData.done)
      return Dart_NewDouble(sharedData.done ? -1.f : 0.f);

    UIMessage m;
    while (nextUIMessage(m))
    {
      if (m.wndMessage == WM_SIZE)
      {
        render.GetDevice()->SetVideoMode(m.value.asResize.width, m.value.asResize.height, sharedData.mainWnd->GetNative());
      }
      else if (m.wndMessage == WM_MOUSEMOVE)
      {
        controls.OverrideMousePos(m.value.asMousePos.x, m.value.asMousePos.y);
      }
      else if (m.wndMessage == WM_QUIT || m.wndMessage == WM_DESTROY || m.wndMessage == WM_CLOSE)
      {
        sharedData.done = true;
        return Dart_NewDouble(-1.f);
      }
    }

    float dt = Timer::CountDeltaTime();

    physics.Update(dt);
    for (auto scene : sharedData.scenes)
      scene->Execute(dt);
    render.Execute(dt);
    controls.Update(dt);

    physics.Fetch();

    return Dart_NewDouble(dt);
  }

  /*Dart_Handle addScene(Dart_Handle scene_type)
  {
    std::unique_lock<std::mutex> lock(sharedData.critSec);

    AtumScene* scene = new AtumScene;

    scene->scene = new Scene;
    scene->scene->Init();

    sharedData.scenes.push_back(scene->scene);

    return newObject(scene_type, scene);
  }*/

  Dart_Handle addScene(Scene *scene)
  {
    std::unique_lock<std::mutex> lock(sharedData.critSec);

    /*AtumScene* scene = new AtumScene;

    scene->scene = new Scene;
    scene->scene->Init();

    sharedData.scenes.push_back(scene->scene);*/

    Dart_Handle library = Dart_RootLibrary();
    Dart_Handle sceneClass = Dart_GetClass(library, Dart_NewStringFromCString("Scene"));

    return newObject(sceneClass, new AtumExt_Scene);
  }

  /*Dart_Handle addPhysScene(Dart_Handle scene_type)
  {
    std::unique_lock<std::mutex> lock(sharedData.critSec);

    AtumPhysScene* scene = new AtumPhysScene;

    scene->scene = physics.CreateScene();

    sharedData.physScenes.push_back(scene->scene);

    return newObject(scene_type, scene);
  }*/

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
    if (!sharedData.ready)
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

AtumCore::SharedData AtumCore::sharedData;

NATIVE_METHOD_LIST(AtumCore,
  NATIVE_METHOD(AtumCore, init),
  NATIVE_METHOD(AtumCore, update),
  NATIVE_METHOD(AtumCore, addScene, Scene*),
//  NATIVE_METHOD(AtumCore, addScene, Dart_Handle),
//  NATIVE_METHOD(AtumCore, addPhysScene, Dart_Handle),
  NATIVE_METHOD(AtumCore, controlsGetAlias, const char*),
  NATIVE_METHOD(AtumCore, controlsIsDebugKeyActive, const char*),
  NATIVE_METHOD(AtumCore, controlsIsDebugKeyPressed, const char*),
  NATIVE_FUNCON(AtumCore, servicePort));

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