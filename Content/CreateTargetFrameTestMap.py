import unreal


MAP_PATH = "/Game/Maps/TF_TargetFrameTestMap"
TEMPLATE_PATH = "/Engine/Maps/Templates/Template_Default"
LEVEL_EDITOR = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ASSET_SUBSYSTEM = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)


def main():
    unreal.log("TargetFrame map creation started.")

    if ASSET_SUBSYSTEM.does_asset_exist(MAP_PATH):
        if not ASSET_SUBSYSTEM.delete_asset(MAP_PATH):
            raise RuntimeError(f"Unable to delete existing map at {MAP_PATH}")

    if not LEVEL_EDITOR.new_level_from_template(MAP_PATH, TEMPLATE_PATH):
        raise RuntimeError(f"Unable to create map from template: {TEMPLATE_PATH}")

    if not LEVEL_EDITOR.load_level(MAP_PATH):
        raise RuntimeError(f"Unable to load map: {MAP_PATH}")

    if not LEVEL_EDITOR.save_current_level():
        raise RuntimeError(f"Unable to save level: {MAP_PATH}")

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log(f"TargetFrame map creation finished: {MAP_PATH}")


if __name__ == "__main__":
    main()
