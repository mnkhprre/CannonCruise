# SDK-independent unit tests (Faz 6 / roadmap "test kapsamÄ±").
# Each test compiles only the framework sources under test â€” no RenderWare,
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

# Faz 4.6 â€” Bullet physics backend (Havok yerine); binary parametreleri
# gravity -9.8 / friction 0.6 / restitution 0.3 / adÄ±m 1/60 doÄŸrulanÄ±r.
add_sdk_test(physics_test
    ${CMAKE_CURRENT_SOURCE_DIR}/physics_test.cpp
    ${CODE_DIR}/PhysicsBackend.cpp
)
# Bullet include'larÄ± hedeflere yayÄ±lmaz â†’ elle
target_include_directories(physics_test PRIVATE ${bullet3_SOURCE_DIR}/src)
target_link_libraries(physics_test PRIVATE
    BulletDynamics BulletCollision LinearMath)

# Faz 4.6h â€” Havok 2.2.1 shim: binary adlandÄ±rmasÄ± (hkp Ã¶neki YOK) +
# Bullet motoru Ã¼zerine gerÃ§ek implementasyon (hkWorld/hkRigidBody/hkPhantom).
# gravity -9.8 / friction 0.6 / restitution 0.3 (sub_55E6D0) + stepDeltaTime.
add_executable(havokshim_test
    ${CMAKE_CURRENT_SOURCE_DIR}/havokshim_test.cpp
    ${CODE_DIR}/havok/havokimpl.cpp
    ${CODE_DIR}/PhysicsBackend.cpp
)
target_include_directories(havokshim_test PRIVATE
    ${SDK_INCLUDE_DIRS}
    ${CODE_DIR}/havok
    ${bullet3_SOURCE_DIR}/src
)
target_link_libraries(havokshim_test PRIVATE
    BulletDynamics BulletCollision LinearMath
)
add_test(NAME havokshim_test COMMAND havokshim_test)

# Faz 4.6 â€” entity/trigger akÄ±ÅŸÄ±: CPhysicsEntity eriÅŸimcileri + AddToWorld
# singleton kaydÄ±; CCollisionTrigger phantom + PollOverlaps enter/leave.
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

