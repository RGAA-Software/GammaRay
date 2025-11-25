import os
import sys
import shutil

from shutil import copy2
from shutil import copyfile
from shutil import copytree

from gen_pack_name import gen_package_name

def collceion_program_files(force_update, in_target_path):

    base_path = "./../"

    print("the path is : {}".format(base_path))

    ignore_files = [
        "plugin_amf_encoder.dll",
        "plugin_clipboard.dll",
        "plugin_dda_capture.dll",
        "plugin_event_replayer.dll",
        "plugin_ffmpeg_encoder.dll",
        "plugin_file_transfer.dll",
        "plugin_frame_debugger.dll",
        "plugin_frame_resizer.dll",
        "plugin_media_recorder.dll",
        "plugin_mock_video_stream.dll",
        "plugin_multi_screens.dll",
        "plugin_net_relay.dll",
        "plugin_net_rtc.dll",
        "plugin_net_udp.dll",
        "plugin_net_ws.dll",
        "plugin_nvenc_encoder.dll",
        "plugin_obj_detector.dll",
        "plugin_opus_encoder.dll",
        "plugin_was_audio_capture.dll",
        "plugin_vr_manager.dll",
        "tc_rtc_client.dll",
        "protoc.exe",
        "test_audio_capture.exe",
        "test_common.exe",
        "test_http_client.exe",
        "test_opus_api.exe",
        "test_opus_decode.exe",
        "test_opus_encode.exe",
        "test_opus_extensions.exe",
        "test_opus_padding.exe",
        "test_resolutions.exe",
        "yuvconstants.exe",
        "yuvconvert.exe",
        "protoc-gen-upbdefs.exe",
        "protoc-gen-upb_minitable.exe",
        "protoc-gen-upb.exe",
        "cpuid.exe",
        "skin_official.dll",
        "skin_opensource.dll",
        "test_http.exe",
        "vc_redist.x64_cpy.exe",
        "uninstall.exe"
    ]

    files_with_ref_path = []
    files = os.listdir(base_path)
    for file in files:
        found = False
        for ignore_file in ignore_files:
            if file == ignore_file:
                found = True
                break
        if found:
            continue

        file_path = base_path + "/" + file
        if ".dll" in file:
            files_with_ref_path.append(file_path)
        if ".DLL" in file:
            files_with_ref_path.append(file_path)
        if ".exe" in file:
            files_with_ref_path.append(file_path)
        if ".key" in file:
            files_with_ref_path.append(file_path)
        if ".toml" in file:
            files_with_ref_path.append(file_path)
        if ".ico" in file:
            files_with_ref_path.append(file_path)

    resources_file_path = []
    #resources_file_path.append("resources/MicrosoftYaqiHei-2.ttf")

    folders_path = []
    folders_path.append(base_path + "iconengines")
    folders_path.append(base_path + "imageformats")
    folders_path.append(base_path + "bearer")
    folders_path.append(base_path + "audio")
    folders_path.append(base_path + "mediaservice")
    folders_path.append(base_path + "platforms")
    folders_path.append(base_path + "playlistformats")
    folders_path.append(base_path + "plugins")
    folders_path.append(base_path + "sdw_plugins")
    folders_path.append(base_path + "styles")
    folders_path.append(base_path + "resources")
    folders_path.append(base_path + "generic")
    folders_path.append(base_path + "tls")
    folders_path.append(base_path + "networkinformation")
    folders_path.append(base_path + "tc_app")
    folders_path.append(base_path + "platforminputcontexts")
    folders_path.append(base_path + "qml")
    folders_path.append(base_path + "qmltooling")
    folders_path.append(base_path + "gr_plugins")
    folders_path.append(base_path + "gr_plugins_client")
    folders_path.append(base_path + "gr_client")
    folders_path.append(base_path + "certs")
    folders_path.append(base_path + "web")
    folders_path.append(base_path + "gr_skins")

    target_path = base_path + "package/packages/com.rgaa.gammaray/data"#+ "gammaray" + target_folder_suffix
    if len(in_target_path) > 0:
        target_path = in_target_path

    if force_update and os.path.exists(target_path):
        shutil.rmtree(target_path)

    if not os.path.exists(target_path):
        os.makedirs(target_path)

    for file in files_with_ref_path:
        file_name = file.split("/")[-1]
        print("copy file {} to {}".format(file_name, target_path + "/" + file_name))
        copyfile(file, target_path + "/" + file_name)

    for file in resources_file_path:
        file_name = file.split("/")[-1]
        print("copy file {} to {}".format(file_name, resources_path + "/" + file_name))
        copy2(file, resources_path + "/" + file_name)

    for folder in folders_path:
        file_name = folder.split("/")[-1]
        print("copy folder {} to {}".format(file_name, folder))
        try:
            copytree(folder, target_path + "/" + file_name)
        except:
            print("3rd libs folder already exists, use : force-update if you want to update them.")

# python install.py target_path

if __name__ == "__main__":
    print("arg : {}".format(sys.argv))
    force_update = True

    target_path = ""
    if len(sys.argv) >= 2:
        target_path = sys.argv[1]
    else:
        target_path = gen_package_name()

    # delete it
    try:
        print("will delete folder: {}".format(target_path))
        shutil.rmtree(target_path)
    except:
        print("")

    collceion_program_files(force_update, target_path)