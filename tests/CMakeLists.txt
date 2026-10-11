# SDK-independent unit tests (Faz 6 / roadmap "test kapsamı").
# Each test compiles only the framework sources under test — no RenderWare,
# Havok or platform SDK is required.
#
# Run from the code/ build tree:
#   cmake -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build -j8
#   ctest --test-dir build --output-on-failure

set(CODE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../code)

set(SDK_INCLUDE_DIRS
    ${CODE_DIR}
    ${CODE_DIR}/framework
    ${CODE_DIR}/framework/core
    ${CODE_DIR}/framework/toolkits
)

# add_sdk_test(<name> <extra sources...>)
function(add_sdk_test name)
    add_executable(${name} ${ARGN})
    target_include_directories(${name} PRIVATE ${SDK_INCLUDE_DIRS})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

add_sdk_test(havok_memory_manager_test
    ${CMAKE_CURRENT_SOURCE_DIR}/havok_memory_manager_test.cpp
    ${CODE_DIR}/HavokMemoryManager.cpp
)

add_sdk_test(ceventhandler_test
    ${CMAKE_CURRENT_SOURCE_DIR}/ceventhandler_test.cpp
    ${CODE_DIR}/framework/core/eventhandler/ceventhandler.cpp
)

add_sdk_test(cclassfactory_test
    ${CMAKE_CURRENT_SOURCE_DIR}/cclassfactory_test.cpp
    ${CODE_DIR}/framework/core/attributehandler/cclassfactory.cpp
)

add_sdk_test(cattributehandler_test
    ${CMAKE_CURRENT_SOURCE_DIR}/cattributehandler_test.cpp
    ${CODE_DIR}/framework/core/attributehandler/cattributehandler.cpp
    ${CODE_DIR}/framework/core/attributehandler/cclassfactory.cpp
)

add_sdk_test(fontsystem_test
    ${CMAKE_CURRENT_SOURCE_DIR}/fontsystem_test.cpp
    ${CODE_DIR}/FontSystem.cpp
    ${CODE_DIR}/rwstubs.cpp
)

add_sdk_test(renderpipeline_test
    ${CMAKE_CURRENT_SOURCE_DIR}/renderpipeline_test.cpp
    ${CODE_DIR}/RenderPipeline.cpp
)

# Faz 4.6 — Bullet physics backend (Havok yerine); binary parametreleri
# gravity -9.8 / friction 0.6 / restitution 0.3 / adım 1/60 doğrulanır.
add_sdk_test(physics_test
    ${CMAKE_CURRENT_SOURCE_DIR}/physics_test.cpp
    ${CODE_DIR}/PhysicsBackend.cpp
)
# Bullet include'ları hedeflere yayılmaz → elle
target_include_directories(physics_test PRIVATE ${bullet3_SOURCE_DIR}/src)
target_link_libraries(physics_test PRIVATE
    BulletDynamics BulletCollision LinearMath)

# Faz 4.6 — entity/trigger akışı: CPhysicsEntity erişimcileri + AddToWorld
# singleton kaydı; CCollisionTrigger phantom + PollOverlaps enter/leave.
add_sdk_test(physics_entity_test
    ${CMAKE_CURRENT_SOURCE_DIR}/physics_entity_test.cpp
    ${CODE_DIR}/PhysicsEntity.cpp
    ${CODE_DIR}/PhysicsWorld.cpp
    ${CODE_DIR}/CollisionTrigger.cpp
    ${CODE_DIR}/PhysicsBackend.cpp
    ${CODE_DIR}/framework/core/eventhandler/ceventhandler.cpp
    ${CODE_DIR}/framework/core/attributehandler/cclassfactory.cpp
    ${CODE_DIR}/rwstubs.cpp
)
target_include_directories(physics_entity_test PRIVATE ${bullet3_SOURCE_DIR}/src)
target_link_libraries(physics_entity_test PRIVATE
    BulletDynamics BulletCollision LinearMath)

