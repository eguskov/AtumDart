
part of atum;

class Scene extends _Native {
  Load(String name) native "AtumExt_Scene::Load";
  DeleteObject(SceneObject obj) native "AtumExt_Scene::DeleteObject";
  DelFromGroup(SceneObject obj, String name) native "AtumExt_Scene::DelFromGroup";
  Play() native "AtumExt_Scene::Play";
  SceneObject _AddObject() native "AtumExt_Scene::AddObject";
  Init() native "AtumExt_Scene::Init";
  SceneObject _Find() native "AtumExt_Scene::Find";
  int _GetObjectsCount() native "AtumExt_Scene::GetObjectsCount";
  SceneObject _GetObj() native "AtumExt_Scene::GetObj";
  DeleteAllObjects() native "AtumExt_Scene::DeleteAllObjects";
  Save(String name) native "AtumExt_Scene::Save";
  Execute(double dt) native "AtumExt_Scene::Execute";
  Stop() native "AtumExt_Scene::Stop";
  DelFromAllGroups(SceneObject obj) native "AtumExt_Scene::DelFromAllGroups";
  bool Playing() native "AtumExt_Scene::Playing";
  AddToGroup(SceneObject obj, String name) native "AtumExt_Scene::AddToGroup";
  int get ObjectsCount => _GetObjectsCount();
  SceneObject get Obj => _GetObj();

  _ctor() native "AtumExt_Scene::AtumExt_Scene";

  Scene() {
    _ctor();
  }
}

