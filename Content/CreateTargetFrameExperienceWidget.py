import unreal

ASSET_NAME = "WBP_TargetFrameExperience"
PACKAGE_PATH = "/Game/UI"
ASSET_PATH = f"{PACKAGE_PATH}/{ASSET_NAME}"
PARENT_CLASS_PATH = "/Script/TargetFrame.TargetFrameUserExperienceWidget"

TEXT = unreal.Text

PANEL_COLOR = unreal.LinearColor(0.05, 0.07, 0.10, 1.0)
TEXT_COLOR = unreal.LinearColor(0.95, 0.97, 1.0, 1.0)
MUTED_TEXT_COLOR = unreal.LinearColor(0.66, 0.72, 0.80, 1.0)
ACCENT_COLOR = unreal.LinearColor(0.95, 0.56, 0.19, 1.0)
BRAND_COLOR = unreal.LinearColor(0.18, 0.82, 0.76, 1.0)
WARNING_COLOR = unreal.LinearColor(0.98, 0.77, 0.32, 1.0)
ROBOTO_FONT = unreal.load_object(None, "/Engine/EngineFonts/Roboto.Roboto")


def make_margin(left, top=None, right=None, bottom=None):
    if top is None:
        top = left
    if right is None:
        right = left
    if bottom is None:
        bottom = top
    return unreal.Margin(left, top, right, bottom)


def make_slate_color(color):
    slate_color = unreal.SlateColor()
    slate_color.specified_color = color
    return slate_color


def make_font(size, bold=False):
    font = unreal.SlateFontInfo()
    font.font_object = ROBOTO_FONT
    font.size = size
    font.typeface_font_name = "Bold" if bold else "Regular"
    return font


def compile_and_save(widget_blueprint):
    if hasattr(unreal, "BlueprintEditorLibrary"):
        unreal.BlueprintEditorLibrary.compile_blueprint(widget_blueprint)
    elif hasattr(unreal, "KismetEditorUtilities"):
        unreal.KismetEditorUtilities.compile_blueprint(widget_blueprint)

    unreal.EditorAssetLibrary.save_loaded_asset(widget_blueprint)


def add_widget(widget_blueprint, widget_class, name, parent_name=""):
    return unreal.EditorUtilityLibrary.add_source_widget(widget_blueprint, widget_class, name, parent_name)


def set_canvas_layout(widget, minimum, maximum=None, alignment=(0.0, 0.0), position=(0.0, 0.0), size=(100.0, 100.0), z_order=None, fill=False):
    slot = unreal.CanvasPanelSlot.cast(widget.get_editor_property("slot"))
    anchors = unreal.Anchors()
    anchors.minimum = unreal.Vector2D(minimum[0], minimum[1])
    if maximum is None:
        anchors.maximum = unreal.Vector2D(minimum[0], minimum[1])
    else:
        anchors.maximum = unreal.Vector2D(maximum[0], maximum[1])
    slot.set_anchors(anchors)
    slot.set_alignment(unreal.Vector2D(alignment[0], alignment[1]))
    if fill:
        slot.set_offsets(unreal.Margin(0.0, 0.0, 0.0, 0.0))
    else:
        slot.set_position(unreal.Vector2D(position[0], position[1]))
        slot.set_size(unreal.Vector2D(size[0], size[1]))
    if z_order is not None:
        slot.set_z_order(z_order)


def set_box_slot(widget, padding=None, fill=False, horizontal_alignment=None, vertical_alignment=None):
    slot = widget.get_editor_property("slot")
    if padding is not None and hasattr(slot, "set_padding"):
        slot.set_padding(padding)
    if fill and hasattr(slot, "set_size"):
        child_size = slot.get_editor_property("size")
        child_size.size_rule = unreal.SlateSizeRule.FILL
        child_size.value = 1.0
        slot.set_size(child_size)
    if horizontal_alignment is not None and hasattr(slot, "set_horizontal_alignment"):
        slot.set_horizontal_alignment(horizontal_alignment)
    if vertical_alignment is not None and hasattr(slot, "set_vertical_alignment"):
        slot.set_vertical_alignment(vertical_alignment)