# Faz 4.6d â€” librw stream doÄŸrulamasÄ±: gerÃ§ek Streams/*.RWS asset'i
# RwStream shim Ã¼zerinden chunk chunk taranÄ±r (kapÄ± 4 sinyali).
# add_sdk_test yerine elle: test argÃ¼man olarak asset yolunu alÄ±r.
add_executable(rwlibrw_test
    ${CMAKE_CURRENT_SOURCE_DIR}/rwlibrw_test.cpp
    ${CODE_DIR}/rw_librw.cpp
)
target_include_directories(rwlibrw_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(rwlibrw_test PRIVATE librw::librw)
add_test(NAME rwlibrw_test COMMAND rwlibrw_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS)

# Faz 4.6e â€” rwstudio konteyner parse: asset kaydÄ±/manifest envanteri +
# gÃ¶mÃ¼lÃ¼ txd payload doÄŸrulamasÄ± (hexdump forensics ile sabit beklentiler).
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

# Faz 4.6h â€” RenderBehaviourWorld property apply: 0x0704 idâ†’offset
# haritasÄ± (FUN_004c3e40) gerÃ§ek MAINMENU obj#0 verisiyle doÄŸrulanÄ±r.
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

# Faz 4.6i â€” CWbCameraBehavior property apply: 51 case'lik switch
# (sub_4C3450) gerÃ§ek MAINMENU obj#1 verisiyle doÄŸrulanÄ±r.
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

# Faz 4.6j â€” CAudioStreamEntity property apply: 6 case'lik switch
# (sub_458BD0) gerÃ§ek MAINMENU obj#2 verisiyle doÄŸrulanÄ±r.
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

# Faz 4.6k â€” BehaviourLoader create-by-name dispatch: sÄ±nÄ±f adÄ± â†’
# CClassFactory â†’ create â†’ apply virtual; render/camera/audio uÃ§tan uca.
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

# Faz 4.6l â€” GÃ¶mÃ¼lÃ¼ TXD (d3d8 IMAGE) decode: MAINMENU gÃ¶mÃ¼lÃ¼ txdsi
# (16 texture) decode edilip name/format/piksel doÄŸrulanÄ±r.
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

# Faz 4.6m â€” Statik dÃ¼nya Ã§arpÄ±ÅŸma (.col / STATICWORLDCOLLISION): FUN_0043bfa0
# dÃ¼zeni decode + PhysicsShape_CreateTriMesh; Ã¼Ã§ gerÃ§ek dÃ¼nya sayÄ±/bbox/shape.
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

# Faz 4.6n â€” Render platformu smoke: librw sahne kurulumu + kare dÃ¶ngÃ¼sÃ¼
# (clear/BeginUpdate/Im2D test karesi/EndUpdate) + terminate/yeniden kurulum.
# NULL platform headless â€” device no-op; kare API sÃ¶zleÅŸmesi doÄŸrulanÄ±r.
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

# Faz 4.6o â€” WorldGeometry decode + clump instantiate: [len][libid] prefix
# + STRUCT/GEOMETRY/EXT + ATOMICÃ—N (gÃ¶mÃ¼lÃ¼ geometry) â€” Ã¼Ã§ gerÃ§ek dÃ¼nya
# envanter sayÄ±larÄ± + VIKING01 clump (gÃ¶mÃ¼lÃ¼ streamRead) + bozuk girdi.
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

# Faz 4.6s — WorldGeometry TRAILING chunk decode (atomikler sonrası 0x22/0x1E/
# 0x39; Faz 4.6o'da yalnızca tip okunuyordu). Header alanları + body çözümü.
add_executable(trailingchunk_test
    ${CMAKE_CURRENT_SOURCE_DIR}/trailingchunk_test.cpp
    ${CODE_DIR}/WorldGeometry.cpp
    ${CODE_DIR}/RwStudioStream.cpp
    ${CODE_DIR}/rw_librw.cpp
)
target_include_directories(trailingchunk_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(trailingchunk_test PRIVATE librw::librw)
add_test(NAME trailingchunk_test COMMAND trailingchunk_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/VIKING01.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/IDUNA03.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/BEAVER01.RWS)

# Faz 4.6p â€” dÃ¼nya spawn zinciri: 0x0704 kayÄ±tlarÄ± â†’ CClassFactory
# create + apply (bilinen sÄ±nÄ±flar Ã¼retilir; bilinmeyenler sessiz atla).
# WHOLE_ARCHIVE: RWS_REGISTER_CLASS statik kayÄ±tlarÄ± archive'tan Ã§ekilsin.
add_executable(worldspawn_test
    ${CMAKE_CURRENT_SOURCE_DIR}/worldspawn_test.cpp
)
target_include_directories(worldspawn_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(worldspawn_test PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME worldspawn_test COMMAND worldspawn_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/VIKING01.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS)

# Faz 4.6q â€” ocean smoke: COcean Ä±zgara kurulumu + tile dalgalarÄ± + Im3D
# render yolunun Ã§Ã¶kmeden Ã§alÄ±ÅŸmasÄ± (NULL device no-op).
add_executable(ocean_test
    ${CMAKE_CURRENT_SOURCE_DIR}/ocean_test.cpp
)
target_include_directories(ocean_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(ocean_test PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME ocean_test COMMAND ocean_test)

# Faz 4.6r-1 â€” Rt2dMaestro .anm parser: Menu.anm (32 dÃ¼ÄŸÃ¼m) + CAG_Menu.anm
# (44 dÃ¼ÄŸÃ¼m) fixture â€” header/node/sahne baÅŸlÄ±ÄŸÄ± birebir.
add_executable(rt2danim_test
    ${CMAKE_CURRENT_SOURCE_DIR}/rt2danim_test.cpp
)
target_include_directories(rt2danim_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(rt2danim_test PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME rt2danim_test COMMAND rt2danim_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAPMENU.RWS)

# Faz 4.6r-4/altyapÄ± â€” menÃ¼+harita durum makinesi (headless, enginesiz).
add_executable(menustate_test
    ${CMAKE_CURRENT_SOURCE_DIR}/menustate_test.cpp
)
target_include_directories(menustate_test PRIVATE ${SDK_INCLUDE_DIRS})
target_link_libraries(menustate_test PRIVATE
    "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME menustate_test COMMAND menustate_test
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAINMENU.RWS
    ${CMAKE_CURRENT_SOURCE_DIR}/../Streams/MAPMENU.RWS)

# .RWS asset yol Ã§Ã¶zÃ¼cÃ¼ â€” CANNONCRUISE_STREAMS env ezmesi + sentinel Ã¶nek
# taramasÄ± (derin build yerleÅŸimleri). Merkezi Ã§Ã¶zÃ¼cÃ¼ 5 elle-yazÄ±lÄ± aday
# listesinin yerini alir (main.cpp / SceneManager.cpp).
add_executable(rwsassets_test
    ${CMAKE_CURRENT_SOURCE_DIR}/rwsassets_test.cpp
    ${CODE_DIR}/RwsAssets.cpp
)
target_include_directories(rwsassets_test PRIVATE ${SDK_INCLUDE_DIRS} ${CODE_DIR})
add_test(NAME rwsassets_test COMMAND rwsassets_test)

# FMV sahne tanÄ±mlayicisi (CFMVBehavior) â€” 8 dosyalÄ±k geÃ§is grafigi (JINGLE/
# INTRO1-3/CREDITS/OUTRO1-3), video yolu ".\Video\Win32\<ad>.bik" ve HEDEF
# sahne baglantisi (INTRO1 -> NORDIC01; uppercase denemesi sart).
add_executable(fmvscene_test
    ${CMAKE_CURRENT_SOURCE_DIR}/fmvscene_test.cpp
    ${CODE_DIR}/FmvScene.cpp
    ${CODE_DIR}/RwsAssets.cpp
)
target_include_directories(fmvscene_test PRIVATE ${SDK_INCLUDE_DIRS} ${CODE_DIR})
add_test(NAME fmvscene_test COMMAND fmvscene_test)

# LocalizationManager test - Localiza/lang.def, langmap.def ve lang_<dil>.txt
# parsing, BOM dogrulama, string lookup ve MENUTRIGGER event dispatch.
add_executable(localization_test
    ${CMAKE_CURRENT_SOURCE_DIR}/localization_test.cpp
)
target_include_directories(localization_test PRIVATE ${SDK_INCLUDE_DIRS} ${CODE_DIR})
target_link_libraries(localization_test PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME localization_test COMMAND localization_test)

# CShipPlayer test - reflection class factory kaydi, skor, firtina ve respawn
add_executable(shipplayer_test
    ${CMAKE_CURRENT_SOURCE_DIR}/shipplayer_test.cpp
)
target_include_directories(shipplayer_test PRIVATE ${SDK_INCLUDE_DIRS} ${CODE_DIR})
target_link_libraries(shipplayer_test PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME shipplayer_test COMMAND shipplayer_test)

# InputManager test - Focus stack, button edge trigger detection ve ShipPlayerInputProxy
add_executable(inputmanager_test
    ${CMAKE_CURRENT_SOURCE_DIR}/inputmanager_test.cpp
)
target_include_directories(inputmanager_test PRIVATE ${SDK_INCLUDE_DIRS} ${CODE_DIR})
target_link_libraries(inputmanager_test PRIVATE "$<LINK_LIBRARY:WHOLE_ARCHIVE,CannonCruiseLib>")
add_test(NAME inputmanager_test COMMAND inputmanager_test)
