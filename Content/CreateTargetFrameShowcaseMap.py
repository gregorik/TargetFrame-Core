import unreal


MAP_PATH = "/Game/Maps/TF_TargetFrameShowcase"
TEMPLATE_PATH = "/Engine/Maps/Templates/Template_Default"
SHOWCASE_CLASS_PATH = "/Script/Scalability.TargetFrameShowcaseActor"

LEVEL_EDITOR = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ASSET_SUBSYSTEM = unreal.get_editor_subsystem(unreal.EditorAssetSubsystem)
ACTOR_SUBSYSTEM = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def clear_level():
    keep_classes = {"WorldSettings", "LevelScriptActor", "Brush"}
    for actor in ACTOR_SUBSYSTEM.get_all_level_actors():
        class_name = actor.get_class().get_name()
        if class_name in keep_classes:
            continue
        ACTOR_SUBSYSTEM.destroy_actor(actor)


def main():
    unreal.log("TargetFrame showcase map creation started.")

    if ASSET_SUBSYSTEM.does_asset_exist(MAP_PATH):
        if not LEVEL_EDITOR.load_level(MAP_PATH):
            raise RuntimeError(f"Unable to load existing map: {MAP_PATH}")
    else:
        if not LEVEL_EDITOR.new_level_from_template(MAP_PATH, TEMPLATE_PATH):
            raise RuntimeError(f"Unable to create map from template: {TEMPLATE_PATH}")

        if not LEVEL_EDITOR.load_level(MAP_PATH):
            raise RuntimeError(f"Unable to load new map: {MAP_PATH}")

    clear_level()

    showcase_class = unreal.load_class(None, SHOWCASE_CLASS_PATH)
    if showcase_class is None:
        raise RuntimeError(f"Unable to load showcase class: {SHOWCASE_CLASS_PATH}")

    showcase_actor = ACTOR_SUBSYSTEM.spawn_actor_from_class(showcase_class, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
    if showcase_actor is None:
        raise RuntimeError("Unable to spawn TargetFrame showcase actor.")
    showcase_actor.set_actor_label("TargetFrameShowcase")

    player_start = ACTOR_SUBSYSTEM.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-5200.0, 0.0, 140.0), unreal.Rotator(0.0, 0.0, 0.0))
    if player_start is not None:
        player_start.set_actor_label("ShowcasePlayerStart")

    if not LEVEL_EDITOR.save_current_level():
        raise RuntimeError(f"Unable to save level: {MAP_PATH}")

    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log(f"TargetFrame showcase map creation finished: {MAP_PATH}")


if __name__ == "__main__":
    main()