def configure_text(widget, text, size, color, bold=False, wrap=0.0, justification=unreal.TextJustify.LEFT, visibility=None):
    widget.set_text(TEXT(text))
    widget.set_editor_property("font", make_font(size, bold))
    widget.set_editor_property("color_and_opacity", make_slate_color(color))
    widget.set_editor_property("justification", justification)
    widget.set_editor_property("auto_wrap_text", wrap > 0.0)
    widget.set_editor_property("wrap_text_at", wrap)
    if visibility is not None:
        widget.set_visibility(visibility)


def create_button(widget_blueprint, parent_name, button_name, label_name, label_text, label_color, fill=True, padding=None, font_size=12, wrap=0.0):
    button = add_widget(widget_blueprint, unreal.Button, button_name, parent_name)
    set_box_slot(
        button,
        padding=padding if padding is not None else make_margin(0.0, 0.0, 10.0, 0.0),
        fill=fill,
        horizontal_alignment=unreal.HorizontalAlignment.H_ALIGN_FILL,
        vertical_alignment=unreal.VerticalAlignment.V_ALIGN_FILL,
    )

    label = add_widget(widget_blueprint, unreal.TextBlock, label_name, button_name)
    configure_text(
        label,
        label_text,
        font_size,
        label_color,
        bold=True,
        wrap=wrap,
        justification=unreal.TextJustify.CENTER,
        visibility=unreal.SlateVisibility.HIT_TEST_INVISIBLE,
    )
    return button, label


def create_section_card(widget_blueprint, parent_name, border_name, heading_name, heading_text, heading_color, value_name, value_placeholder, value_wrap=560.0, bottom_padding=12.0):
    border = add_widget(widget_blueprint, unreal.Border, border_name, parent_name)
    set_box_slot(border, padding=make_margin(0.0, 0.0, 0.0, bottom_padding))
    content = add_widget(widget_blueprint, unreal.VerticalBox, f"{border_name}Content", border_name)

    heading = add_widget(widget_blueprint, unreal.TextBlock, heading_name, f"{border_name}Content")
    configure_text(heading, heading_text, 12, heading_color, bold=True)

    value = add_widget(widget_blueprint, unreal.TextBlock, value_name, f"{border_name}Content")
    set_box_slot(value, padding=make_margin(0.0, 10.0, 0.0, 0.0))
    configure_text(value, value_placeholder, 12, TEXT_COLOR, wrap=value_wrap)
    return border, value


def create_scroll_list_card(widget_blueprint, parent_name, border_name, heading_name, heading_text, heading_color, scroll_name, list_name, bottom_padding=12.0):
    border = add_widget(widget_blueprint, unreal.Border, border_name, parent_name)
    set_box_slot(border, padding=make_margin(0.0, 0.0, 0.0, bottom_padding))
    content = add_widget(widget_blueprint, unreal.VerticalBox, f"{border_name}Content", border_name)

    heading = add_widget(widget_blueprint, unreal.TextBlock, heading_name, f"{border_name}Content")
    configure_text(heading, heading_text, 12, heading_color, bold=True)

    scroll = add_widget(widget_blueprint, unreal.ScrollBox, scroll_name, f"{border_name}Content")
    set_box_slot(scroll, padding=make_margin(0.0, 10.0, 0.0, 0.0))

    list_box = add_widget(widget_blueprint, unreal.VerticalBox, list_name, scroll_name)
    return border, list_box


def cleanup_probe_assets():
    for probe_asset in (
        "/Game/UI/WBP_PythonProbe",
        "/Game/UI/WBP_IntrospectionProbe",
        "/Game/UI/WBP_ContentParentProbe",
    ):
        if unreal.EditorAssetLibrary.does_asset_exist(probe_asset):
            unreal.EditorAssetLibrary.delete_asset(probe_asset)