# Faz 4.6d — librw stream doğrulaması: gerçek Streams/*.RWS asset'i
# RwStream shim üzerinden chunk chunk taranır (kapı 4 sinyali).
# add_sdk_test yerine elle: test argüman olarak asset yolunu alır.
add_executable(rwlibrw_test
    ${CMAKE_CURRENT_SOURCE_DIR}/rwlibrw_test.cpp
    ${CODE_DIR}/rw_librw.cpp
)
target_include_directories(rwlibrw_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(rwlibrw_test PRIVATE librw::librw)
add_test(NAME rwlibrw_test COMMAND rwlibrw_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS)

# Faz 4.6e — rwstudio konteyner parse: asset kaydı/manifest envanteri +
# gömülü txd payload doğrulaması (hexdump forensics ile sabit beklentiler).
add_executable(rwstudio_test
    ${CMAKE_CURRENT_SOURCE_DIR}/rwstudio_test.cpp
    ${CODE_DIR}/rw_librw.cpp
    ${CODE_DIR}/RwStudioStream.cpp
)
target_include_directories(rwstudio_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(rwstudio_test PRIVATE librw::librw)
add_test(NAME rwstudio_test COMMAND rwstudio_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/GLOBAL.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/VIKING01.RWS)

# Faz 4.6h — RenderBehaviourWorld property apply: 0x0704 id→offset
# haritası (FUN_004c3e40) gerçek MAINMENU obj#0 verisiyle doğrulanır.
add_executable(renderworld_test
    ${CMAKE_CURRENT_SOURCE_DIR}/renderworld_test.cpp
    ${CODE_DIR}/RenderBehaviourWorld.cpp
    ${CODE_DIR}/rw_librw.cpp
    ${CODE_DIR}/RwStudioStream.cpp
)
target_include_directories(renderworld_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(renderworld_test PRIVATE librw::librw)
add_test(NAME renderworld_test COMMAND renderworld_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS)

# Faz 4.6i — CWbCameraBehavior property apply: 51 case'lik switch
# (sub_4C3450) gerçek MAINMENU obj#1 verisiyle doğrulanır.
add_executable(camerastate_test
    ${CMAKE_CURRENT_SOURCE_DIR}/camerastate_test.cpp
    ${CODE_DIR}/CameraState.cpp
    ${CODE_DIR}/rw_librw.cpp
    ${CODE_DIR}/RwStudioStream.cpp
)
target_include_directories(camerastate_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(camerastate_test PRIVATE librw::librw)
add_test(NAME camerastate_test COMMAND camerastate_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS)

# Faz 4.6j — CAudioStreamEntity property apply: 6 case'lik switch
# (sub_458BD0) gerçek MAINMENU obj#2 verisiyle doğrulanır.
add_executable(audiostream_test
    ${CMAKE_CURRENT_SOURCE_DIR}/audiostream_test.cpp
    ${CODE_DIR}/AudioStreamEntity.cpp
    ${CODE_DIR}/rw_librw.cpp
    ${CODE_DIR}/RwStudioStream.cpp
)
target_include_directories(audiostream_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(audiostream_test PRIVATE librw::librw)
add_test(NAME audiostream_test COMMAND audiostream_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS)

# Faz 4.6k — BehaviourLoader create-by-name dispatch: sınıf adı →
# CClassFactory → create → apply virtual; render/camera/audio uçtan uca.
add_executable(behaviourloader_test
    ${CMAKE_CURRENT_SOURCE_DIR}/behaviourloader_test.cpp
    ${CODE_DIR}/BehaviourLoader.cpp
    ${CODE_DIR}/RenderBehaviourWorld.cpp
    ${CODE_DIR}/CameraState.cpp
    ${CODE_DIR}/AudioStreamEntity.cpp
    ${CODE_DIR}/rw_librw.cpp
    ${CODE_DIR}/RwStudioStream.cpp
    ${CODE_DIR}/framework/core/attributehandler/cclassfactory.cpp
)
target_include_directories(behaviourloader_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(behaviourloader_test PRIVATE librw::librw)
add_test(NAME behaviourloader_test COMMAND behaviourloader_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS)

# Faz 4.6l — Gömülü TXD (d3d8 IMAGE) decode: MAINMENU gömülü txdsi
# (16 texture) decode edilip name/format/piksel doğrulanır.
add_executable(txddecode_test
    ${CMAKE_CURRENT_SOURCE_DIR}/txddecode_test.cpp
    ${CODE_DIR}/TxdDecode.cpp
    ${CODE_DIR}/rw_librw.cpp
    ${CODE_DIR}/RwStudioStream.cpp
)
target_include_directories(txddecode_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(txddecode_test PRIVATE librw::librw)
add_test(NAME txddecode_test COMMAND txddecode_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS)

# Faz 4.6m — Statik dünya çarpışma (.col / STATICWORLDCOLLISION): FUN_0043bfa0
# düzeni decode + PhysicsShape_CreateTriMesh; üç gerçek dünya sayı/bbox/shape.
add_executable(coltrimesh_test
    ${CMAKE_CURRENT_SOURCE_DIR}/coltrimesh_test.cpp
    ${CODE_DIR}/StaticCollisionData.cpp
    ${CODE_DIR}/PhysicsBackend.cpp
    ${CODE_DIR}/rw_librw.cpp
    ${CODE_DIR}/RwStudioStream.cpp
)
target_include_directories(coltrimesh_test PRIVATE
    ${SDK_INCLUDE_DIRS}
    ${bullet3_SOURCE_DIR}/src
)
target_link_libraries(coltrimesh_test PRIVATE
    librw::librw
    BulletDynamics BulletCollision LinearMath
)
add_test(NAME coltrimesh_test COMMAND coltrimesh_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/VIKING01.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/IDUNA03.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/BEAVER01.RWS)

# Faz 4.6n — Render platformu smoke: librw sahne kurulumu + kare döngüsü
# (clear/BeginUpdate/Im2D test karesi/EndUpdate) + terminate/yeniden kurulum.
# NULL platform headless — device no-op; kare API sözleşmesi doğrulanır.
add_executable(renderplatform_test
    ${CMAKE_CURRENT_SOURCE_DIR}/renderplatform_test.cpp
    ${CODE_DIR}/RenderPlatform.cpp
    ${CODE_DIR}/RenderPipeline.cpp
    ${CODE_DIR}/TxdDecode.cpp
    ${CODE_DIR}/rw_librw.cpp
)
target_include_directories(renderplatform_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(renderplatform_test PRIVATE librw::librw)
add_test(NAME renderplatform_test COMMAND renderplatform_test)

# Faz 4.6o — WorldGeometry decode + clump instantiate: [len][libid] prefix
# + STRUCT/GEOMETRY/EXT + ATOMIC×N (gömülü geometry) — üç gerçek dünya
# envanter sayıları + VIKING01 clump (gömülü streamRead) + bozuk girdi.
add_executable(worldgeometry_test
    ${CMAKE_CURRENT_SOURCE_DIR}/worldgeometry_test.cpp
    ${CODE_DIR}/WorldGeometry.cpp
    ${CODE_DIR}/RwStudioStream.cpp
    ${CODE_DIR}/rw_librw.cpp
)
target_include_directories(worldgeometry_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(worldgeometry_test PRIVATE librw::librw)
add_test(NAME worldgeometry_test COMMAND worldgeometry_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/VIKING01.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/IDUNA03.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/BEAVER01.RWS)

# Faz 4.6p — dünya spawn zinciri: 0x0704 kayıtları → CClassFactory
# create + apply (bilinen sınıflar üretilir; bilinmeyenler sessiz atla).
# WHOLE_ARCHIVE: RWS_REGISTER_CLASS statik kayıtları archive'tan çekilsin.
add_executable(worldspawn_test
    ${CMAKE_CURRENT_SOURCE_DIR}/worldspawn_test.cpp
)
target_include_directories(worldspawn_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(worldspawn_test PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME worldspawn_test COMMAND worldspawn_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/VIKING01.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS)

# Faz 4.6q — ocean smoke: COcean ızgara kurulumu + tile dalgaları + Im3D
# render yolunun çökmeden çalışması (NULL device no-op).
add_executable(ocean_test
    ${CMAKE_CURRENT_SOURCE_DIR}/ocean_test.cpp
)
target_include_directories(ocean_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(ocean_test PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME ocean_test COMMAND ocean_test)

# Faz 4.6r-1 — Rt2dMaestro .anm parser: Menu.anm (32 düğüm) + CAG_Menu.anm
# (44 düğüm) fixture — header/node/sahne başlığı birebir.
add_executable(rt2danim_test
    ${CMAKE_CURRENT_SOURCE_DIR}/rt2danim_test.cpp
)
target_include_directories(rt2danim_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(rt2danim_test PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME rt2danim_test COMMAND rt2danim_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAPMENU.RWS)

# Faz 4.6r-4/altyapı — menü+harita durum makinesi (headless, enginesiz).
add_executable(menustate_test
    ${CMAKE_CURRENT_SOURCE_DIR}/menustate_test.cpp
)
target_include_directories(menustate_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(menustate_test PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME menustate_test COMMAND menustate_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAPMENU.RWS)
