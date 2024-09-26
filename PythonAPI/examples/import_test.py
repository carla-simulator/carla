import unreal
import xml.etree.ElementTree as etree
import random

#obj = unreal.find_asset(None, "SM_DodgeCharger2024Parked2")

#mov_scene = unreal.MovieSceneSequence("ImportTestScene")
#mov_scene.add_spawnable_from_instance(obj)
#unreal.MovieSceneParameterSection

TagToObjectDictionary = {}
IDToInstanceDictionary = {}
IDToTransformSectionDict = {}
IDToEventTrackSectionDict = {}
IDToAnimTrackIdleSectionDict = {}
IDToAnimTrackWalkingSectionDict = {}

TagToObjectDictionary["lincoln"]    = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/LincolnMkz2024/BP_Lincoln2024.BP_Lincoln2024'")
TagToObjectDictionary["mini"]    = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/Mini2024/BP_Mini2024.BP_Mini2024'")
TagToObjectDictionary["dodge"]      = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/DodgeCharger2024/BP_Charger2024.BP_Charger2024'")
TagToObjectDictionary["nissan"]     = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/NissanPatrol2024/BP_NissanPatrol2024.BP_NissanPatrol2024'")
TagToObjectDictionary["ambulance"]  = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/Ambulance2024/BP_Ambulance2024.BP_Ambulance2024'")
TagToObjectDictionary["sprinter"]   = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/Sprinter2024/BP_Sprinter2024.BP_Sprinter2024'")
TagToObjectDictionary["fuso"]       = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/MitsubishiFusoRosa2024/BP_FusoRosa2024.BP_FusoRosa2024'")
TagToObjectDictionary["firetruck"]  = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/Firetruck2024/BP_FireTruck2024.BP_FireTruck2024'")
TagToObjectDictionary["dodgecop"]   = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/DodgeChargerCop2024/BP_ChargerCop2024.BP_ChargerCop2024'")
TagToObjectDictionary["carlacola"]  = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/CarlaCola2024/BP_CarlaCola2024.BP_CarlaCola2024'")
TagToObjectDictionary["cooper"]     = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/Mini2024/BP_Mini2024.BP_Mini2024'")
TagToObjectDictionary["taxi"]   = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/FordCrown2024/BP_Ford_Crown2024.BP_Ford_Crown2024'")
TagToObjectDictionary["UE4.ford"] = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/Mustang/BP_Mustang66.BP_Mustang66'")
TagToObjectDictionary["UE4.bmw"] = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/BmwGrandTourer/BP_BmwGranTourer.BP_BmwGranTourer'")
TagToObjectDictionary["UE4.audi"] = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/AudiTT/BP_AudiTT.BP_AudiTT'")
TagToObjectDictionary["UE4.mercedes"] = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/MercedesCCC/BP_MercedesCCC.BP_MercedesCCC'")
TagToObjectDictionary["UE4.chevrolet"] = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/ChevroletImpala/BP_ChevroletImpala.BP_ChevroletImpala'")
# TagToObjectDictionary["pedestrian"] = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker.BP_Walker'")

# vehicle_parent = unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Vehicles/BaseVehiclePawn.BaseVehiclePawn'")

walkers = [
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_AB001_G3.BP_Walker_AB001_G3'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_AG001_G3.BP_Walker_AG001_G3'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_WalkerEuBoy02_v1.BP_WalkerEuBoy02_v1'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_WalkerEuGirl02_v1.BP_WalkerEuGirl02_v1'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_Male_Asia02_v3.BP_Walker_Male_Asia02_v3'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_FemaleAfro03_v3.BP_Walker_FemaleAfro03_v3'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_FemaleAfro02D.BP_Walker_FemaleAfro02D'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_FemaleAfro02B.BP_Walker_FemaleAfro02B'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_MaleAmer_v3.BP_Walker_MaleAmer_v3'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_MaleAmer_v1.BP_Walker_MaleAmer_v1'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_FemaleAsia_v2.BP_Walker_FemaleAsia_v2'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_FemaleEuro_v2.BP_Walker_FemaleEuro_v2'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_MaleAfro_v1.BP_Walker_MaleAfro_v1'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_FemaleAfro_v2.BP_Walker_FemaleAfro_v2'"),
     unreal.load_asset("/Script/Engine.Blueprint'/Game/Carla/Blueprints/Walkers/BP_Walker_Female_Euro_Owv_v3.BP_Walker_Female_Euro_Owv_v3'"),
]