def main():
    unreal.EditorAssetLibrary.make_directory(PACKAGE_PATH)
    cleanup_probe_assets()

    if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
        unreal.EditorAssetLibrary.delete_asset(ASSET_PATH)

    parent_class = unreal.load_class(None, PARENT_CLASS_PATH)
    if parent_class is None:
        raise RuntimeError(f"Unable to load widget parent class at {PARENT_CLASS_PATH}")

    factory = unreal.WidgetBlueprintFactory()
    factory.set_editor_property("parent_class", parent_class)

    widget_blueprint = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name=ASSET_NAME,
        package_path=PACKAGE_PATH,
        asset_class=unreal.WidgetBlueprint,
        factory=factory,
    )

    root = add_widget(widget_blueprint, unreal.CanvasPanel, "RootCanvas", "")

    launcher_button = add_widget(widget_blueprint, unreal.Button, "LauncherButton", "RootCanvas")
    set_canvas_layout(launcher_button, minimum=(1.0, 0.0), alignment=(1.0, 0.0), position=(-28.0, 24.0), size=(260.0, 46.0), z_order=20)
    launcher_label = add_widget(widget_blueprint, unreal.TextBlock, "LauncherLabelText", "LauncherButton")
    configure_text(launcher_label, "Get Recommended Setup", 12, PANEL_COLOR, bold=True, justification=unreal.TextJustify.CENTER, visibility=unreal.SlateVisibility.HIT_TEST_INVISIBLE)

    scrim_border = add_widget(widget_blueprint, unreal.Border, "ScrimBorder", "RootCanvas")
    set_canvas_layout(scrim_border, minimum=(0.0, 0.0), maximum=(1.0, 1.0), fill=True, z_order=5)
    scrim_border.set_visibility(unreal.SlateVisibility.COLLAPSED)

    panel_border = add_widget(widget_blueprint, unreal.Border, "PanelBorder", "RootCanvas")
    set_canvas_layout(panel_border, minimum=(1.0, 0.5), alignment=(1.0, 0.5), position=(-28.0, 0.0), size=(660.0, 680.0), z_order=10)
    panel_border.set_visibility(unreal.SlateVisibility.COLLAPSED)

    panel_scroll = add_widget(widget_blueprint, unreal.ScrollBox, "PanelScroll", "PanelBorder")
    panel_content = add_widget(widget_blueprint, unreal.VerticalBox, "PanelContent", "PanelScroll")

    header_card = add_widget(widget_blueprint, unreal.Border, "HeaderCard", "PanelContent")
    set_box_slot(header_card, padding=make_margin(0.0, 0.0, 0.0, 12.0))
    header_content = add_widget(widget_blueprint, unreal.VerticalBox, "HeaderContent", "HeaderCard")

    header_title = add_widget(widget_blueprint, unreal.TextBlock, "HeaderTitleText", "HeaderContent")
    set_box_slot(header_title, padding=make_margin(0.0, 0.0, 0.0, 6.0))
    configure_text(header_title, "Getting your setup ready", 19, TEXT_COLOR, bold=True, wrap=620.0)

    header_summary = add_widget(widget_blueprint, unreal.TextBlock, "HeaderSummaryText", "HeaderContent")
    set_box_slot(header_summary, padding=make_margin(0.0, 0.0, 0.0, 10.0))
    configure_text(header_summary, "TargetFrame is taking a quick look at this PC and getting a smooth starting setup ready.", 12, MUTED_TEXT_COLOR, wrap=620.0)

    header_meta_row = add_widget(widget_blueprint, unreal.HorizontalBox, "HeaderMetaRow", "HeaderContent")
    set_box_slot(header_meta_row)

    header_meta_spacer = add_widget(widget_blueprint, unreal.SizeBox, "HeaderMetaSpacer", "HeaderMetaRow")
    set_box_slot(
        header_meta_spacer,
        fill=True,
        horizontal_alignment=unreal.HorizontalAlignment.H_ALIGN_FILL,
        vertical_alignment=unreal.VerticalAlignment.V_ALIGN_CENTER,
    )

    hardware_pill = add_widget(widget_blueprint, unreal.Border, "HardwarePillBorder", "HeaderMetaRow")
    set_box_slot(
        hardware_pill,
        horizontal_alignment=unreal.HorizontalAlignment.H_ALIGN_RIGHT,
        vertical_alignment=unreal.VerticalAlignment.V_ALIGN_CENTER,
    )
    header_pill_text = add_widget(widget_blueprint, unreal.TextBlock, "HeaderHardwarePillText", "HardwarePillBorder")
    configure_text(header_pill_text, "PC check in progress", 11, ACCENT_COLOR, bold=True, wrap=180.0, justification=unreal.TextJustify.CENTER)

    create_section_card(widget_blueprint, "PanelContent", "OverviewCard", "OverviewHeadingText", "Current status", ACCENT_COLOR, "OverviewValueText", "Waiting for first update.")
    create_section_card(widget_blueprint, "PanelContent", "ProjectCheckCard", "ProjectCheckHeadingText", "Setup readiness", BRAND_COLOR, "ProjectCheckValueText", "TargetFrame updates this card automatically while the panel is open.")

    controls_card = add_widget(widget_blueprint, unreal.Border, "ControlsCard", "PanelContent")
    set_box_slot(controls_card, padding=make_margin(0.0, 0.0, 0.0, 12.0))
    controls_content = add_widget(widget_blueprint, unreal.VerticalBox, "ControlsContent", "ControlsCard")

    presets_label = add_widget(widget_blueprint, unreal.TextBlock, "PresetsLabelText", "ControlsContent")
    configure_text(presets_label, "Quick choices", 11, MUTED_TEXT_COLOR, bold=True)

    presets_row = add_widget(widget_blueprint, unreal.HorizontalBox, "PresetsRow", "ControlsContent")
    set_box_slot(presets_row, padding=make_margin(0.0, 8.0, 0.0, 10.0))
    create_button(widget_blueprint, "PresetsRow", "Preset30Button", "Preset30LabelText", "Lowest load\n30 FPS", TEXT_COLOR)
    create_button(widget_blueprint, "PresetsRow", "Preset45Button", "Preset45LabelText", "Most stable\n45 FPS", TEXT_COLOR)
    create_button(widget_blueprint, "PresetsRow", "Preset60Button", "Preset60LabelText", "Balanced\n60 FPS", PANEL_COLOR)
    create_button(widget_blueprint, "PresetsRow", "Preset90Button", "Preset90LabelText", "High refresh\n90 FPS", TEXT_COLOR, padding=make_margin(0.0))

    actions_primary_row = add_widget(widget_blueprint, unreal.HorizontalBox, "ActionsPrimaryRow", "ControlsContent")
    set_box_slot(actions_primary_row, padding=make_margin(0.0, 0.0, 0.0, 10.0))
    create_button(
        widget_blueprint,
        "ActionsPrimaryRow",
        "ApplyPolicyButton",
        "ApplyPolicyLabelText",
        "Apply recommended setup",
        PANEL_COLOR,
        font_size=11,
        wrap=220.0,
    )
    actions_secondary_row = add_widget(widget_blueprint, unreal.HorizontalBox, "ActionsSecondaryRow", "ControlsContent")
    create_button(
        widget_blueprint,
        "ActionsSecondaryRow",
        "RebenchmarkButton",
        "RebenchmarkLabelText",
        "Check this PC again",
        TEXT_COLOR,
        font_size=11,
        wrap=220.0,
    )
    create_button(
        widget_blueprint,
        "ActionsSecondaryRow",
        "ToggleFPSIndicatorButton",
        "ToggleFPSIndicatorLabelText",
        "Toggle FPS Indicator",
        TEXT_COLOR,
        font_size=11,
        wrap=220.0,
    )
    create_button(
        widget_blueprint,
        "ActionsSecondaryRow",
        "ClosePanelButton",
        "ClosePanelLabelText",
        "Close",
        WARNING_COLOR,
        font_size=11,
        wrap=220.0,
        padding=make_margin(0.0),
    )

    create_scroll_list_card(widget_blueprint, "PanelContent", "NotesCard", "NotesHeadingText", "Why this setup", BRAND_COLOR, "SetupNotesScroll", "SetupNotesList")

    advanced_row = add_widget(widget_blueprint, unreal.HorizontalBox, "AdvancedDetailsRow", "PanelContent")
    set_box_slot(advanced_row, padding=make_margin(0.0, 0.0, 0.0, 12.0))
    create_button(widget_blueprint, "AdvancedDetailsRow", "ToggleAdvancedDetailsButton", "ToggleAdvancedDetailsLabelText", "Show advanced details", TEXT_COLOR, padding=make_margin(0.0))

    shipping_card, _ = create_section_card(widget_blueprint, "PanelContent", "ShippingCapsuleCard", "ShippingCapsuleHeadingText", "Automatic setup details", BRAND_COLOR, "ShippingCapsuleValueText", "Waiting for first update.")
    rendering_card, _ = create_section_card(widget_blueprint, "PanelContent", "RenderingCard", "RenderingHeadingText", "Visual details", BRAND_COLOR, "RenderingValueText", "Waiting for first update.")
    hardware_card, _ = create_section_card(widget_blueprint, "PanelContent", "HardwareCard", "HardwareHeadingText", "PC details", WARNING_COLOR, "HardwareValueText", "Waiting for first update.")
    action_card, _ = create_section_card(widget_blueprint, "PanelContent", "ActionCard", "ActionHeadingText", "Latest adjustment", ACCENT_COLOR, "ActionValueText", "Waiting for first update.")
    events_card, _ = create_scroll_list_card(widget_blueprint, "PanelContent", "EventsCard", "EventsHeadingText", "Recent changes", ACCENT_COLOR, "RecentEventsScroll", "RecentEventsList", bottom_padding=0.0)

    for advanced_card in (shipping_card, rendering_card, hardware_card, action_card, events_card):
        advanced_card.set_visibility(unreal.SlateVisibility.COLLAPSED)

    onboarding_border = add_widget(widget_blueprint, unreal.Border, "OnboardingBorder", "RootCanvas")
    set_canvas_layout(onboarding_border, minimum=(0.5, 0.5), alignment=(0.5, 0.5), position=(0.0, 0.0), size=(760.0, 540.0), z_order=15)
    onboarding_border.set_visibility(unreal.SlateVisibility.COLLAPSED)

    onboarding_scroll = add_widget(widget_blueprint, unreal.ScrollBox, "OnboardingScroll", "OnboardingBorder")
    onboarding_content = add_widget(widget_blueprint, unreal.VerticalBox, "OnboardingContent", "OnboardingScroll")

    onboarding_kicker = add_widget(widget_blueprint, unreal.TextBlock, "OnboardingKickerText", "OnboardingContent")
    set_box_slot(onboarding_kicker, padding=make_margin(0.0, 6.0, 0.0, 0.0))
    configure_text(onboarding_kicker, "TargetFrame Setup Wizard", 12, BRAND_COLOR, bold=True)

    onboarding_title = add_widget(widget_blueprint, unreal.TextBlock, "OnboardingTitleText", "OnboardingContent")
    set_box_slot(onboarding_title, padding=make_margin(0.0, 8.0, 0.0, 10.0))
    configure_text(onboarding_title, "Let's set up TargetFrame", 20, TEXT_COLOR, bold=True, wrap=620.0)

    onboarding_body = add_widget(widget_blueprint, unreal.TextBlock, "OnboardingBodyText", "OnboardingContent")
    set_box_slot(onboarding_body, padding=make_margin(0.0, 0.0, 0.0, 12.0))
    configure_text(onboarding_body, "TargetFrame can take a quick look at this PC, pick a smooth starting point, and keep the game steady without you digging through a long settings menu.", 12, MUTED_TEXT_COLOR, wrap=620.0)

    onboarding_hw_border = add_widget(widget_blueprint, unreal.Border, "OnboardingHardwareCard", "OnboardingContent")
    set_box_slot(onboarding_hw_border, padding=make_margin(0.0, 0.0, 0.0, 10.0))
    onboarding_hw_content = add_widget(widget_blueprint, unreal.VerticalBox, "OnboardingHardwareContent", "OnboardingHardwareCard")
    onboarding_hw_heading = add_widget(widget_blueprint, unreal.TextBlock, "OnboardingHardwareHeadingText", "OnboardingHardwareContent")
    configure_text(onboarding_hw_heading, "Detected PC", 12, ACCENT_COLOR, bold=True)
    onboarding_hw_text = add_widget(widget_blueprint, unreal.TextBlock, "OnboardingHardwareText", "OnboardingHardwareContent")
    set_box_slot(onboarding_hw_text, padding=make_margin(0.0, 10.0, 0.0, 0.0))
    configure_text(onboarding_hw_text, "Waiting for PC check.", 12, TEXT_COLOR, wrap=600.0)

    onboarding_recommendation_border = add_widget(widget_blueprint, unreal.Border, "OnboardingRecommendationCard", "OnboardingContent")
    set_box_slot(onboarding_recommendation_border, padding=make_margin(0.0, 0.0, 0.0, 12.0))
    onboarding_recommendation_content = add_widget(widget_blueprint, unreal.VerticalBox, "OnboardingRecommendationContent", "OnboardingRecommendationCard")
    onboarding_recommendation_heading = add_widget(widget_blueprint, unreal.TextBlock, "OnboardingRecommendationHeadingText", "OnboardingRecommendationContent")
    configure_text(onboarding_recommendation_heading, "Recommended setup", 12, BRAND_COLOR, bold=True)
    onboarding_recommendation_text = add_widget(widget_blueprint, unreal.TextBlock, "OnboardingRecommendationText", "OnboardingRecommendationContent")
    set_box_slot(onboarding_recommendation_text, padding=make_margin(0.0, 10.0, 0.0, 0.0))
    configure_text(onboarding_recommendation_text, "Waiting for setup notes.", 12, TEXT_COLOR, wrap=600.0)

    remember_row = add_widget(widget_blueprint, unreal.HorizontalBox, "OnboardingRememberRow", "OnboardingContent")
    set_box_slot(remember_row, padding=make_margin(0.0, 0.0, 0.0, 16.0))

    remember_checkbox = add_widget(widget_blueprint, unreal.CheckBox, "RememberOnboardingCheckBox", "OnboardingRememberRow")
    set_box_slot(remember_checkbox, vertical_alignment=unreal.VerticalAlignment.V_ALIGN_CENTER)

    remember_label = add_widget(widget_blueprint, unreal.TextBlock, "RememberOnboardingLabelText", "OnboardingRememberRow")
    set_box_slot(
        remember_label,
        padding=make_margin(10.0, 0.0, 0.0, 0.0),
        fill=True,
        horizontal_alignment=unreal.HorizontalAlignment.H_ALIGN_FILL,
        vertical_alignment=unreal.VerticalAlignment.V_ALIGN_CENTER,
    )
    configure_text(remember_label, "Do not show this setup step again on this PC", 11, MUTED_TEXT_COLOR, wrap=560.0)

    onboarding_buttons_row = add_widget(widget_blueprint, unreal.HorizontalBox, "OnboardingButtonsRow", "OnboardingContent")
    create_button(widget_blueprint, "OnboardingButtonsRow", "RunGuidedSetupButton", "RunGuidedSetupLabelText", "Use recommended setup", PANEL_COLOR)
    create_button(widget_blueprint, "OnboardingButtonsRow", "OpenControlPanelButton", "OpenControlPanelLabelText", "Choose settings myself", PANEL_COLOR)
    create_button(widget_blueprint, "OnboardingButtonsRow", "SkipOnboardingButton", "SkipOnboardingLabelText", "Skip for now", TEXT_COLOR, padding=make_margin(0.0))

    compile_and_save(widget_blueprint)
    unreal.log(f"CreateTargetFrameExperienceWidget: created {ASSET_PATH}")


if __name__ == "__main__":
    main()
