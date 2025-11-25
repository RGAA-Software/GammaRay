import re
def extract_project_version(file_path):
    with open(file_path, 'r') as file:
        content = file.read()

    # 使用正则表达式匹配 #define PROJECT_VERSION "x.x.x"
    match = re.search(r'#define\s+PROJECT_VERSION\s+"([0-9.]+)"', content)
    if match:
        return match.group(1)
    return None

def gen_package_name() ->str:
    target_name = "GammaRay_" + extract_project_version("../version_config.h")
    return target_name

def gen_package_pdb_name() ->str:
    target_name = "GammaRay_pdb_" + extract_project_version("../version_config.h")
    return target_name