walk_anim = unreal.load_asset("/Script/Engine.AnimSequence'/Game/Carla/Animations/GEN2/AS_male2_WalkCicle0E.AS_male2_WalkCicle0E'")
idle_anim = unreal.load_asset("/Script/Engine.AnimSequence'/Game/Carla/Animations/Depot/Carla_1st/AS_Carla_Idle0B.AS_Carla_Idle0B'")

def populate_track(sequence, track, num_sections = 1, section_length_seconds = 1):

	for i in range(num_sections):
		section = track.add_section()
		section.set_start_frame_seconds(i*section_length_seconds)
		section.set_end_frame_seconds(section_length_seconds)

def populate_binding(sequence, binding, num_sections = 1, section_length_seconds = 1):

	for track in binding.get_tracks():
		populate_track(sequence, track, num_sections, section_length_seconds)
		



def create_level_sequence():

    #unreal.EditorAssetLibrary().delete_asset("/Game/TestImportSequence")
    sequence = unreal.AssetToolsHelpers.get_asset_tools().create_asset("TestImportSequence", "/Game/", unreal.LevelSequence, unreal.LevelSequenceFactoryNew())

    tree = etree.parse("C:/CarlaUE5/PythonAPI/examples/test.xml")
    root = tree.getroot()
    
    #sequence.add_possessable(unreal.load_object(None, "BP_TrafficLightNew_T10_master_largeBIG_rsc9"))

    for object in root[0]:
        #try:
        if not object[0].text in TagToObjectDictionary or object[0].text == "pedestrian":
            # IDToInstanceDictionary[object.text] = sequence.add_spawnable_from_instance(TagToObjectDictionary["pedestrian"])
            IDToInstanceDictionary[object.text] = sequence.add_spawnable_from_instance(walkers[random.randint(0,len(walkers)-1)])
        else:
            IDToInstanceDictionary[object.text] = sequence.add_spawnable_from_instance(TagToObjectDictionary[object[0].text])
            
        
        transform_track = IDToInstanceDictionary[object.text].add_track(unreal.MovieScene3DTransformTrack)
        transform_section = transform_track.add_section()
        transform_section.set_start_frame_bounded(0)
        transform_section.set_end_frame_bounded(0)
        transform_section.set_editor_property('use_quaternion_interpolation', True)
        IDToTransformSectionDict[object.text] = transform_section

        if object[0].text != "pedestrian" and object[0].text in TagToObjectDictionary:
            event_track = IDToInstanceDictionary[object.text].add_track(unreal.MovieSceneEventTrack)
            event_section = event_track.add_event_trigger_section()
            event_section.set_start_frame_bounded(0)
            event_section.set_end_frame_bounded(0)
            IDToEventTrackSectionDict[object.text] = event_section

        else:
            anim_track = IDToInstanceDictionary[object.text].add_track(unreal.MovieSceneSkeletalAnimationTrack)
            anim_track.set_display_name("Walking Animation")

            anim_section_walking = anim_track.add_section()
            anim_section_walking.set_start_frame_bounded(True)
            anim_section_walking.set_end_frame_bounded(True)
            anim_section_walking.set_start_frame(0)
            anim_section_walking.set_end_frame(len(root))

            anim_params = anim_section_walking.get_editor_property('Params')
            anim_params.set_editor_property('force_custom_mode', True)
            anim_params.set_editor_property('animation', walk_anim)  
            anim_section_walking.set_editor_property('Params', anim_params)

            anim_track = IDToInstanceDictionary[object.text].add_track(unreal.MovieSceneSkeletalAnimationTrack)
            anim_track.set_display_name("Idle Animation")

            anim_section_idle = anim_track.add_section()
            anim_section_idle.set_start_frame_bounded(True)
            anim_section_idle.set_end_frame_bounded(True)
            anim_section_idle.set_start_frame(0)
            anim_section_idle.set_end_frame(len(root))

            anim_params = anim_section_idle.get_editor_property('Params')
            anim_params.set_editor_property('force_custom_mode', True)
            anim_params.set_editor_property('animation', idle_anim)  
            anim_section_idle.set_editor_property('Params', anim_params)

            IDToAnimTrackWalkingSectionDict[object.text] = anim_section_walking
            IDToAnimTrackIdleSectionDict[object.text] = anim_section_idle
        
        #except:
        #    print("Couldn't find key", object.text)
	
    
    for j in range(0, len(root), 1):

        tmp = root[j]

        for i in range(0, len(tmp), 1):
            
            position = [float(tmp[i][1][0].text), float(tmp[i][1][1].text), float(tmp[i][1][2].text)]
            rotation = [float(tmp[i][2][0].text), float(tmp[i][2][1].text), float(tmp[i][2][2].text)]

            #try:
            channels = IDToTransformSectionDict[tmp[i].text].get_all_channels()
            
            channels[0].add_key(unreal.FrameNumber(j), position[0]*100.0)
            channels[1].add_key(unreal.FrameNumber(j), position[1]*100.0)
            channels[2].add_key(unreal.FrameNumber(j), position[2]*100.0)

            channels[3].add_key(unreal.FrameNumber(j), rotation[0])
            channels[4].add_key(unreal.FrameNumber(j), rotation[2])
            channels[5].add_key(unreal.FrameNumber(j), rotation[1])

            if tmp[i][0].text != "pedestrian" and tmp[i][0].text in TagToObjectDictionary:
                if j % 10 == 0: #debug to iterate quicker
                    quick_binding = unreal.SequencerTools.create_quick_binding(sequence,  IDToInstanceDictionary[tmp[i].text].get_object_template(), "SequencerSetLightState", True)
                    payload = [str(tmp[i][3].text)]
                    event = unreal.SequencerTools.create_event(sequence, IDToEventTrackSectionDict[tmp[i].text], quick_binding, payload)
                    channels = IDToEventTrackSectionDict[tmp[i].text].get_all_channels()
                    channels[0].add_key(unreal.FrameNumber(j), event)
            
            else:
                walk_channel = IDToAnimTrackWalkingSectionDict[tmp[i].text].get_all_channels()
                idle_channel = IDToAnimTrackIdleSectionDict[tmp[i].text].get_all_channels()

                if tmp[i][4].text == "1":
                    walk_channel[0].add_key(unreal.FrameNumber(j), 1.0)
                    idle_channel[0].add_key(unreal.FrameNumber(j), 0.0)
                else:
                    walk_channel[0].add_key(unreal.FrameNumber(j), 0.0)
                    idle_channel[0].add_key(unreal.FrameNumber(j), 1.0)
                

        #except:
        #    print("The privided ID has no instance", tmp[i].text)

    #obj = unreal.find_object(None, "SM_DodgeCharger2024Parked2")
    #obj = unreal.load_asset('/Engine/Content/Carla/Static/Car/4Wheeled/DodgeCharger2024/SM_DodgeCharger2024Parked')
    #obj = unreal.load_asset("/Script/Engine.StaticMesh'/Game/Carla/Static/Car/4Wheeled/DodgeCharger2024/SM_DodgeCharger2024Parked.SM_DodgeCharger2024Parked'")
    #obj = unreal.load_asset("StaticMesh'/Engine/BasicShapes/Cube.Cube'")
    #obj_binding = sequence.add_possessable(obj)
    #obj_binding.add_track(unreal.MovieScene3DTransformTrack)
    #populate_binding(sequence, obj_binding, 1, 5)

    #spawnable_car_binding = sequence.add_spawnable_from_instance(TagToObjectDictionary["lincoln"])
    #transform_track = spawnable_car_binding.add_track(unreal.MovieScene3DTransformTrack)

    spawnable_camera_binding = sequence.add_spawnable_from_class(unreal.CineCameraActor)
    transform_section = spawnable_camera_binding.add_track(unreal.MovieScene3DTransformTrack).add_section()
    transform_section.set_start_frame_bounded(0)
    transform_section.set_end_frame_bounded(0)
	
create_level_sequence()